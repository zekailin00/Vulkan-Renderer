#include "application.h"

class Scene1: public Application
{
public:
    void OnCreated() override;
    void OnUpdated(float ts) override;
    void OnDestroy() override;

    int editorHandle = -1;
};

void Scene1::OnCreated() 
{
    editorHandle = SetActiveScene(Scene::NewScene("scene1", nullptr));

    Scene* scene = GetActiveScene(editorHandle);

    Entity* camera = scene->NewEntity();

    renderer::CameraComponent* camComponent =
        reinterpret_cast<renderer::CameraComponent*>(
            camera->AddComponent(Component::Type::Camera));
    
    renderer::CameraProperties camProp{};
    camProp.UseFrameExtent = false;
    camProp.Extent = {1920, 1080};
    camComponent->camera->RebuildCamera(camProp);

    camera->SetLocalTransform({0,0,15}, {0,0,0}, {1,1,1});
    renderer->SetWindowContent(camComponent->camera);

    Entity* lineEntity = scene->NewEntity();

    renderer::LineComponent* lineComponent =
        reinterpret_cast<renderer::LineComponent*>(
            lineEntity->AddComponent(Component::Type::Line));

    lineComponent->lineRenderer->AddLine(
        {{0,0,0},{2,2,2}}
    );
    lineComponent->lineRenderer->AddLine(
        {{-3,0,0},{0,0,0}}
    );

    std::vector<renderer::LineData> lineDataList{};

    renderer::VulkanLineGenerator::GetCircle(
        lineDataList, {0,0,0}, {0,0,1}, 1, 32);
    
    lineComponent->lineRenderer->AddLines(lineDataList);

    renderer::LineRenderer::LineProperties* prop =
        lineComponent->lineRenderer->GetLineProperties();
    
    prop->color = {1, 0, 0};
    prop->width = 2;

    {
        Entity* kinematicFloor = scene->NewEntity();

        physics::DynamicBodyComponent* comp =
            dynamic_cast<physics::DynamicBodyComponent*>(
                kinematicFloor->AddComponent(Component::Type::DynamicBody));
        
        physics::PhysicsContext* context = comp->dynamicBody->GetContext();

        physics::CollisionShape * box = context->AddCollisionShape(
            comp->dynamicBody, physics::GeometryType::eBOX);
        
        box->SetGeometry(physics::BoxGeometry(10, 1, 10));
        comp->dynamicBody->SetKinematic(true);
        comp->dynamicBody->SetGlobalTransform(
            glm::translate(glm::mat4(1.0f), {0, -5, 0})
        );
    }
}

void Scene1::OnUpdated(float ts) {}

void Scene1::OnDestroy()
{
    delete EraseActiveScene(editorHandle);
}

int main(int argc, char** argv)
{
    Scene1 app{};
    app.Run();
    return 0;
}