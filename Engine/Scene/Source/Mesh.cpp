#include <Scene/Mesh.h>
#include <RHI/RHI_Device.h>

namespace grom {

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    if (m_VertexBuffer)
    {
        m_VertexBuffer->Release();
        m_VertexBuffer = nullptr;
    }
    if (m_IndexBuffer)
    {
        m_IndexBuffer->Release();
        m_IndexBuffer = nullptr;
    }
}

bool Mesh::Create(const TArray<Vertex>& vertices, const TArray<u32>& indices, const TArray<MeshSection>& sections)
{
    m_Vertices = vertices;
    m_Indices = indices;
    m_Sections = sections;
    m_VertexCount = static_cast<u32>(vertices.Size());
    m_IndexCount = static_cast<u32>(indices.Size());

    Device* dev = Device::GetActiveDevice();
    if (!dev) return false;

    BufferDesc vbDesc;
    vbDesc.Size = static_cast<u32>(vertices.Size() * sizeof(Vertex));
    vbDesc.Stride = sizeof(Vertex);
    vbDesc.Type = EBufferType::Vertex;
    vbDesc.Usage = EBufferUsage::Default;
    vbDesc.InitialData = const_cast<Vertex*>(vertices.Data());
    vbDesc.DebugName = m_Name + "_VB";

    m_VertexBuffer = Buffer::Create(vbDesc, dev->GetAPI());
    if (!m_VertexBuffer) return false;

    BufferDesc ibDesc;
    ibDesc.Size = static_cast<u32>(indices.Size() * sizeof(u32));
    ibDesc.Stride = sizeof(u32);
    ibDesc.Type = EBufferType::Index;
    ibDesc.Usage = EBufferUsage::Default;
    ibDesc.InitialData = const_cast<u32*>(indices.Data());
    ibDesc.DebugName = m_Name + "_IB";

    m_IndexBuffer = Buffer::Create(ibDesc, dev->GetAPI());
    if (!m_IndexBuffer) return false;

    m_Uploaded = true;
    return true;
}

u32 Mesh::GetSectionCount() const
{
    return static_cast<u32>(m_Sections.Size());
}

const MeshSection& Mesh::GetSection(u32 index) const
{
    return m_Sections[index];
}

void Mesh::CreateBox(f32 width, f32 height, f32 depth)
{
    TArray<Vertex> verts;
    TArray<u32> idx;
    TArray<MeshSection> sections;

    f32 hw = width * 0.5f;
    f32 hh = height * 0.5f;
    f32 hd = depth * 0.5f;

    struct Face {
        GVec3 Pos[4];
        GVec3 Normal;
        GVec4 Tangent;
        f32 u[4], v[4];
    };

    Face faces[6] = {
        // Front
        {{{-hw, -hh,  hd}, { hw, -hh,  hd}, { hw,  hh,  hd}, {-hw,  hh,  hd}}, {0,0,1}, {1,0,0,1}, {0,1,0,1}},
        // Back
        {{{ hw, -hh, -hd}, {-hw, -hh, -hd}, {-hw,  hh, -hd}, { hw,  hh, -hd}}, {0,0,-1}, {-1,0,0,1}, {0,1,0,1}},
        // Right
        {{{ hw, -hh,  hd}, { hw, -hh, -hd}, { hw,  hh, -hd}, { hw,  hh,  hd}}, {1,0,0}, {0,0,-1,1}, {0,1,0,1}},
        // Left
        {{{-hw, -hh, -hd}, {-hw, -hh,  hd}, {-hw,  hh,  hd}, {-hw,  hh, -hd}}, {-1,0,0}, {0,0,1,1}, {0,1,0,1}},
        // Top
        {{{-hw,  hh,  hd}, { hw,  hh,  hd}, { hw,  hh, -hd}, {-hw,  hh, -hd}}, {0,1,0}, {1,0,0,1}, {1,0,0,1}},
        // Bottom
        {{{-hw, -hh, -hd}, { hw, -hh, -hd}, { hw, -hh,  hd}, {-hw, -hh,  hd}}, {0,-1,0}, {1,0,0,1}, {0,0,0,1}},
    };

    for (u32 f = 0; f < 6; ++f)
    {
        for (u32 v = 0; v < 4; ++v)
        {
            Vertex vert;
            vert.Position = faces[f].Pos[v];
            vert.Normal = faces[f].Normal;
            vert.Tangent = faces[f].Tangent;
            vert.UV0 = GVec2(faces[f].u[v], faces[f].v[v]);
            vert.UV1 = GVec2(0.0f, 0.0f);
            vert.Color = GColor::White();
            verts.Add(vert);
        }

        u32 base = f * 4;
        idx.Add(base + 0); idx.Add(base + 1); idx.Add(base + 2);
        idx.Add(base + 2); idx.Add(base + 3); idx.Add(base + 0);

        MeshSection section;
        section.IndexOffset = f * 6;
        section.IndexCount = 6;
        section.VertexOffset = f * 4;
        section.VertexCount = 4;
        section.MaterialIndex = 0;
        sections.Add(section);
    }

    Create(verts, idx, sections);
}

void Mesh::CreateSphere(f32 radius, u32 segments, u32 rings)
{
    TArray<Vertex> verts;
    TArray<u32> idx;
    TArray<MeshSection> sections;

    for (u32 r = 0; r <= rings; ++r)
    {
        f32 theta = (f32)r * GROM_PI / (f32)rings;
        f32 sinTheta = std::sin(theta);
        f32 cosTheta = std::cos(theta);

        for (u32 s = 0; s <= segments; ++s)
        {
            f32 phi = (f32)s * 2.0f * GROM_PI / (f32)segments;
            f32 sinPhi = std::sin(phi);
            f32 cosPhi = std::cos(phi);

            Vertex vert;
            vert.Position = GVec3(cosPhi * sinTheta * radius, cosTheta * radius, sinPhi * sinTheta * radius);
            vert.Normal = vert.Position / radius;
            vert.Tangent = GVec4(-sinPhi, 0.0f, cosPhi, 1.0f);
            vert.UV0 = GVec2((f32)s / (f32)segments, (f32)r / (f32)rings);
            vert.UV1 = GVec2(0.0f, 0.0f);
            vert.Color = GColor::White();
            verts.Add(vert);
        }
    }

    for (u32 r = 0; r < rings; ++r)
    {
        for (u32 s = 0; s < segments; ++s)
        {
            u32 i0 = r * (segments + 1) + s;
            u32 i1 = i0 + 1;
            u32 i2 = (r + 1) * (segments + 1) + s;
            u32 i3 = i2 + 1;

            idx.Add(i0); idx.Add(i1); idx.Add(i2);
            idx.Add(i2); idx.Add(i1); idx.Add(i3);
        }
    }

    MeshSection section;
    section.IndexOffset = 0;
    section.IndexCount = static_cast<u32>(idx.Size());
    section.VertexOffset = 0;
    section.VertexCount = static_cast<u32>(verts.Size());
    section.MaterialIndex = 0;
    sections.Add(section);

    Create(verts, idx, sections);
}

void Mesh::CreatePlane(f32 width, f32 depth, u32 segmentsX, u32 segmentsZ)
{
    TArray<Vertex> verts;
    TArray<u32> idx;
    TArray<MeshSection> sections;

    f32 hw = width * 0.5f;
    f32 hd = depth * 0.5f;

    for (u32 z = 0; z <= segmentsZ; ++z)
    {
        for (u32 x = 0; x <= segmentsX; ++x)
        {
            Vertex vert;
            vert.Position = GVec3(
                -hw + (f32)x / (f32)segmentsX * width,
                0.0f,
                -hd + (f32)z / (f32)segmentsZ * depth
            );
            vert.Normal = GVec3(0.0f, 1.0f, 0.0f);
            vert.Tangent = GVec4(1.0f, 0.0f, 0.0f, 1.0f);
            vert.UV0 = GVec2((f32)x / (f32)segmentsX, (f32)z / (f32)segmentsZ);
            vert.UV1 = GVec2(0.0f, 0.0f);
            vert.Color = GColor::White();
            verts.Add(vert);
        }
    }

    for (u32 z = 0; z < segmentsZ; ++z)
    {
        for (u32 x = 0; x < segmentsX; ++x)
        {
            u32 i0 = z * (segmentsX + 1) + x;
            u32 i1 = i0 + 1;
            u32 i2 = (z + 1) * (segmentsX + 1) + x;
            u32 i3 = i2 + 1;

            idx.Add(i0); idx.Add(i1); idx.Add(i2);
            idx.Add(i2); idx.Add(i1); idx.Add(i3);
        }
    }

    MeshSection section;
    section.IndexOffset = 0;
    section.IndexCount = static_cast<u32>(idx.Size());
    section.VertexOffset = 0;
    section.VertexCount = static_cast<u32>(verts.Size());
    section.MaterialIndex = 0;
    sections.Add(section);

    Create(verts, idx, sections);
}

}
