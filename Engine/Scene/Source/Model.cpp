#include <Scene/Model.h>
#include <Scene/Camera.h>
#include <RHI/RHI_Device.h>
#include <RHI/RHI_Buffer.h>
#include <Materials/ShaderPermutation.h>
#include <Materials/MaterialInstance.h>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <utility>

namespace grom {

Model::Model() : SceneNode("Model")
{
}

Model::Model(const GString& name) : SceneNode(name)
{
}

Model::~Model()
{
    for (u32 i = 0; i < m_MeshComponents.Size(); ++i)
    {
        if (m_MeshComponents[i].Material)
        {
            delete m_MeshComponents[i].Material;
            m_MeshComponents[i].Material = nullptr;
        }
    }
    m_MeshComponents.Clear();
}

u32 Model::AddMeshComponent(Mesh* mesh, MaterialInstance* material)
{
    MeshComponent comp;
    comp.MeshData = mesh;
    comp.Material = material;
    comp.SectionIndex = 0;
    comp.LocalToMesh = GMatrix4x4::Identity();
    comp.bVisible = true;
    m_MeshComponents.Add(comp);
    m_BoundsDirty = true;
    return static_cast<u32>(m_MeshComponents.Size() - 1);
}

MeshComponent* Model::GetMeshComponent(u32 index)
{
    if (index >= m_MeshComponents.Size()) return nullptr;
    return &m_MeshComponents[index];
}

void Model::AddLOD(const ModelLOD& lod)
{
    m_LODs.Add(lod);
}

void Model::SetActiveLOD(u32 lodIndex)
{
    if (lodIndex < m_LODs.Size())
    {
        m_ActiveLOD = lodIndex;
    }
}

void Model::CalculateBounds()
{
    GVec3 minBound(1e10f, 1e10f, 1e10f);
    GVec3 maxBound(-1e10f, -1e10f, -1e10f);

    for (u32 i = 0; i < m_MeshComponents.Size(); ++i)
    {
        Mesh* mesh = m_MeshComponents[i].MeshData;
        if (!mesh) continue;

        const TArray<Vertex>& verts = mesh->GetVertices();
        for (u32 v = 0; v < verts.Size(); ++v)
        {
            minBound.x = (std::min)(minBound.x, verts[v].Position.x);
            minBound.y = (std::min)(minBound.y, verts[v].Position.y);
            minBound.z = (std::min)(minBound.z, verts[v].Position.z);
            maxBound.x = (std::max)(maxBound.x, verts[v].Position.x);
            maxBound.y = (std::max)(maxBound.y, verts[v].Position.y);
            maxBound.z = (std::max)(maxBound.z, verts[v].Position.z);
        }
    }

    m_BoundsCenter = (minBound + maxBound) * 0.5f;
    m_BoundsExtents = (maxBound - minBound) * 0.5f;
    m_BoundsRadius = m_BoundsExtents.Length();
    m_BoundsDirty = false;
}

void Model::Render(Device* device, Camera* camera)
{
    if (!device || !camera || !IsVisible()) return;

    GMatrix4x4 worldMatrix = GetWorldMatrix();

    for (u32 i = 0; i < m_MeshComponents.Size(); ++i)
    {
        MeshComponent& comp = m_MeshComponents[i];
        if (!comp.bVisible || !comp.MeshData || !comp.Material) continue;

        ShaderPermutation* perm = comp.Material->GetActivePermutation();
        if (!perm) continue;

        device->SetPipeline(perm->GetPipeline());

        Buffer* paramBlock = comp.Material->GetParameterBlock();
        if (paramBlock)
        {
            device->SetConstantBuffer(paramBlock, 2, EShaderType::Pixel);
            device->SetConstantBuffer(paramBlock, 2, EShaderType::Vertex);
        }

        device->SetVertexBuffer(comp.MeshData->GetVertexBuffer(), 0);
        device->SetIndexBuffer(comp.MeshData->GetIndexBuffer());

        const TArray<MeshSection>& sections = comp.MeshData->GetSections();
        if (comp.SectionIndex < sections.Size())
        {
            const MeshSection& section = sections[comp.SectionIndex];
            device->DrawIndexed(section.IndexCount, section.IndexOffset, section.VertexOffset);
        }
    }
}

bool Model::LoadFromFile(const GString& filepath)
{
    if (filepath.Find(".gltf") >= 0 || filepath.Find(".glb") >= 0)
    {
        return LoadGLTF(filepath);
    }
    return false;
}

// -----------------------------------------------------------------------
// Minimal glTF 2.0 parser (handles both .gltf JSON and .glb binary)
// -----------------------------------------------------------------------

static std::vector<u8> ReadFileBytes(const char* path)
{
    FILE* f = nullptr;
    fopen_s(&f, path, "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<u8> buf(static_cast<usize>(len));
    if (len > 0)
        fread(buf.data(), 1, static_cast<usize>(len), f);
    fclose(f);
    return buf;
}

// Simple JSON value tree for parsing glTF
struct GLTFValue
{
    enum Type { Null, Bool, Number, String, Array, Object };
    Type type = Null;
    bool b = false;
    f64 num = 0;
    std::string strval;
    std::vector<GLTFValue> arr;
    std::vector<std::pair<std::string, GLTFValue>> obj;

    const GLTFValue* Get(const char* key) const
    {
        for (auto& kv : obj)
            if (kv.first == key) return &kv.second;
        return nullptr;
    }

    const GLTFValue& operator[](const char* key) const
    {
        static GLTFValue s_null;
        for (auto& kv : obj)
            if (kv.first == key) return kv.second;
        return s_null;
    }

    const GLTFValue& operator[](usize i) const
    {
        static GLTFValue s_null;
        if (i < arr.size()) return arr[i];
        return s_null;
    }

    usize Size() const { return arr.size(); }
    bool IsNull() const { return type == Null; }
    f64 AsNumber(f64 def = 0) const { return type == Number ? num : def; }
    int AsInt(int def = 0) const { return type == Number ? static_cast<int>(num) : def; }
    const std::string& AsString() const { return strval; }
    bool AsBool(bool def = false) const { return type == Bool ? b : def; }

    const GLTFValue* Find(const char* key) const
    {
        for (auto& kv : obj)
            if (kv.first == key) return &kv.second;
        return nullptr;
    }
};

static void SkipWhitespace(const char*& p)
{
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
        ++p;
}

static char Peek(const char*& p) { SkipWhitespace(p); return *p; }

static void Expect(const char*& p, char c)
{
    SkipWhitespace(p);
    if (*p == c) { ++p; return; }
}

static GLTFValue ParseValue(const char*& p);

static std::string ParseString(const char*& p)
{
    Expect(p, '"');
    std::string s;
    while (*p && *p != '"')
    {
        if (*p == '\\')
        {
            ++p;
            switch (*p) {
                case '"': s += '"'; break;
                case '\\': s += '\\'; break;
                case '/': s += '/'; break;
                case 'b': s += '\b'; break;
                case 'f': s += '\f'; break;
                case 'n': s += '\n'; break;
                case 'r': s += '\r'; break;
                case 't': s += '\t'; break;
                case 'u':
                {
                    char hex[5] = {};
                    for (int i = 0; i < 4; ++i) hex[i] = *++p;
                    s += static_cast<char>(std::strtol(hex, nullptr, 16));
                    break;
                }
                default: s += *p; break;
            }
        }
        else s += *p;
        ++p;
    }
    Expect(p, '"');
    return s;
}

static GLTFValue ParseArray(const char*& p)
{
    GLTFValue v;
    v.type = GLTFValue::Array;
    Expect(p, '[');
    while (Peek(p) && Peek(p) != ']')
    {
        v.arr.push_back(ParseValue(p));
        SkipWhitespace(p);
        if (*p == ',') ++p;
    }
    Expect(p, ']');
    return v;
}

static GLTFValue ParseObject(const char*& p)
{
    GLTFValue v;
    v.type = GLTFValue::Object;
    Expect(p, '{');
    while (Peek(p) && Peek(p) != '}')
    {
        std::string key = ParseString(p);
        Expect(p, ':');
        v.obj.push_back({ key, ParseValue(p) });
        SkipWhitespace(p);
        if (*p == ',') ++p;
    }
    Expect(p, '}');
    return v;
}

static GLTFValue ParseNumber(const char*& p)
{
    GLTFValue v;
    v.type = GLTFValue::Number;
    char* end = nullptr;
    v.num = std::strtod(p, &end);
    if (end) p = end;
    return v;
}

static GLTFValue ParseBool(const char*& p)
{
    GLTFValue v;
    v.type = GLTFValue::Bool;
    if (std::strncmp(p, "true", 4) == 0) { v.b = true; p += 4; }
    else if (std::strncmp(p, "false", 5) == 0) { v.b = false; p += 5; }
    return v;
}

static GLTFValue ParseValue(const char*& p)
{
    SkipWhitespace(p);
    if (!*p) return {};
    if (*p == '"') { GLTFValue v; v.type = GLTFValue::String; v.strval = ParseString(p); return v; }
    if (*p == '{') return ParseObject(p);
    if (*p == '[') return ParseArray(p);
    if (*p == 't' || *p == 'f') return ParseBool(p);
    if (*p == 'n') { p += 4; return {}; } // null
    return ParseNumber(p);
}

static GLTFValue ParseJSON(const std::vector<u8>& data)
{
    const char* p = reinterpret_cast<const char*>(data.data());
    return ParseValue(p);
}

static f32 GetFloat(const f32* data, int comp, int components, f32 def)
{
    if (comp < components) return data[comp];
    return def;
}

bool Model::LoadGLTF(const GString& filepath)
{
    std::vector<u8> fileData = ReadFileBytes(filepath.c_str());
    if (fileData.empty()) return false;

    bool isBinary = false;
    std::vector<u8> jsonData;
    std::vector<u8> binData;
    GString baseDir = filepath;
    {
        i32 slash = (i32)baseDir.Len() - 1;
        while (slash >= 0 && baseDir.c_str()[slash] != '/' && baseDir.c_str()[slash] != '\\')
            --slash;
        if (slash >= 0)
            baseDir = baseDir.SubStr(0, static_cast<usize>(slash + 1));
        else
            baseDir = "";
    }

    isBinary = (fileData[0] == 'g' && fileData[1] == 'l' && fileData[2] == 'T' && fileData[3] == 'F');

    if (isBinary)
    {
        struct GLBHeader { u32 magic; u32 version; u32 length; };
        struct GLBChunk { u32 chunkLen; u32 chunkType; };

        if (fileData.size() < sizeof(GLBHeader)) return false;
        const GLBHeader* header = reinterpret_cast<const GLBHeader*>(fileData.data());
        GROM_UNUSED(header);

        usize offset = sizeof(GLBHeader);
        // First chunk: JSON
        if (offset + sizeof(GLBChunk) > fileData.size()) return false;
        const GLBChunk* chunk = reinterpret_cast<const GLBChunk*>(fileData.data() + offset);
        offset += sizeof(GLBChunk);
        if (offset + chunk->chunkLen > fileData.size()) return false;
        jsonData.assign(fileData.begin() + offset, fileData.begin() + offset + chunk->chunkLen);
        offset += chunk->chunkLen;

        // Second chunk: BIN (optional)
        if (offset + sizeof(GLBChunk) <= fileData.size())
        {
            chunk = reinterpret_cast<const GLBChunk*>(fileData.data() + offset);
            offset += sizeof(GLBChunk);
            if (offset + chunk->chunkLen <= fileData.size())
                binData.assign(fileData.begin() + offset, fileData.begin() + offset + chunk->chunkLen);
        }
    }
    else
    {
        jsonData = fileData;
    }

    GLTFValue root = ParseJSON(jsonData);
    if (root.IsNull()) return false;

    const GLTFValue& meshes = root["meshes"];
    const GLTFValue& accessors = root["accessors"];
    const GLTFValue& bufferViews = root["bufferViews"];
    const GLTFValue& buffers = root["buffers"];
    const GLTFValue& materials = root["materials"];
    GROM_UNUSED(root["scenes"]);
    GROM_UNUSED(root["nodes"]);
    GROM_UNUSED(root["textures"]);
    GROM_UNUSED(root["images"]);

    // Load external buffers
    struct LoadedBuffer {
        std::vector<u8> data;
    };
    std::vector<LoadedBuffer> loadedBuffers;
    for (usize i = 0; i < buffers.Size(); ++i)
    {
        const GLTFValue& buf = buffers[i];
        LoadedBuffer lb;
        if (isBinary && i == 0)
        {
            lb.data = binData;
        }
        else
        {
            const GLTFValue* uriVal = buf.Find("uri");
            if (uriVal)
            {
                GString uriPath = baseDir + uriVal->AsString().c_str();
                lb.data = ReadFileBytes(uriPath.c_str());
            }
        }
        loadedBuffers.push_back(std::move(lb));
    }

    // Create meshes
    for (usize m = 0; m < meshes.Size(); ++m)
    {
        const GLTFValue& meshNode = meshes[m];
        const GLTFValue& primitives = meshNode["primitives"];

        for (usize p = 0; p < primitives.Size(); ++p)
        {
            const GLTFValue& prim = primitives[p];

            int posAccIdx = prim["attributes"]["POSITION"].AsInt(-1);
            int normAccIdx = prim["attributes"]["NORMAL"].AsInt(-1);
            int tanAccIdx = prim["attributes"]["TANGENT"].AsInt(-1);
            int tex0AccIdx = prim["attributes"]["TEXCOORD_0"].AsInt(-1);
            int tex1AccIdx = prim["attributes"]["TEXCOORD_1"].AsInt(-1);
            int colAccIdx = prim["attributes"]["COLOR_0"].AsInt(-1);
            int idxAccIdx = prim["indices"].AsInt(-1);
            int matIdx = prim["material"].AsInt(-1);

            if (posAccIdx < 0) continue;

            auto ReadAccessor = [&](int idx, auto& outVec, auto fillFn) -> bool
            {
                if (idx < 0) return false;
                const GLTFValue& acc = accessors[idx];
                int bvIdx = acc["bufferView"].AsInt(-1);
                if (bvIdx < 0) return false;
                const GLTFValue& bv = bufferViews[bvIdx];
                int bufIdx = bv["buffer"].AsInt(0);
                usize byteOffset = static_cast<usize>(bv["byteOffset"].AsNumber(0));
                if (bufIdx >= (int)loadedBuffers.size()) return false;
                const auto& bufData = loadedBuffers[bufIdx].data;
                usize accOffset = static_cast<usize>(acc["byteOffset"].AsNumber(0));
                const u8* src = bufData.data() + byteOffset + accOffset;
                usize count = static_cast<usize>(acc["count"].AsNumber(0));
                usize compType = static_cast<usize>(acc["componentType"].AsNumber(0));
                usize stride = static_cast<usize>(bv["byteStride"].AsNumber(0));
                if (stride == 0) stride = compType == 5126 ? 12 : 0; // float3 default

                outVec.resize(count);
                for (usize i = 0; i < count; ++i)
                {
                    if (compType == 5126)
                    {
                        const f32* f = reinterpret_cast<const f32*>(src + i * (stride ? stride : sizeof(f32) * 4));
                        fillFn(i, f);
                    }
                    else if (compType == 5123)
                    {
                        // unsigned short
                        const u16* us = reinterpret_cast<const u16*>(src + i * (stride ? stride : sizeof(u16)));
                        fillFn(i, reinterpret_cast<const f32*>(us));
                    }
                }
                return true;
            };

            std::vector<GVec3> positions;
            std::vector<GVec3> normals;
            std::vector<GVec4> tangents;
            std::vector<GVec2> uvs0;
            std::vector<GVec2> uvs1;
            std::vector<GColor> colors;
            std::vector<u32> indices;

            usize vertexCount = static_cast<usize>(accessors[posAccIdx]["count"].AsNumber(0));

            ReadAccessor(posAccIdx, positions, [&](usize i, const f32* f) {
                positions[i] = GVec3(f[0], f[1], f[2]);
            });

            ReadAccessor(normAccIdx, normals, [&](usize i, const f32* f) {
                normals[i] = GVec3(f[0], f[1], f[2]);
            });

            if (tanAccIdx >= 0)
            {
                tangents.resize(vertexCount);
                ReadAccessor(tanAccIdx, tangents, [&](usize i, const f32* f) {
                    tangents[i] = GVec4(f[0], f[1], f[2], f[3]);
                });
            }

            if (tex0AccIdx >= 0)
            {
                uvs0.resize(vertexCount);
                ReadAccessor(tex0AccIdx, uvs0, [&](usize i, const f32* f) {
                    uvs0[i] = GVec2(f[0], f[1]);
                });
            }

            if (tex1AccIdx >= 0)
            {
                uvs1.resize(vertexCount);
                ReadAccessor(tex1AccIdx, uvs1, [&](usize i, const f32* f) {
                    uvs1[i] = GVec2(f[0], f[1]);
                });
            }

            if (colAccIdx >= 0)
            {
                colors.resize(vertexCount);
                ReadAccessor(colAccIdx, colors, [&](usize i, const f32* f) {
                    colors[i] = GColor(
                        static_cast<u8>(f[0] * 255.0f),
                        static_cast<u8>(f[1] * 255.0f),
                        static_cast<u8>(f[2] * 255.0f),
                        static_cast<u8>(GetFloat(f, 3, 4, 1.0f) * 255.0f)
                    );
                });
            }

            // Read indices
            if (idxAccIdx >= 0)
            {
                const GLTFValue& idxAcc = accessors[idxAccIdx];
                int bvIdx = idxAcc["bufferView"].AsInt(-1);
                if (bvIdx >= 0)
                {
                    const GLTFValue& bv = bufferViews[bvIdx];
                    int bufIdx = bv["buffer"].AsInt(0);
                    usize byteOffset = static_cast<usize>(bv["byteOffset"].AsNumber(0));
                    usize accOffset = static_cast<usize>(idxAcc["byteOffset"].AsNumber(0));
                    usize compType = static_cast<usize>(idxAcc["componentType"].AsNumber(0));
                    usize count = static_cast<usize>(idxAcc["count"].AsNumber(0));
                    if (bufIdx < (int)loadedBuffers.size())
                    {
                        const u8* src = loadedBuffers[bufIdx].data.data() + byteOffset + accOffset;
                        indices.resize(count);
                        for (usize i = 0; i < count; ++i)
                        {
                            if (compType == 5125) // UINT
                                indices[i] = reinterpret_cast<const u32*>(src)[i];
                            else if (compType == 5123) // USHORT
                                indices[i] = reinterpret_cast<const u16*>(src)[i];
                            else if (compType == 5122) // SHORT
                                indices[i] = static_cast<u32>(reinterpret_cast<const i16*>(src)[i]);
                        }
                    }
                }
            }

            // Build Vertex array
            TArray<Vertex> verts;
            verts.Reserve(static_cast<u32>(vertexCount));
            for (usize i = 0; i < vertexCount; ++i)
            {
                Vertex v;
                v.Position = i < positions.size() ? positions[i] : GVec3(0, 0, 0);
                v.Normal = i < normals.size() ? normals[i] : GVec3(0, 1, 0);
                v.Tangent = i < tangents.size() ? tangents[i] : GVec4(1, 0, 0, 1);
                v.UV0 = i < uvs0.size() ? uvs0[i] : GVec2(0, 0);
                v.UV1 = i < uvs1.size() ? uvs1[i] : GVec2(0, 0);
                v.Color = i < colors.size() ? colors[i] : GColor(255, 255, 255, 255);
                verts.Add(v);
            }

            GString meshName = meshNode["name"].AsString().c_str();
            if (meshName.IsEmpty())
                meshName = GString::Format("Mesh_%zu", m);

            Mesh* mesh = new Mesh();
            mesh->SetName(meshName);

            TArray<u32> gIndices;
            for (usize i = 0; i < indices.size(); ++i)
                gIndices.Add(indices[i]);

            TArray<MeshSection> sections;
            MeshSection sec;
            sec.IndexOffset = 0;
            sec.IndexCount = static_cast<u32>(indices.size());
            sec.VertexOffset = 0;
            sec.VertexCount = static_cast<u32>(vertexCount);
            sec.MaterialIndex = matIdx >= 0 ? static_cast<u32>(matIdx) : 0;
            sec.MaterialName = "";
            sec.SectionName = meshName;
            sections.Add(sec);

            if (!mesh->Create(verts, gIndices, sections))
            {
                delete mesh;
                continue;
            }

            MaterialInstance* mat = nullptr;
            // Try to create a basic material from glTF material
            if (matIdx >= 0 && matIdx < (int)materials.Size())
            {
                mat = new MaterialInstance();
                const GLTFValue& matNode = materials[static_cast<usize>(matIdx)];
                const GLTFValue& pbrMR = matNode["pbrMetallicRoughness"];
                if (!pbrMR.IsNull())
                {
                    const GLTFValue& baseColor = pbrMR["baseColorFactor"];
                    if (!baseColor.IsNull() && baseColor.Size() >= 3)
                    {
                        GColor col(
                            static_cast<u8>(baseColor[static_cast<usize>(0)].AsNumber(1) * 255),
                            static_cast<u8>(baseColor[static_cast<usize>(1)].AsNumber(1) * 255),
                            static_cast<u8>(baseColor[static_cast<usize>(2)].AsNumber(1) * 255),
                            255
                        );
                        mat->SetColor("BaseColor", col);
                    }
                }
            }

            AddMeshComponent(mesh, mat);
        }
    }

    if (m_MeshComponents.Size() > 0)
        CalculateBounds();

    return m_MeshComponents.Size() > 0;
}

}
