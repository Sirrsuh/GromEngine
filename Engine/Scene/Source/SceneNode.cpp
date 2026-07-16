#include <Scene/SceneNode.h>

namespace grom {

u32 SceneNode::s_NextID = 1;

SceneNode::SceneNode()
    : m_Position(0.0f, 0.0f, 0.0f)
    , m_Rotation(0.0f, 0.0f, 0.0f, 1.0f)
    , m_Scale(1.0f, 1.0f, 1.0f)
    , m_Parent(nullptr)
    , m_ID(s_NextID++)
    , m_Visible(true)
    , m_TransformDirty(true)
{
    m_WorldMatrix = GMatrix4x4::Identity();
}

SceneNode::SceneNode(const GString& name)
    : SceneNode()
{
    m_Name = name;
}

SceneNode::~SceneNode()
{
    for (u32 i = 0; i < m_Children.Size(); ++i)
    {
        delete m_Children[i];
    }
    m_Children.Clear();
}

void SceneNode::SetPosition(const GVec3& pos)
{
    m_Position = pos;
    MarkDirty();
}

void SceneNode::SetRotation(const GQuat& rot)
{
    m_Rotation = rot;
    MarkDirty();
}

void SceneNode::SetScale(const GVec3& scale)
{
    m_Scale = scale;
    MarkDirty();
}

void SceneNode::SetTransform(const GTransform& transform)
{
    m_Position = transform.Translation;
    m_Rotation = transform.Rotation;
    m_Scale = transform.Scale;
    MarkDirty();
}

GVec3 SceneNode::GetPosition() const { return m_Position; }
GQuat SceneNode::GetRotation() const { return m_Rotation; }
GVec3 SceneNode::GetScale() const { return m_Scale; }

GTransform SceneNode::GetLocalTransform() const
{
    GTransform t;
    t.Translation = m_Position;
    t.Rotation = m_Rotation;
    t.Scale = m_Scale;
    return t;
}

GMatrix4x4 SceneNode::GetLocalMatrix() const
{
    GMatrix4x4 t = GMatrix4x4::Translation(m_Position.x, m_Position.y, m_Position.z);
    GMatrix4x4 r = m_Rotation.ToMatrix();
    GMatrix4x4 s = GMatrix4x4::Scale(m_Scale.x, m_Scale.y, m_Scale.z);
    return t * r * s;
}

GMatrix4x4 SceneNode::GetWorldMatrix() const
{
    if (m_TransformDirty)
    {
        if (m_Parent)
        {
            m_WorldMatrix = m_Parent->GetWorldMatrix() * GetLocalMatrix();
        }
        else
        {
            m_WorldMatrix = GetLocalMatrix();
        }
        m_TransformDirty = false;
    }
    return m_WorldMatrix;
}

void SceneNode::SetWorldMatrix(const GMatrix4x4& world)
{
    m_WorldMatrix = world;
    m_TransformDirty = false;

    GVec3 pos, scale;
    GQuat rot;
    world.Decompose(pos, rot, scale);
    m_Position = pos;
    m_Rotation = rot;
    m_Scale = scale;
}

GVec3 SceneNode::GetWorldPosition() const
{
    GMatrix4x4 w = GetWorldMatrix();
    return GVec3(w.m[0][3], w.m[1][3], w.m[2][3]);
}

GVec3 SceneNode::GetForward() const
{
    GMatrix4x4 w = GetWorldMatrix();
    return GVec3(-w.m[2][0], -w.m[2][1], -w.m[2][2]).Normalized();
}

GVec3 SceneNode::GetRight() const
{
    GMatrix4x4 w = GetWorldMatrix();
    return GVec3(w.m[0][0], w.m[0][1], w.m[0][2]).Normalized();
}

GVec3 SceneNode::GetUp() const
{
    GMatrix4x4 w = GetWorldMatrix();
    return GVec3(w.m[1][0], w.m[1][1], w.m[1][2]).Normalized();
}

void SceneNode::SetParent(SceneNode* parent)
{
    if (m_Parent)
    {
        m_Parent->RemoveChild(this);
    }
    m_Parent = parent;
    MarkDirty();
}

void SceneNode::AddChild(SceneNode* child)
{
    if (!child || child->m_Parent == this) return;
    m_Children.Add(child);
    child->m_Parent = this;
    child->MarkDirty();
}

void SceneNode::RemoveChild(SceneNode* child)
{
    for (u32 i = 0; i < m_Children.Size(); ++i)
    {
        if (m_Children[i] == child)
        {
            m_Children.RemoveAt(i);
            child->m_Parent = nullptr;
            child->MarkDirty();
            return;
        }
    }
}

void SceneNode::MarkDirty()
{
    m_TransformDirty = true;
    for (u32 i = 0; i < m_Children.Size(); ++i)
    {
        m_Children[i]->MarkDirty();
    }
    OnTransformChanged();
}

void SceneNode::Traverse(const GMatrix4x4& parentWorld, bool parentVisible)
{
    GROM_UNUSED(parentWorld);
    bool effectiveVisible = parentVisible && m_Visible;
    if (!effectiveVisible) return;

    GetWorldMatrix();

    for (u32 i = 0; i < m_Children.Size(); ++i)
    {
        m_Children[i]->Traverse(m_WorldMatrix, effectiveVisible);
    }
}

}
