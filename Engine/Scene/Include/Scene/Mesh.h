#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <RHI/RHI_Buffer.h>
#include <RHI/RHI_Pipeline.h>
#include <Materials/MaterialInstance.h>

namespace grom {

struct MeshSection {
    u32 IndexOffset;
    u32 IndexCount;
    u32 VertexOffset;
    u32 VertexCount;
    u32 MaterialIndex;
    GString MaterialName;
    GString SectionName;
};

struct MeshLOD {
    TArray<MeshSection> Sections;
    f32 ScreenSize;
};

struct Vertex {
    GVec3 Position;
    GVec3 Normal;
    GVec4 Tangent;
    GVec2 UV0;
    GVec2 UV1;
    GColor Color;
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    bool Create(const TArray<Vertex>& vertices, const TArray<u32>& indices, const TArray<MeshSection>& sections);
    void CreateBox(f32 width, f32 height, f32 depth);
    void CreateSphere(f32 radius, u32 segments, u32 rings);
    void CreatePlane(f32 width, f32 depth, u32 segmentsX, u32 segmentsZ);

    Buffer* GetVertexBuffer() const { return m_VertexBuffer; }
    Buffer* GetIndexBuffer() const { return m_IndexBuffer; }

    u32 GetVertexCount() const { return m_VertexCount; }
    u32 GetIndexCount() const { return m_IndexCount; }
    u32 GetSectionCount() const;
    const MeshSection& GetSection(u32 index) const;
    const TArray<MeshSection>& GetSections() const { return m_Sections; }

    const TArray<Vertex>& GetVertices() const { return m_Vertices; }

    void SetName(const GString& name) { m_Name = name; }
    GString GetName() const { return m_Name; }

private:
    GString m_Name;
    TArray<Vertex> m_Vertices;
    TArray<u32> m_Indices;
    TArray<MeshSection> m_Sections;
    Buffer* m_VertexBuffer = nullptr;
    Buffer* m_IndexBuffer = nullptr;
    u32 m_VertexCount = 0;
    u32 m_IndexCount = 0;
    bool m_Uploaded = false;
};

}
