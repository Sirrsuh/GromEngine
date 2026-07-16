#include <Scene/Model.h>
#include <Scene/Camera.h>
#include <RHI/RHI_Device.h>
#include <RHI/RHI_Buffer.h>
#include <Materials/ShaderPermutation.h>
#include <Materials/MaterialInstance.h>
#include <algorithm>

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

bool Model::LoadGLTF(const GString& filepath)
{
    GROM_UNUSED(filepath);
    return false;
}

}
