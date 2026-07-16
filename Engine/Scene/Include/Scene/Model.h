#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <Scene/SceneNode.h>
#include <Scene/Mesh.h>
#include <Materials/MaterialInstance.h>

namespace grom {

struct MeshComponent {
    Mesh* MeshData = nullptr;
    MaterialInstance* Material = nullptr;
    u32 SectionIndex = 0;
    GMatrix4x4 LocalToMesh;
    bool bSelected = false;
    bool bVisible = true;
};

struct ModelLOD {
    TArray<MeshComponent> Components;
    f32 ScreenSize = 0.0f;
};

class Model : public SceneNode {
public:
    Model();
    explicit Model(const GString& name);
    ~Model();

    u32 AddMeshComponent(Mesh* mesh, MaterialInstance* material = nullptr);
    MeshComponent* GetMeshComponent(u32 index);
    u32 GetMeshComponentCount() const { return static_cast<u32>(m_MeshComponents.Size()); }

    void AddLOD(const ModelLOD& lod);
    void SetActiveLOD(u32 lodIndex);
    u32 GetActiveLOD() const { return m_ActiveLOD; }
    u32 GetLODCount() const { return static_cast<u32>(m_LODs.Size()); }

    void CalculateBounds();
    GVec3 GetBoundsCenter() const { return m_BoundsCenter; }
    GVec3 GetBoundsExtents() const { return m_BoundsExtents; }
    f32 GetBoundsRadius() const { return m_BoundsRadius; }

    void Render(class Device* device, class Camera* camera);

    bool LoadFromFile(const GString& filepath);
    bool LoadGLTF(const GString& filepath);

private:
    TArray<MeshComponent> m_MeshComponents;
    TArray<ModelLOD> m_LODs;
    u32 m_ActiveLOD = 0;
    GVec3 m_BoundsCenter;
    GVec3 m_BoundsExtents;
    f32 m_BoundsRadius = 0.0f;
    bool m_BoundsDirty = true;
};

}
