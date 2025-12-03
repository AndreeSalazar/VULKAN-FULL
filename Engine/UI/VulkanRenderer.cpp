#include "VulkanRenderer.h"
#include "../Core/Log.h"
#include "engine_ui_ffi.h"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace UI {

// Helper function to read file
static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }
    
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

// UIVertex está definido en engine_ui_ffi.h (desde Rust)

VulkanRenderer::VulkanRenderer() {
}

VulkanRenderer::~VulkanRenderer() {
    Shutdown();
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                                  VkMemoryPropertyFlags properties, 
                                  VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

bool VulkanRenderer::Initialize(
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkQueue graphicsQueue,
    uint32_t graphicsQueueFamilyIndex,
    VkRenderPass renderPass,
    VkDescriptorPool descriptorPool,
    VkFormat imageFormat,
    VkSampleCountFlagBits msaaSamples
) {
    if (bInitialized) {
        UE_LOG_WARNING(LogCategories::UI, "VulkanRenderer already initialized");
        return true;
    }
    
    this->instance = instance;
    this->physicalDevice = physicalDevice;
    this->device = device;
    this->graphicsQueue = graphicsQueue;
    this->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    this->renderPass = renderPass;
    this->descriptorPool = descriptorPool;
    
    // Crear command pool para upload de texturas
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool for UI!");
    }
    
    try {
        createPipeline(imageFormat, msaaSamples);
        createFontTexture();
        
        // Intentar actualizar la textura de fuente profesional durante la inicialización
        // Si no está disponible aún, se actualizará después del primer render de eGUI
        FontTextureData fontTexData{};
        if (egui_get_font_texture_data(&fontTexData) && fontTexData.has_data && 
            fontTexData.width > 0 && fontTexData.height > 0) {
            // Actualizar con la textura profesional real si está disponible
            UE_LOG_INFO(LogCategories::UI, "Updating PROFESSIONAL font texture during initialization: %ux%u", fontTexData.width, fontTexData.height);
            updateFontTexture(fontTexData.pixels_ptr, fontTexData.width, fontTexData.height);
        } else {
            UE_LOG_INFO(LogCategories::UI, "Professional font texture not ready yet, will be updated after first render");
        }
        
        bInitialized = true;
        UE_LOG_INFO(LogCategories::UI, "VulkanRenderer initialized successfully");
        return true;
    } catch (const std::exception& e) {
        UE_LOG_ERROR(LogCategories::UI, "Failed to initialize VulkanRenderer: %s", e.what());
        return false;
    }
}

void VulkanRenderer::createPipeline(VkFormat imageFormat, VkSampleCountFlagBits msaaSamples) {
    // Load shaders
    auto vertShaderCode = readFile("shaders/ui_vert.spv");
    auto fragShaderCode = readFile("shaders/ui_frag.spv");
    
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Vertex input (usando UIVertex del FFI)
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(struct UIVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(struct UIVertex, pos);
    
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(struct UIVertex, tex_coord);
    
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32_UINT; // uint32 para el color empaquetado
    attributeDescriptions[2].offset = offsetof(struct UIVertex, color);
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport (dynamic)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 1920.0f; // Will be set dynamically
    viewport.height = 1080.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {1920, 1080}; // Will be set dynamically
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    // Dynamic state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Rasterizer (UI debe renderizarse sin culling)
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; // Sin culling para UI
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Cambiar a counter-clockwise
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;
    
    // Depth stencil (UI no necesita depth testing)
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    
    // Color blending (alpha blending for UI)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Mantener alpha de la fuente
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    // Descriptor set layout for font texture
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;
    
    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    
    // Push constants for transform
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 4; // vec2 scale + vec2 translate
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    
    // Store descriptor set layout for later use
    this->descriptorSetLayout = descriptorSetLayout;
    
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    
    return shaderModule;
}

void VulkanRenderer::createFontTexture(uint32_t width, uint32_t height) {
    // Crear textura de fuente profesional
    // Si width/height son 0, intentar obtener el tamaño real de eGUI
    uint32_t texWidth = width;
    uint32_t texHeight = height;
    
    if (texWidth == 0 || texHeight == 0) {
        // Intentar obtener el tamaño real de eGUI
        FontTextureData fontTexData{};
        if (egui_get_font_texture_data(&fontTexData) && fontTexData.has_data && 
            fontTexData.width > 0 && fontTexData.height > 0) {
            texWidth = fontTexData.width;
            texHeight = fontTexData.height;
            UE_LOG_INFO(LogCategories::UI, "Creating PROFESSIONAL font texture with real size from eGUI: %ux%u", texWidth, texHeight);
        } else {
            texWidth = 512;  // Tamaño profesional inicial
            texHeight = 512;
            UE_LOG_INFO(LogCategories::UI, "Font texture not ready yet, creating initial %ux%u (will be updated with real professional texture after first render)", texWidth, texHeight);
        }
    } else {
        UE_LOG_INFO(LogCategories::UI, "Creating font texture with specified size: %ux%u", texWidth, texHeight);
    }
    
    // Guardar el tamaño
    fontImageWidth = texWidth;
    fontImageHeight = texHeight;
    
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateImage(device, &imageInfo, nullptr, &fontImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create font image!");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, fontImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &fontImageMemory) != VK_SUCCESS) {
        vkDestroyImage(device, fontImage, nullptr);
        throw std::runtime_error("failed to allocate font image memory!");
    }
    
    vkBindImageMemory(device, fontImage, fontImageMemory, 0);
    
    // IMPORTANTE: Transicionar la imagen al layout correcto antes de usarla
    // Necesitamos un command buffer temporal para esto
    VkCommandBufferAllocateInfo cmdAllocInfo{};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandPool = commandPool;
    cmdAllocInfo.commandBufferCount = 1;
    
    VkCommandBuffer transitionCmdBuffer;
    vkAllocateCommandBuffers(device, &cmdAllocInfo, &transitionCmdBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(transitionCmdBuffer, &beginInfo);
    
    // Transicionar de UNDEFINED a SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = fontImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(transitionCmdBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    vkEndCommandBuffer(transitionCmdBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transitionCmdBuffer;
    
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &transitionCmdBuffer);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = fontImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(device, &viewInfo, nullptr, &fontImageView) != VK_SUCCESS) {
        vkFreeMemory(device, fontImageMemory, nullptr);
        vkDestroyImage(device, fontImage, nullptr);
        throw std::runtime_error("failed to create font image view!");
    }
    
    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST; // Sin mipmaps para texto
    
    if (vkCreateSampler(device, &samplerInfo, nullptr, &fontSampler) != VK_SUCCESS) {
        vkDestroyImageView(device, fontImageView, nullptr);
        vkFreeMemory(device, fontImageMemory, nullptr);
        vkDestroyImage(device, fontImage, nullptr);
        throw std::runtime_error("failed to create font sampler!");
    }
    
    // IMPORTANTE: La imagen debe estar en el layout correcto antes de crear el descriptor set
    // La imagen está en VK_IMAGE_LAYOUT_UNDEFINED, la transicionaremos a SHADER_READ_ONLY_OPTIMAL
    // después de copiar los datos de la textura profesional
    
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocDescInfo{};
    allocDescInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocDescInfo.descriptorPool = descriptorPool;
    allocDescInfo.descriptorSetCount = 1;
    allocDescInfo.pSetLayouts = &descriptorSetLayout;
    
    VkResult allocResult = vkAllocateDescriptorSets(device, &allocDescInfo, &fontDescriptorSet);
    if (allocResult != VK_SUCCESS) {
        UE_LOG_ERROR(LogCategories::UI, "Failed to allocate descriptor set: %d", allocResult);
        vkDestroySampler(device, fontSampler, nullptr);
        vkDestroyImageView(device, fontImageView, nullptr);
        vkFreeMemory(device, fontImageMemory, nullptr);
        vkDestroyImage(device, fontImage, nullptr);
        throw std::runtime_error("failed to allocate descriptor set!");
    }
    
    // Update descriptor set - IMPORTANTE: La imagen debe estar en SHADER_READ_ONLY_OPTIMAL
    // La imagen se transicionará al layout correcto cuando se actualice con la textura profesional
    VkDescriptorImageInfo imageInfoDesc{};
    imageInfoDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoDesc.imageView = fontImageView;
    imageInfoDesc.sampler = fontSampler;
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = fontDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfoDesc;
    
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    
    UE_LOG_INFO(LogCategories::UI, "Font descriptor set allocated and updated successfully");
}

void VulkanRenderer::ensureBufferSize(VkBuffer& buffer, VkDeviceMemory& bufferMemory,
                                     size_t& currentSize, size_t requiredSize,
                                     VkBufferUsageFlags usage) {
    if (requiredSize <= currentSize && buffer != VK_NULL_HANDLE) {
        return; // Buffer ya es suficientemente grande y existe
    }
    
    // Destruir buffer anterior si existe
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
    if (bufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, bufferMemory, nullptr);
        bufferMemory = VK_NULL_HANDLE;
    }
    
    // Crear nuevo buffer más grande (redondear a múltiplo de 1024)
    // Mínimo 1KB para evitar buffers de tamaño 0
    size_t newSize = std::max(((requiredSize + 1023) / 1024) * 1024, size_t(1024));
    try {
        createBuffer(newSize, usage, 
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     buffer, bufferMemory);
        currentSize = newSize;
        static bool firstBuffer = true;
        if (firstBuffer) {
            UE_LOG_INFO(LogCategories::UI, "Created UI buffer: size=%zu bytes, usage=0x%x", newSize, usage);
            firstBuffer = false;
        }
    } catch (const std::exception& e) {
        UE_LOG_ERROR(LogCategories::UI, "Failed to create buffer: %s", e.what());
        throw;
    }
}

void VulkanRenderer::updateBuffers(const void* vertices, size_t vertexCount, 
                                   const void* indices, size_t indexCount) {
    if (vertexCount == 0 || indexCount == 0) {
        static bool firstWarning = true;
        if (firstWarning) {
            UE_LOG_WARNING(LogCategories::UI, "updateBuffers: vertexCount=%zu or indexCount=%zu is 0", vertexCount, indexCount);
            firstWarning = false;
        }
        return;
    }
    
    size_t vertexSize = vertexCount * sizeof(UIVertex);
    size_t indexSize = indexCount * sizeof(uint32_t);
    
    // Asegurar que los buffers sean lo suficientemente grandes
    ensureBufferSize(vertexBuffer, vertexBufferMemory, vertexBufferSize, vertexSize,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    ensureBufferSize(indexBuffer, indexBufferMemory, indexBufferSize, indexSize,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    
    // Verificar que los buffers fueron creados correctamente
    if (vertexBuffer == VK_NULL_HANDLE || indexBuffer == VK_NULL_HANDLE) {
        UE_LOG_ERROR(LogCategories::UI, "updateBuffers: Buffers are null! vertexBuffer=%p, indexBuffer=%p", 
                     (void*)vertexBuffer, (void*)indexBuffer);
        return;
    }
    
    // Mapear memoria y copiar datos
    void* vertexData;
    VkResult result = vkMapMemory(device, vertexBufferMemory, 0, vertexSize, 0, &vertexData);
    if (result != VK_SUCCESS) {
        UE_LOG_ERROR(LogCategories::UI, "updateBuffers: Failed to map vertex memory: %d", result);
        return;
    }
    memcpy(vertexData, vertices, vertexSize);
    vkUnmapMemory(device, vertexBufferMemory);
    
    void* indexData;
    result = vkMapMemory(device, indexBufferMemory, 0, indexSize, 0, &indexData);
    if (result != VK_SUCCESS) {
        UE_LOG_ERROR(LogCategories::UI, "updateBuffers: Failed to map index memory: %d", result);
        return;
    }
    memcpy(indexData, indices, indexSize);
    vkUnmapMemory(device, indexBufferMemory);
}

void VulkanRenderer::Render(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t width, uint32_t height) {
    if (!bInitialized) {
        static bool firstWarning = true;
        if (firstWarning) {
            UE_LOG_WARNING(LogCategories::UI, "VulkanRenderer::Render called but not initialized!");
            firstWarning = false;
        }
        return;
    }
    
    // Verificar que el command buffer es válido
    if (commandBuffer == VK_NULL_HANDLE) {
        static bool firstError = true;
        if (firstError) {
            UE_LOG_ERROR(LogCategories::UI, "Render: commandBuffer is null!");
            firstError = false;
        }
        return;
    }
    
    // ===== DETECTAR TEXTURA DE FUENTE PROFESIONAL (NO ACTUALIZAR AQUÍ) =====
    // NO podemos actualizar la textura aquí porque estamos dentro de un command buffer
    // En su lugar, solo detectamos si hay una nueva textura y la marcamos para actualización
    // La actualización real se hará después del frame (fuera del command buffer)
    static bool fontTextureDetected = false;
    if (!fontTextureDetected && !pendingFontTextureUpdate) {
        FontTextureData fontTexData{};
        if (egui_get_font_texture_data(&fontTexData) && fontTexData.has_data && 
            fontTexData.width > 0 && fontTexData.height > 0) {
            // Marcar que hay una textura profesional disponible
            // La actualizaremos después del frame usando UpdateFontTextureIfNeeded()
            pendingFontTextureUpdate = true;
            pendingFontTextureData = fontTexData;
            fontTextureDetected = true;
            UE_LOG_INFO(LogCategories::UI, "Detected PROFESSIONAL font texture: %ux%u (will update after frame)", 
                       fontTexData.width, fontTexData.height);
        }
    }
    
    // TEST: Crear UI de prueba hardcodeada para verificar que el pipeline funciona
    static bool testMode = false; // ✅ DESACTIVADO - usar datos reales de eGUI
    static bool testModeLogged = false;
    
    if (testMode) {
        // Crear un rectángulo que cubra TODO EL VIEWPORT para asegurar visibilidad
        // Coordenadas en screen space - cubrir toda la pantalla (con margen pequeño)
        float margin = 50.0f;
        float x1 = margin;                    // Margen izquierdo
        float y1 = margin;                    // Margen superior
        float x2 = width - margin;            // Margen derecho
        float y2 = height - margin;           // Margen inferior
        
        struct UIVertex testVerts[] = {
            {{x1, y1}, {0.0f, 0.0f}, 0xFF0000FF}, // ROJO: ARGB = A(FF) R(FF) G(00) B(00) = 0xFF0000FF
            {{x2, y1}, {1.0f, 0.0f}, 0xFF0000FF},
            {{x2, y2}, {1.0f, 1.0f}, 0xFF0000FF},
            {{x1, y2}, {0.0f, 1.0f}, 0xFF0000FF},
        };
        // Usar uint16_t para que coincida con VK_INDEX_TYPE_UINT16
        uint16_t testIndices[] = {
            0, 1, 2,  // Primer triángulo
            0, 2, 3   // Segundo triángulo
        };
        
        if (!testModeLogged) {
            UE_LOG_INFO(LogCategories::UI, "🧪 TEST MODE: Rectángulo ROJO en (%.0f,%.0f)-(%.0f,%.0f), screen=%ux%u", 
                       x1, y1, x2, y2, width, height);
            testModeLogged = true;
        }
        
        updateBuffers(testVerts, 4, testIndices, 6);
    } else {
        // Modo normal: usar datos de eGUI
        // NOTA: NO actualizar la textura durante el renderizado - esto causa problemas
        // La textura se actualiza solo una vez durante la inicialización
        // Si necesitamos actualizar la textura, debe hacerse fuera del command buffer recording
        
        // Obtener datos de renderizado desde Rust
        RenderData renderData{};
        if (!egui_get_render_data(&renderData)) {
            // No hay nada que renderizar
            static bool firstSkip = true;
            if (firstSkip) {
                UE_LOG_WARNING(LogCategories::UI, "VulkanRenderer::Render: No render data available");
                firstSkip = false;
            }
            return;
        }
        
        // Usar datos de eGUI
        updateBuffers(renderData.vertices_ptr, renderData.vertices_count,
                      renderData.indices_ptr, renderData.indices_count);
        
        if (renderData.vertices_count == 0 || renderData.indices_count == 0) {
            return;
        }
        
        // DEBUG: Log de algunos vértices para verificar coordenadas
        static bool firstVertexLog = true;
        if (firstVertexLog && renderData.vertices_count > 0) {
            const struct UIVertex* verts = static_cast<const struct UIVertex*>(renderData.vertices_ptr);
            UE_LOG_INFO(LogCategories::UI, "=== PRIMEROS VÉRTICES DE eGUI ===");
            for (size_t i = 0; i < std::min(renderData.vertices_count, size_t(10)); i++) {
                uint32_t c = verts[i].color;
                float r = float((c >> 0) & 0xFF) / 255.0f;
                float g = float((c >> 8) & 0xFF) / 255.0f;
                float b = float((c >> 16) & 0xFF) / 255.0f;
                float a = float((c >> 24) & 0xFF) / 255.0f;
                UE_LOG_INFO(LogCategories::UI, "  V[%zu]: pos=(%.1f, %.1f), RGBA=(%.2f, %.2f, %.2f, %.2f)", 
                           i, verts[i].pos[0], verts[i].pos[1], r, g, b, a);
            }
            firstVertexLog = false;
        }
    }
    
    // Determinar indexCount según el modo
    uint32_t indexCount = 0;
    if (testMode) {
        indexCount = 6; // 2 triángulos para el rectángulo de prueba
    } else {
        RenderData renderData{};
        if (egui_get_render_data(&renderData)) {
            indexCount = static_cast<uint32_t>(renderData.indices_count);
        } else {
            return; // No hay datos
        }
    }
    
    if (indexCount == 0) {
        return;
    }
    
    // Verificar que el pipeline existe
    if (graphicsPipeline == VK_NULL_HANDLE) {
        static bool firstError = true;
        if (firstError) {
            UE_LOG_ERROR(LogCategories::UI, "Render: graphicsPipeline is null!");
            firstError = false;
        }
        return;
    }
    
    // Bind pipeline UI (IMPORTANTE: esto sobrescribe el pipeline del cubo)
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    
    static bool pipelineBoundLogged = false;
    if (testMode && !pipelineBoundLogged) {
        UE_LOG_INFO(LogCategories::UI, "🧪 Pipeline UI bound (esto sobrescribe el pipeline del cubo)");
        pipelineBoundLogged = true;
    }
    
    // Set viewport and scissor (override cube's settings)
    // Standard viewport configuration for UI rendering
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {width, height};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    static bool viewportLogged = false;
    if (!viewportLogged) {
        UE_LOG_INFO(LogCategories::UI, "🔍 Viewport UI: (%u, %u), Scissor: (%u, %u)", width, height, width, height);
        viewportLogged = true;
    }
    
    // Push constants: Screen space to clip space transformation
    // eGUI screen space: (0,0) = top-left, Y increases downward, X increases rightward
    // Vulkan clip space: (-1,-1) = bottom-left, (1,1) = top-right, Y increases upward
    //
    // Standard transformation formula:
    // X: [0, width]  -> [-1, 1]  => clipX = (x / width) * 2.0 - 1.0 = (x * 2.0/width) - 1.0
    // Y: [0, height] -> [1, -1]  => clipY = 1.0 - (y / height) * 2.0 = (y * -2.0/height) + 1.0
    //
    // In form: clip = pos * scale + translate
    // scaleX = 2.0 / width,  translateX = -1.0
    // scaleY = -2.0 / height, translateY = 1.0
    float scaleX = 2.0f / static_cast<float>(width);
    float scaleY = -2.0f / static_cast<float>(height); // Negative to flip Y axis
    float translateX = -1.0f;
    float translateY = 1.0f;
    
    // IMPORTANTE: El shader espera vec2 scale y vec2 translate
    // En memoria GLSL: vec2 scale (8 bytes), vec2 translate (8 bytes)
    // Orden: [scale.x, scale.y, translate.x, translate.y]
    // Usar array para asegurar el orden correcto en memoria
    float pushConstants[4] = {scaleX, scaleY, translateX, translateY};
    
    // Debug: Log transformation parameters (only once)
    static bool transformLogged = false;
    if (!transformLogged) {
        UE_LOG_INFO(LogCategories::UI, "UI Transform: scale=(%.6f, %.6f), translate=(%.6f, %.6f), viewport=(%u, %u)",
                   scaleX, scaleY, translateX, translateY, width, height);
        
        // Verify corner transformations
        float testX = 0.0f, testY = 0.0f;
        float clipX = testX * scaleX + translateX;
        float clipY = testY * scaleY + translateY;
        UE_LOG_INFO(LogCategories::UI, "  Corner (0,0) -> clip (%.3f, %.3f) [expected: (-1, 1)]", clipX, clipY);
        
        testX = static_cast<float>(width);
        testY = static_cast<float>(height);
        clipX = testX * scaleX + translateX;
        clipY = testY * scaleY + translateY;
        UE_LOG_INFO(LogCategories::UI, "  Corner (%u,%u) -> clip (%.3f, %.3f) [expected: (1, -1)]", width, height, clipX, clipY);
        
        transformLogged = true;
    }
    
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 
                      0, sizeof(pushConstants), pushConstants);
    
    // Bind descriptor set (requerido por el pipeline layout)
    // El fragment shader usará la textura, pero para colores rojos la ignorará
    if (fontDescriptorSet == VK_NULL_HANDLE) {
        static bool firstError = true;
        if (firstError) {
            UE_LOG_ERROR(LogCategories::UI, "Render: fontDescriptorSet is null!");
            firstError = false;
        }
        return;
    }
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipelineLayout, 0, 1, &fontDescriptorSet, 0, nullptr);
    
    // Verificar que los buffers existen antes de bindearlos
    if (vertexBuffer == VK_NULL_HANDLE || indexBuffer == VK_NULL_HANDLE) {
        static bool firstError = true;
        if (firstError) {
            UE_LOG_ERROR(LogCategories::UI, "Render: Buffers are null! vertexBuffer=%p, indexBuffer=%p", 
                         (void*)vertexBuffer, (void*)indexBuffer);
            firstError = false;
        }
        return;
    }
    
    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    // Bind index buffer (eGUI usa uint32, así que usar UINT32)
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    // Draw
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    
    static bool firstDraw = true;
    if (firstDraw) {
        if (testMode) {
            UE_LOG_INFO(LogCategories::UI, "🧪 TEST MODE: Dibujando 6 índices (rectángulo rojo)");
            UE_LOG_INFO(LogCategories::UI, "   Si ves un rectángulo rojo, el pipeline funciona!");
        } else {
            UE_LOG_INFO(LogCategories::UI, "✅ DRAW CALL: %u indices (~%u triangles)", indexCount, indexCount / 3);
        }
        UE_LOG_INFO(LogCategories::UI, "   Viewport: %ux%u | Pipeline bound", width, height);
        firstDraw = false;
    }
}

void VulkanRenderer::updateFontTexture(const void* pixels, uint32_t width, uint32_t height) {
    if (!pixels || width == 0 || height == 0) {
        UE_LOG_WARNING(LogCategories::UI, "updateFontTexture: Invalid parameters. W:%u, H:%u", width, height);
        return;
    }
    
    // Si la textura no existe o el tamaño cambió, necesitamos recrearla
    bool needsRecreation = (fontImage == VK_NULL_HANDLE) || 
                           (fontImageWidth != width || fontImageHeight != height);
    
    if (needsRecreation) {
        UE_LOG_INFO(LogCategories::UI, "Font texture needs recreation: current=%ux%u, new=%ux%u", 
                   fontImageWidth, fontImageHeight, width, height);
        
        // Esperar a que el dispositivo termine cualquier operación pendiente
        vkDeviceWaitIdle(device);
        
        // Destruir recursos antiguos
        if (fontSampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, fontSampler, nullptr);
            fontSampler = VK_NULL_HANDLE;
        }
        if (fontImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, fontImageView, nullptr);
            fontImageView = VK_NULL_HANDLE;
        }
        if (fontImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, fontImage, nullptr);
            fontImage = VK_NULL_HANDLE;
        }
        if (fontImageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, fontImageMemory, nullptr);
            fontImageMemory = VK_NULL_HANDLE;
        }
        
        // Recrear con el nuevo tamaño
        createFontTexture(width, height);
        
        // Actualizar el descriptor set con la nueva imagen
        VkDescriptorImageInfo imageInfoDesc{};
        imageInfoDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfoDesc.imageView = fontImageView;
        imageInfoDesc.sampler = fontSampler;
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = fontDescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfoDesc;
        
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        
        UE_LOG_INFO(LogCategories::UI, "Font texture recreated and descriptor set updated: %ux%u", width, height);
    }
    
    // Ahora actualizar la textura con los nuevos datos
    UE_LOG_INFO(LogCategories::UI, "Updating font texture data: %ux%u", width, height);
    
    // Crear staging buffer
    VkDeviceSize imageSize = width * height * 4; // RGBA
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);
    
    // Copiar datos a staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    // Command buffer para upload
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer uploadCommandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &uploadCommandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(uploadCommandBuffer, &beginInfo);
    
    // Transition image a TRANSFER_DST
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = fontImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
    vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
                        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    // Copy buffer to image
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    // IMPORTANTE: Verificar que el extent no exceda el tamaño de la imagen
    // Por ahora asumimos que la imagen fue creada con el tamaño correcto
    region.imageExtent = {width, height, 1};
    
    // Verificar que el tamaño es válido
    if (width == 0 || height == 0 || width > 4096 || height > 4096) {
        UE_LOG_ERROR(LogCategories::UI, "Invalid font texture size: %ux%u", width, height);
        vkEndCommandBuffer(uploadCommandBuffer);
        vkFreeCommandBuffers(device, commandPool, 1, &uploadCommandBuffer);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        return;
    }
    
    vkCmdCopyBufferToImage(uploadCommandBuffer, stagingBuffer, fontImage, 
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    // Transition back to SHADER_READ_ONLY
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 
                        0, nullptr, 1, &barrier);
    
    vkEndCommandBuffer(uploadCommandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &uploadCommandBuffer;
    
    // Usar un fence en lugar de vkQueueWaitIdle para ser más seguro
    VkFence uploadFence = VK_NULL_HANDLE;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(device, &fenceInfo, nullptr, &uploadFence) != VK_SUCCESS) {
        UE_LOG_ERROR(LogCategories::UI, "Failed to create upload fence, falling back to vkQueueWaitIdle");
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
    } else {
        // Submit con fence
        VkResult submitResult = vkQueueSubmit(graphicsQueue, 1, &submitInfo, uploadFence);
        if (submitResult != VK_SUCCESS) {
            UE_LOG_ERROR(LogCategories::UI, "vkQueueSubmit failed in updateFontTexture: %d", submitResult);
            vkDestroyFence(device, uploadFence, nullptr);
            vkFreeCommandBuffers(device, commandPool, 1, &uploadCommandBuffer);
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            return;
        }
        
        // Esperar al fence con timeout (máximo 1 segundo)
        VkResult waitResult = vkWaitForFences(device, 1, &uploadFence, VK_TRUE, 1000000000); // 1 segundo en nanosegundos
        if (waitResult != VK_SUCCESS) {
            UE_LOG_WARNING(LogCategories::UI, "vkWaitForFences timeout or error in updateFontTexture: %d", waitResult);
        }
        
        vkDestroyFence(device, uploadFence, nullptr);
    }
    
    vkFreeCommandBuffers(device, commandPool, 1, &uploadCommandBuffer);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::UpdateFontTextureIfNeeded() {
    // Este método debe llamarse DESPUÉS del frame, fuera del command buffer
    // Actualiza la textura de fuente si hay una actualización pendiente
    if (!pendingFontTextureUpdate) {
        return;
    }
    
    if (!pendingFontTextureData.has_data || 
        pendingFontTextureData.width == 0 || 
        pendingFontTextureData.height == 0) {
        pendingFontTextureUpdate = false;
        return;
    }
    
    UE_LOG_INFO(LogCategories::UI, "Updating PROFESSIONAL font texture after frame: %ux%u", 
               pendingFontTextureData.width, pendingFontTextureData.height);
    
    // Ahora sí podemos actualizar la textura de forma segura (fuera del command buffer)
    updateFontTexture(pendingFontTextureData.pixels_ptr, 
                     pendingFontTextureData.width, 
                     pendingFontTextureData.height);
    
    // Marcar como completado
    pendingFontTextureUpdate = false;
}

void VulkanRenderer::Shutdown() {
    if (!bInitialized) return;
    
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);
        
        if (fontSampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, fontSampler, nullptr);
        }
        if (fontImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, fontImageView, nullptr);
        }
        if (fontImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, fontImage, nullptr);
        }
        if (fontImageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, fontImageMemory, nullptr);
        }
        if (vertexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, vertexBuffer, nullptr);
        }
        if (vertexBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, vertexBufferMemory, nullptr);
        }
        if (indexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, indexBuffer, nullptr);
        }
        if (indexBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, indexBufferMemory, nullptr);
        }
        if (graphicsPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, graphicsPipeline, nullptr);
        }
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        }
        if (descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        }
        if (commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, commandPool, nullptr);
        }
    }
    
    bInitialized = false;
}

} // namespace UI

