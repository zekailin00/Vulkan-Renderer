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
        });
}

MaterialEditor::~MaterialEditor()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}


void MaterialEditor::Draw()
{
    ImGui::Begin("Material Editor", nullptr);
    ImGui::Text("Properties of the selected material");
    ImGui::Separator();



    ShowMaterialProperties();
    
    ImGui::End();
}

void MaterialEditor::ShowMaterialProperties()
{
    AssetManager* manager = AssetManager::GetInstance();

    if (!availableMaterialCached)
    {
        manager->GetAvailableMaterials(availableMaterials);
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
                    manager->GetMaterial(availableMaterials[n]);
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
        AssetManager* manager = AssetManager::GetInstance();
        manager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    std::string resourcePath;
    if (properties->albedoTexture)
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
                //TODO: event to change texture
                // AssetManager* manager = AssetManager::GetInstance();
                // selectedMat->AddAlbedoTexture(
                //     manager->GetTexture(availableTextures[n]));
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!properties->albedoTexture) ImGui::BeginDisabled();
    {
        bool removeTex = ImGui::Button("Remove##1");
        ImGui::SameLine();
        bool openInEditor = ImGui::Button("Open in Editor##1");

        if (removeTex)
        {
            //TODO: event
            // selectedMat->ResetAlbedoTexture();
        }
        else if (openInEditor)
        {
            // TODO: texture select event
        }
    }
    if (!properties->albedoTexture) ImGui::EndDisabled();

    ImGui::Text("Color:");
    if (properties->albedoTexture) ImGui::BeginDisabled();
    {
        glm::vec3 color = properties->albedo;
        ImGui::ColorEdit3("Color##1", (float*)&color[0],
            ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB |
            ImGuiColorEditFlags_Float);
        selectedMat->SetAlbedo(color);
    }
    if (properties->albedoTexture) ImGui::EndDisabled();

}

void MaterialEditor::ShowMetallicSection(
    const renderer::MaterialProperties* properties)
{
    ImGui::SeparatorText("Metallic");

    if (!availableTexureCached)
    {
        AssetManager* manager = AssetManager::GetInstance();
        manager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    std::string resourcePath;
    if (properties->metallicTexture)
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
                //TODO: event to change texture
                // AssetManager* manager = AssetManager::GetInstance();
                // selectedMat->AddMetallicTexture(
                //     manager->GetTexture(availableTextures[n]));
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!properties->metallicTexture) ImGui::BeginDisabled();
    {
        bool removeTex = ImGui::Button("Remove##2");
        ImGui::SameLine();
        bool openInEditor = ImGui::Button("Open in Editor##2");

        if (removeTex)
        {
            //TODO: event
            // selectedMat->ResetMetallicTexture();
        }
        else if (openInEditor)
        {
            // TODO: texture select event
        }
    }
    if (!properties->metallicTexture) ImGui::EndDisabled();

    ImGui::Text("Metallic:");
    if (properties->metallicTexture) ImGui::BeginDisabled();
    {
        float metallic = properties->metallic;
        ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f, "%.2f");
        selectedMat->SetMetallic(metallic);
    }
    if (properties->metallicTexture) ImGui::EndDisabled();
}

void MaterialEditor::ShowRoughnessSection(
    const renderer::MaterialProperties* properties)

{
    ImGui::SeparatorText("Roughness");

    if (!availableTexureCached)
    {
        AssetManager* manager = AssetManager::GetInstance();
        manager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    std::string resourcePath;
    if (properties->roughnessTexture)
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
                //TODO: event to change texture
                // AssetManager* manager = AssetManager::GetInstance();
                // selectedMat->AddroughnessTexture(
                //     manager->GetTexture(availableTextures[n]));
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!properties->roughnessTexture) ImGui::BeginDisabled();
    {
        bool removeTex = ImGui::Button("Remove##3");
        ImGui::SameLine();
        bool openInEditor = ImGui::Button("Open in Editor##3");

        if (removeTex)
        {
            //TODO: event
            // selectedMat->ResetroughnessTexture();
        }
        else if (openInEditor)
        {
            // TODO: texture select event
        }
    }
    if (!properties->roughnessTexture) ImGui::EndDisabled();

    ImGui::Text("Roughness:");
    if (properties->roughnessTexture) ImGui::BeginDisabled();
    {
        float roughness = properties->roughness;
        ImGui::SliderFloat("Roughness", &roughness, 0.01f, 1.0f, "%.2f");
        selectedMat->SetRoughness(roughness);
    }
    if (properties->roughnessTexture) ImGui::EndDisabled();
}

void MaterialEditor::PublishMaterialSelectedEvent(
    renderer::Material* mat)
{
    ASSERT(mat != nullptr);

    EventMaterialSelected* event = new EventMaterialSelected();
    event->materialPtr = mat;

    EventQueue::GetInstance()->Publish(EventQueue::Editor,event);
}