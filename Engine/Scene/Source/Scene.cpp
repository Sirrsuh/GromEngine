#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Light.h>
#include <Scene/Mesh.h>
#include <Scene/Model.h>

namespace grom {

Scene::Scene()
{
    m_RootNode = new SceneNode("Root");
}

Scene::~Scene()
{
    delete m_RootNode;
    m_Cameras.Clear();
    m_Lights.Clear();
    m_MeshComponents.Clear();
}

void Scene::SetActiveCamera(Camera* camera)
{
    m_ActiveCamera = camera;
}

void Scene::AddCamera(Camera* camera)
{
    if (!camera) return;
    m_Cameras.Add(camera);
    if (!m_ActiveCamera)
    {
        m_ActiveCamera = camera;
    }
}

void Scene::RemoveCamera(Camera* camera)
{
    for (u32 i = 0; i < m_Cameras.Size(); ++i)
    {
        if (m_Cameras[i] == camera)
        {
            m_Cameras.RemoveAt(i);
            if (m_ActiveCamera == camera)
            {
                m_ActiveCamera = m_Cameras.Size() > 0 ? m_Cameras[0] : nullptr;
            }
            return;
        }
    }
}

void Scene::AddLight(Light* light)
{
    if (!light) return;
    m_Lights.Add(light);
    if (light->GetLightType() == ELightType::Directional && !m_SunLight)
    {
        m_SunLight = light;
    }
}

void Scene::RemoveLight(Light* light)
{
    for (u32 i = 0; i < m_Lights.Size(); ++i)
    {
        if (m_Lights[i] == light)
        {
            if (m_SunLight == light) m_SunLight = nullptr;
            m_Lights.RemoveAt(i);
            return;
        }
    }
}

void Scene::AddMeshComponent(MeshComponent* comp)
{
    if (!comp) return;
    m_MeshComponents.Add(comp);
}

void Scene::RemoveMeshComponent(MeshComponent* comp)
{
    for (u32 i = 0; i < m_MeshComponents.Size(); ++i)
    {
        if (m_MeshComponents[i] == comp)
        {
            m_MeshComponents.RemoveAt(i);
            return;
        }
    }
}

void Scene::Update(f32 deltaTime)
{
    GROM_UNUSED(deltaTime);
    UpdateTransformHierarchy();
}

void Scene::UpdateTransformHierarchy()
{
    GMatrix4x4 identity = GMatrix4x4::Identity();
    m_RootNode->Traverse(identity, true);
}

void Scene::AddNode(SceneNode* node)
{
    if (node && node->GetParent() == nullptr)
    {
        m_RootNode->AddChild(node);
    }
}

void Scene::RemoveNode(SceneNode* node)
{
    if (node && node->GetParent() == m_RootNode)
    {
        m_RootNode->RemoveChild(node);
    }
}

}
