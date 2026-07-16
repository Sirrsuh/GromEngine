#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>

namespace grom {

class SceneNode {
public:
    SceneNode();
    explicit SceneNode(const GString& name);
    virtual ~SceneNode();

    void SetPosition(const GVec3& pos);
    void SetRotation(const GQuat& rot);
    void SetScale(const GVec3& scale);
    void SetTransform(const GTransform& transform);

    GVec3 GetPosition() const;
    GQuat GetRotation() const;
    GVec3 GetScale() const;
    GTransform GetLocalTransform() const;

    GMatrix4x4 GetLocalMatrix() const;
    GMatrix4x4 GetWorldMatrix() const;
    void SetWorldMatrix(const GMatrix4x4& world);

    GVec3 GetWorldPosition() const;
    GVec3 GetForward() const;
    GVec3 GetRight() const;
    GVec3 GetUp() const;

    void SetParent(SceneNode* parent);
    SceneNode* GetParent() const { return m_Parent; }
    void AddChild(SceneNode* child);
    void RemoveChild(SceneNode* child);
    SceneNode* GetChild(u32 index) const { return m_Children[index]; }
    u32 GetChildCount() const { return static_cast<u32>(m_Children.Size()); }
    const TArray<SceneNode*>& GetChildren() const { return m_Children; }

    template<typename T>
    T* AddChildAs(const GString& name = "") {
        T* child = new T(name);
        AddChild(child);
        return child;
    }

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }
    void SetName(const GString& name) { m_Name = name; }
    GString GetName() const { return m_Name; }
    u32 GetID() const { return m_ID; }

    void MarkDirty();
    bool IsTransformDirty() const { return m_TransformDirty; }

    void Traverse(const GMatrix4x4& parentWorld, bool parentVisible);

protected:
    virtual void OnTransformChanged() { GROM_UNUSED(this); }
    virtual void OnVisibilityChanged(bool visible) { GROM_UNUSED(visible); }
    virtual void OnAddedToScene(class Scene* scene) { GROM_UNUSED(scene); }
    virtual void OnRemovedFromScene(class Scene* scene) { GROM_UNUSED(scene); }

    GVec3 m_Position;
    GQuat m_Rotation;
    GVec3 m_Scale;
    mutable GMatrix4x4 m_WorldMatrix;
    SceneNode* m_Parent;
    TArray<SceneNode*> m_Children;
    GString m_Name;
    u32 m_ID;
    bool m_Visible;
    mutable bool m_TransformDirty;

    static u32 s_NextID;
};

}
