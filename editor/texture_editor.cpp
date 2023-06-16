#include "texture_editor.h"

#include "event_queue.h"
#include "events.h"
#include "vulkan_renderer.h"
#include "asset_manager.h"
#include "validation.h"

#include <imgui.h>


TextureEditor::TextureEditor()
{
    subscriberHandle = EventQueue::GetInstance()->Subscribe(EventQueue::Editor,
        [this](Event* event){
            if (event->type == Event::Type::TextureSelected)
            {
                EventTextureSelected* e =
                    dynamic_cast<EventTextureSelected*>(event);
                this->selectedTexture =
                    reinterpret_cast<renderer::VulkanTexture*>(e->texturePtr);

                renderer::VulkanRenderer& vkr =
                    renderer::VulkanRenderer::GetInstance();

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = this->imageDescSet;
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = this->selectedTexture->GetDescriptor();

                vkUpdateDescriptorSets(
                    vkr.vulkanDevice.vkDevice, 1, &descriptorWrite, 0, nullptr);   
            }
            else if (event->type == Event::Type::ProjectOpen)
            {
                EventProjectOpen* e = dynamic_cast<EventProjectOpen*>(event);
                this->assetManager = reinterpret_cast<AssetManager*>(e->assetManager);
            }
            else if (event->type == Event::Type::CloseProject)
            {
                this->selectedTexture = nullptr;
                this->availableTexureCached = false;
                this->availableTextures.clear();
                this->assetManager = nullptr;
            }
            else if (event->type == Event::Type::WorkspaceChanged)
            {
                this->availableTextures.clear();
                this->availableTexureCached = false;
            }
        });

    {
        renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();
        VulkanPipelineLayout& layout = vkr.GetPipelineLayout("display");
        layout.AllocateDescriptorSet("texture",
            vkr.FRAME_IN_FLIGHT, &imageDescSet);
    }
}

TextureEditor::~TextureEditor()
{
    EventQueue::GetInstance()->Unsubscribe(subscriberHandle);
}

void TextureEditor::Draw()
{

    ASSERT(assetManager != nullptr);
    ImGui::Begin("Texture Editor", nullptr);
    ImGui::Text("Properties of the selected editor");
    ImGui::Separator();

    ShowContent();

    ImGui::End();
}

void TextureEditor::ShowContent()
{
    ShowTextureSelection();
    
    if (!selectedTexture)
        return;

    ShowTexturePropertiesSection();
    ShowTexturePreviewSection();
}

void TextureEditor::ShowTextureSelection()
{

    if (!availableTexureCached)
    {
        assetManager->GetAvailableTextures(availableTextures);
        availableTexureCached = true;
    }

    ImGui::Text("Texture:");

    std::string currentTextureStr;
    const char* currentTexturePath;
    if (selectedTexture)
    {
        currentTextureStr = selectedTexture->GetBuildInfo().resourcePath; 
    }
    else
    {
        currentTextureStr = "None";
    }

    currentTexturePath = currentTextureStr.c_str();
    if (ImGui::BeginCombo("Path##1", currentTexturePath))
    {
        for (int n = 0; n < availableTextures.size(); n++)
        {
            const bool isSelected =
                strcmp(availableTextures[n], currentTexturePath) == 0;
            if (ImGui::Selectable(availableTextures[n], isSelected))
            {
                std::shared_ptr<renderer::Texture> texture =
                    assetManager->GetTexture(availableTextures[n]);
                PublishTextureSelectedEvent(texture.get());
            }

            // Set the initial focus to the selected mesh
            // when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void TextureEditor::ShowTexturePropertiesSection()
{
    const renderer::TextureBuildInfo& info =
        selectedTexture->GetBuildInfo();

    ImGui::SeparatorText("Properties");
    {
        std::string addressMode;
        std::string minFilter;
        std::string maxFilter;

        switch (info.addressMode)
        {
        case renderer::REPEAT:
            addressMode = "Repeated";
            break;
        case renderer::MIRRORED_REPEAT:
            addressMode = "Mirrored repeated";
            break;
        case renderer::CLAMP_TO_EDGE:
            addressMode = "Clamp to edge";
            break;
        case renderer::CLAMP_TO_BORDER:
            addressMode = "Clamp to border";
            break;
        default:
            throw;
        }

        switch (info.minFilter)
        {
        case renderer::FILTER_LINEAR:
            minFilter = "Linear";
            break;
        case renderer::FILTER_NEAREST:
            minFilter = "Nearest";
            break;
        default:
            throw;
        }

        switch (info.maxFilter)
        {
        case renderer::FILTER_LINEAR:
            maxFilter = "Linear";
            break;
        case renderer::FILTER_NEAREST:
            maxFilter = "Nearest";
            break;
        default:
            throw;
        }

        ImGui::Text("Address mode:");
        ImGui::SameLine(150); ImGui::Text(addressMode.c_str());
        ImGui::Text("Min filter:");
        ImGui::SameLine(150); ImGui::Text(minFilter.c_str());
        ImGui::Text("Max filter:");
        ImGui::SameLine(150); ImGui::Text(maxFilter.c_str());
    }
}

void TextureEditor::ShowTexturePreviewSection()
{
    ImGui::SeparatorText("Preview");
    {
        ImVec2 contentExtent = ImGui::GetWindowSize();
        glm::vec2 imageExtent = selectedTexture->GetExtent();

        float heightRatio = imageExtent.y / contentExtent.y;
        float widthRatio = imageExtent.x / contentExtent.x;

        if (heightRatio > widthRatio)
        {
            imageExtent.x /= heightRatio;
            imageExtent.y /= heightRatio;
        }
        else
        {
            imageExtent.x /= widthRatio;
            imageExtent.y /= widthRatio;
        }
        
        ImGui::SetCursorPos(ImVec2(
            (contentExtent.x - imageExtent.x) * 0.5f + ImGui::GetCursorPos().x, 
            ImGui::GetCursorPos().y));

        ImGui::Image(imageDescSet, {imageExtent.x, imageExtent.y});
    }
}

void TextureEditor::PublishTextureSelectedEvent(
    renderer::Texture* tex)
{
    ASSERT(tex != nullptr);

    EventTextureSelected* event = new EventTextureSelected();
    event->texturePtr = tex;

    EventQueue::GetInstance()->Publish(EventQueue::Editor, event);
}