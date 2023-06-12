
cd /Users/zekailin00/Git/Vulkan-Renderer/resources/vulkan_shaders/Phong
glslc shader.frag -o frag.spv
glslc shader.vert -o vert.spv

cd /Users/zekailin00/Git/Vulkan-Renderer/resources/vulkan_shaders/transfer
glslc shader.frag -o frag.spv
glslc shader.vert -o vert.spv

cd /Users/zekailin00/Git/Vulkan-Renderer/resources/vulkan_shaders/skybox
glslc shader.frag -o frag.spv
glslc shader.vert -o vert.spv

cd /Users/zekailin00/Git/Vulkan-Renderer/resources/vulkan_shaders/wire
glslc shader.frag -o frag.spv
glslc shader.vert -o vert.spv

cd /Users/zekailin00/Git/Vulkan-Renderer/resources/vulkan_shaders/imgui
glslc shader.frag -o frag.spv
glslc shader.vert -o vert.spv


cd /Users/zekailin00/Git/Vulkan-Renderer
rm -rf build