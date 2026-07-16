#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Scene/SceneNode.h>

namespace grom {

class Camera;
class Light;
struct MeshComponent;

class Scene {
public:
    Scene();
    ~Scene();

    SceneNode* GetRootNode() const { return m_RootNode; }

    void SetActiveCamera(Camera* camera);
    Camera* GetActiveCamera() const { return m_ActiveCamera; }
    void AddCamera(Camera* camera);
    void RemoveCamera(Camera* camera);
    const TArray<Camera*>& GetCameras() const { return m_Cameras; }

    void AddLight(Light* light);
    void RemoveLight(Light* light);
    const TArray<Light*>& GetLights() const { return m_Lights; }
    Light* GetSunLight() const { return m_SunLight; }

    void AddMeshComponent(struct MeshComponent* comp);
    void RemoveMeshComponent(struct MeshComponent* comp);
    const TArray<struct MeshComponent*>& GetMeshComponents() const { return m_MeshComponents; }

    void Update(f32 deltaTime);
    void UpdateTransformHierarchy();

    void AddNode(SceneNode* node);
    void RemoveNode(SceneNode* node);

private:
    SceneNode* m_RootNode;
    Camera* m_ActiveCamera;
    Light* m_SunLight;
    TArray<Camera*> m_Cameras;
    TArray<Light*> m_Lights;
    TArray<struct MeshComponent*> m_MeshComponents;
};

}
