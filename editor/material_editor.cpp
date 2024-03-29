#include "material_editor.h"

#include "event_queue.h"
#include "events.h"
#include "vulkan_texture.h"
#include "asset_manager.h"
#include "validation.h"

#include <imgui.h>


MaterialEditor::MaterialEditor()
{    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::MaterialSelected)
            {
                EventMaterialSelected* e =
                    dynamic_cast<EventMaterialSelected*>(event);
                this->selectedMat = 
                    reinterpret_cast<renderer::VulkanMaterial*>(e->materialPtr);
            }
            else if (event->type == Event::Type::ProjectOpen)
            {
                EventProjectOpen* e = dynamic_cast<EventProjectOpen*>(event);
                this->assetManager = reinterpret_cast<AssetManager*>(e->assetManager);
                this->selectedMat = nullptr;
            }
            else if (event->type == Event::Type::CloseProject)
            {
                this->selectedMat = nullptr;
                this->availableMaterials.clear();
                this->availableMaterialCached = false;
                this->availableTextures.clear();
                this->availableTexureCached = false;
                this->assetManager = nullptr;
            }
            else if (event->type == Event::Type::WorkspaceChanged)
            {
                this->availableMaterials.clear();
                this->availableMaterialCached = false;
                this->availableTextures.clear();
                this->availableTexureCached = false;
            }
        });
}

MaterialEditor::~MaterialEditor()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}


void MaterialEditor::Draw()
{

    ASSERT(assetManager != nullptr);
    ImGui::Begin("Material Editor", nullptr);
    ImGui::Text("Properties of the selected material");
    ImGui::Separator();

    ShowMaterialProperties();
    
    ImGui::End();
}

void MaterialEditor::ShowMaterialProperties()
{

    if (!availableMaterialCached)
    {
        assetManager->GetAvailableMaterials(availableMaterials);
        availableMaterialCached = true;
    }

    ImGui::Text("Material:");

    std::string currentMaterialStr;
    const char* currentMaterialPath;
    if (selectedMat)
    {
        currentMaterialStr = selectedMat->GetProperties()->resourcePath; 
    }
    else
    {
        currentMaterialStr = "None";
    }
    currentMaterialPath = currentMaterialStr.c_str();

    if (ImGui::BeginCombo("Material Path", currentMaterialPath))
    {
        for (int n = 0; n < availableMaterials.size(); n++)
        {
            const bool isSelected =
                strcmp(availableMaterials[n], currentMaterialPath) == 0;
            if (ImGui::Selectable(availableMaterials[n], isSelected))
            {
                std::shared_ptr<renderer::Material> material =
                    assetManager->GetMaterial(availableMaterials[n]);
                PublishMaterialSelectedEvent(material.get());
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    

    if (selectedMat == nullptr)
    {
        ImGui::Text("No material is selected.");
        return;
    }

    const renderer::MaterialProperties* properties =
        selectedMat->GetProperties();

    ShowAlbedoSection(properties);
    ShowMetallicSection(properties);
    ShowRoughnessSection(properties);
}

void MaterialEditor::ShowAlbedoSection(
    const renderer::MaterialProperties* properties)
{
    ImGui::SeparatorText("Albedo");
    
    if (!availableTexureCached)
    {
        assetManager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    std::string resourcePath;
    bool hasTexture = (properties->albedoTexture != nullptr);
    if (hasTexture)
    {
        const std::shared_ptr<renderer::Texture> texture =
            properties->albedoTexture;
        resourcePath = texture->GetBuildInfo().resourcePath;
    }
    else
    {
        resourcePath = "None";
    }

    ImGui::Text("Texture:");
    const char* currentTexPath = resourcePath.c_str();
    if (ImGui::BeginCombo("Texture Path##1", currentTexPath))
    {
        for (int n = 0; n < availableTextures.size(); n++)
        {
            const bool isSelected =
                strcmp(availableTextures[n], currentTexPath) == 0;
            if (ImGui::Selectable(availableTextures[n], isSelected))
            {
                selectedMat->AddAlbedoTexture(
                    assetManager->GetTexture(availableTextures[n]));
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!hasTexture) ImGui::BeginDisabled();
    {
        bool removeTex = ImGui::Button("Remove##1");
        ImGui::SameLine();
        bool openInEditor = ImGui::Button("Open in Editor##1");

        if (removeTex)
        {
            selectedMat->ResetAlbedoTexture();
        }
        else if (openInEditor)
        {
            PublishTextureSelectedEvent(properties->albedoTexture.get());
        }
    }
    if (!hasTexture) ImGui::EndDisabled();

    ImGui::Text("Color:");
    if (hasTexture) ImGui::BeginDisabled();
    {
        glm::vec3 color = properties->albedo;
        ImGui::ColorEdit3("Color##1", (float*)&color[0],
            ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB |
            ImGuiColorEditFlags_Float);
        selectedMat->SetAlbedo(color);
    }
    if (hasTexture) ImGui::EndDisabled();

}

void MaterialEditor::ShowMetallicSection(
    const renderer::MaterialProperties* properties)
{
    ImGui::SeparatorText("Metallic");

    if (!availableTexureCached)
    {
        assetManager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    std::string resourcePath;
    bool hasTexture = (properties->metallicTexture != nullptr);
    if (hasTexture)
    {
        const std::shared_ptr<renderer::Texture> texture =
            properties->metallicTexture;
        resourcePath = texture->GetBuildInfo().resourcePath;
    }
    else
    {
        resourcePath = "None";
    }

    ImGui::Text("Texture:");
    const char* currentTexPath = resourcePath.c_str();
    if (ImGui::BeginCombo("Texture Path##2", currentTexPath))
    {
        for (int n = 0; n < availableTextures.size(); n++)
        {
            const bool isSelected =
                strcmp(availableTextures[n], currentTexPath) == 0;
            if (ImGui::Selectable(availableTextures[n], isSelected))
            {
                selectedMat->AddMetallicTexture(
                    assetManager->GetTexture(availableTextures[n]));
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!hasTexture) ImGui::BeginDisabled();
    {
        bool removeTex = ImGui::Button("Remove##2");
        ImGui::SameLine();
        bool openInEditor = ImGui::Button("Open in Editor##2");

        if (removeTex)
        {
            selectedMat->ResetMetallicTexture();
        }
        else if (openInEditor)
        {
            PublishTextureSelectedEvent(properties->metallicTexture.get());
        }
    }
    if (!hasTexture) ImGui::EndDisabled();

    ImGui::Text("Metallic:");
    if (hasTexture) ImGui::BeginDisabled();
    {
        float metallic = properties->metallic;
        ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f, "%.2f");
        selectedMat->SetMetallic(metallic);
    }
    if (hasTexture) ImGui::EndDisabled();
}

void MaterialEditor::ShowRoughnessSection(
    const renderer::MaterialProperties* properties)

{
    ImGui::SeparatorText("Roughness");

    bool hasTexture = (properties->roughnessTexture != nullptr);
    if (!availableTexureCached)
    {
        assetManager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    std::string resourcePath;
    if (hasTexture)
    {
        const std::shared_ptr<renderer::Texture> texture =
            properties->roughnessTexture;
        resourcePath = texture->GetBuildInfo().resourcePath;
    }
    else
    {
        resourcePath = "None";
    }

    ImGui::Text("Texture:");
    const char* currentTexPath = resourcePath.c_str();
    if (ImGui::BeginCombo("Texture Path##3", currentTexPath))
    {
        for (int n = 0; n < availableTextures.size(); n++)
        {
            const bool isSelected =
                strcmp(availableTextures[n], currentTexPath) == 0;
            if (ImGui::Selectable(availableTextures[n], isSelected))
            {
                selectedMat->AddRoughnessTexture(
                    assetManager->GetTexture(availableTextures[n]));
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!hasTexture) ImGui::BeginDisabled();
    {
        bool removeTex = ImGui::Button("Remove##3");
        ImGui::SameLine();
        bool openInEditor = ImGui::Button("Open in Editor##3");

        if (removeTex)
        {
            selectedMat->ResetRoughnessTexture();
        }
        else if (openInEditor)
        {
            PublishTextureSelectedEvent(properties->roughnessTexture.get());
        }
    }
    if (!hasTexture) ImGui::EndDisabled();

    ImGui::Text("Roughness:");
    if (hasTexture) ImGui::BeginDisabled();
    {
        float roughness = properties->roughness;
        ImGui::SliderFloat("Roughness", &roughness, 0.01f, 1.0f, "%.2f");
        selectedMat->SetRoughness(roughness);
    }
    if (hasTexture) ImGui::EndDisabled();
}

void MaterialEditor::PublishMaterialSelectedEvent(
    renderer::Material* mat)
{
    ASSERT(mat != nullptr);

    EventMaterialSelected* event = new EventMaterialSelected();
    event->materialPtr = mat;

    EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
}

void MaterialEditor::PublishTextureSelectedEvent(
    renderer::Texture* tex)
{
    ASSERT(tex != nullptr);

    EventTextureSelected* event = new EventTextureSelected();
    event->texturePtr = tex;

    EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
}