#ifndef HG_VULKANGRAPHICSPIPELINEHG_H
#define HG_VULKANGRAPHICSPIPELINEHG_H

#include "hg/VulkanUtil/VulkanGraphicsPipeline.h"
#include <vulkan/vulkan.h>
#include "hg/Util/Maths.h"
#include <vector>
#include <array>
namespace hg {
    extern std::vector<uint32_t> const *demoFragSpv;
    extern std::vector<uint32_t> const *demoVertSpv;
    class VulkanGraphicsPipelineHG final {
    public:
        VulkanGraphicsPipelineHG(VkDevice const device) :
            graphicsPipeline(device)
        {}
        VulkanGraphicsPipelineHG(
            VkDevice const device,
            VkExtent2D const swapChainExtent,
            VkPipelineLayout const pipelineLayout,
            VkRenderPass const renderPass
        ) : graphicsPipeline(
                device,
                [pipelineLayout,renderPass](
                    auto const &shaderStages,
                    VkPipelineVertexInputStateCreateInfo const &vertexInputInfo,
                    VkPipelineInputAssemblyStateCreateInfo const &inputAssembly,
                    VkPipelineViewportStateCreateInfo const &viewportState,
                    VkPipelineRasterizationStateCreateInfo const &rasterizer,
                    VkPipelineMultisampleStateCreateInfo const &multisampling,
                    VkPipelineColorBlendStateCreateInfo const &colorBlending,
                    VkPipelineDynamicStateCreateInfo const &dynamicState
                )
                {
                    VkGraphicsPipelineCreateInfo pipelineInfo = {};
                    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                    pipelineInfo.stageCount = gsl::narrow<uint32_t>(shaderStages.size());
                    pipelineInfo.pStages = shaderStages.data();
                    pipelineInfo.pVertexInputState = &vertexInputInfo;
                    pipelineInfo.pInputAssemblyState = &inputAssembly;
                    pipelineInfo.pViewportState = &viewportState;
                    pipelineInfo.pRasterizationState = &rasterizer;
                    pipelineInfo.pMultisampleState = &multisampling;
                    pipelineInfo.pColorBlendState = &colorBlending;
                    pipelineInfo.pDynamicState = &dynamicState;
                    pipelineInfo.layout = pipelineLayout;
                    pipelineInfo.renderPass = renderPass;
                    pipelineInfo.subpass = 0;
                    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

                    return pipelineInfo;
                }(
                    [](auto const &vertShaderModule, auto const &fragShaderModule){
                        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
                        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                        vertShaderStageInfo.module = vertShaderModule.shaderModule;
                        vertShaderStageInfo.pName = "main";

                        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
                        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                        fragShaderStageInfo.module = fragShaderModule.shaderModule;
                        fragShaderStageInfo.pName = "main";

                        return std::array{ vertShaderStageInfo, fragShaderStageInfo };
                    }(VulkanShaderModule(device, *demoVertSpv), VulkanShaderModule(device, *demoFragSpv)),


                    [](auto const &bindingDescriptions, auto const &attributeDescriptions){
                        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
                        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                        vertexInputInfo.vertexBindingDescriptionCount = gsl::narrow<uint32_t>(bindingDescriptions.size());
                        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
                        vertexInputInfo.vertexAttributeDescriptionCount = gsl::narrow<uint32_t>(attributeDescriptions.size());
                        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
                        return vertexInputInfo;
                    }(
                        []{
                            VkVertexInputBindingDescription bindingDescription = {};
                            bindingDescription.binding = 0;
                            bindingDescription.stride = sizeof(Vertex);
                            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                            return std::array{bindingDescription};
                        }(),

                        []{
                            /*
                            https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkVertexInputAttributeDescription.html
                            format must be allowed as a vertex buffer format, as specified by the VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT flag
                            in VkFormatProperties::bufferFeatures returned by vkGetPhysicalDeviceFormatProperties
                            TODO: Ensure this is true
                            */
                            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

                            attributeDescriptions[0].binding = 0;
                            attributeDescriptions[0].location = 0;
                            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
                            attributeDescriptions[0].offset = offsetof(Vertex, pos);

                            attributeDescriptions[1].binding = 0;
                            attributeDescriptions[1].location = 1;
                            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                            attributeDescriptions[1].offset = offsetof(Vertex, colour);

                            attributeDescriptions[2].binding = 0;
                            attributeDescriptions[2].location = 2;
                            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

                            attributeDescriptions[3].binding = 0;
                            attributeDescriptions[3].location = 3;
                            attributeDescriptions[3].format = VK_FORMAT_R8_UINT;
                            attributeDescriptions[3].offset = offsetof(Vertex, useTexture);

                            return attributeDescriptions;
                        }()
                    ),


                    []{
                        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
                        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                        inputAssembly.primitiveRestartEnable = VK_FALSE;
                        return inputAssembly;
                    }(),


                    [](auto const &viewports, auto const &scissors){
                        VkPipelineViewportStateCreateInfo viewportState = {};

                        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                        viewportState.viewportCount = gsl::narrow<uint32_t>(viewports.size());
                        viewportState.pViewports = viewports.data();
                        viewportState.scissorCount = gsl::narrow<uint32_t>(scissors.size());
                        viewportState.pScissors = scissors.data();

                        return viewportState;
                    }(
                        [&swapChainExtent]{
                            VkViewport viewport = {};
                            viewport.x = 0.0f;
                            viewport.y = 0.0f;
                            viewport.width = static_cast<float>(swapChainExtent.width);
                            viewport.height = static_cast<float>(swapChainExtent.height);
                            viewport.minDepth = 0.0f;
                            viewport.maxDepth = 1.0f;
                            return std::array{viewport};
                        }(),

                        [&swapChainExtent]{
                            VkRect2D scissor = {};
                            scissor.offset = { 0, 0 };
                            scissor.extent = swapChainExtent;
                            return std::array{scissor};
                        }()
                    ),

                    []{
                        VkPipelineRasterizationStateCreateInfo rasterizer = {};
                        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                        rasterizer.depthClampEnable = VK_FALSE;
                        rasterizer.rasterizerDiscardEnable = VK_FALSE;
                        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                        rasterizer.lineWidth = 1.0f;
                        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
                        rasterizer.depthBiasEnable = VK_FALSE;
                        return rasterizer;
                    }(),


                    []{
                        VkPipelineMultisampleStateCreateInfo multisampling = {};
                        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                        multisampling.sampleShadingEnable = VK_FALSE;
                        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                        return multisampling;
                    }(),


                    [](auto const &colorBlendAttachments){
                        VkPipelineColorBlendStateCreateInfo colorBlending = {};
                        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                        colorBlending.logicOpEnable = VK_FALSE;
                        colorBlending.logicOp = VK_LOGIC_OP_COPY;
                        colorBlending.attachmentCount = gsl::narrow<uint32_t>(colorBlendAttachments.size());
                        colorBlending.pAttachments = colorBlendAttachments.data();
                        colorBlending.blendConstants[0] = 0.0f;
                        colorBlending.blendConstants[1] = 0.0f;
                        colorBlending.blendConstants[2] = 0.0f;
                        colorBlending.blendConstants[3] = 0.0f;
                        return colorBlending;
                    }([]{
                        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
                        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                        colorBlendAttachment.blendEnable = VK_TRUE;
                        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

                        return std::array{colorBlendAttachment};
                    }()),


                    [](auto const &dynamicStates){
                        VkPipelineDynamicStateCreateInfo dynamicState = {};
                        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                        dynamicState.pNext = nullptr;
                        dynamicState.flags = 0;
                        dynamicState.dynamicStateCount = gsl::narrow<uint32_t>(dynamicStates.size());
                        dynamicState.pDynamicStates = dynamicStates.data();
                        return dynamicState;
                    }([]{
                        return std::array{
                            VK_DYNAMIC_STATE_VIEWPORT
                        };
                    }())
                )
            )
        {
        }
        VulkanGraphicsPipelineHG(VulkanGraphicsPipelineHG const&) = delete;
        VulkanGraphicsPipelineHG(VulkanGraphicsPipelineHG &&o) noexcept = default;
        VulkanGraphicsPipelineHG &operator=(VulkanGraphicsPipelineHG const&) = delete;
        VulkanGraphicsPipelineHG &operator=(VulkanGraphicsPipelineHG &&o) noexcept = default;

        VkPipeline h() const noexcept {
            return graphicsPipeline.graphicsPipeline;
        }
    private:
        VulkanGraphicsPipeline graphicsPipeline;
    };
}
#endif // !HG_VULKANGRAPHICSPIPELINEHG_H
