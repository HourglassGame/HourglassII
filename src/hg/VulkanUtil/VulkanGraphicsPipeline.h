#ifndef HG_VULKANGRAPHICSPIPELINE_H
#define HG_VULKANGRAPHICSPIPELINE_H
#include <vulkan/vulkan.h>
#include "hg/Util/Maths.h"
#include <vector>
namespace hg {
    extern std::vector<uint32_t> const *demoFragSpv;
    extern std::vector<uint32_t> const *demoVertSpv;
    class VulkanGraphicsPipeline final {
    public:
        VulkanGraphicsPipeline(VkDevice const device) :
            device(device),
            graphicsPipeline(VK_NULL_HANDLE)
        {}
        VulkanGraphicsPipeline(
            VkDevice const device,
            VkExtent2D const swapChainExtent,
            VkPipelineLayout const pipelineLayout,
            VkRenderPass const renderPass
        ) : device(device)
        {
            auto const vertShaderModule{ VulkanShaderModule(device, *demoVertSpv) };
            auto const fragShaderModule{ VulkanShaderModule(device, *demoFragSpv) };

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

            VkPipelineShaderStageCreateInfo const shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


            /*
            https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkVertexInputAttributeDescription.html
            format must be allowed as a vertex buffer format, as specified by the VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT flag
            in VkFormatProperties::bufferFeatures returned by vkGetPhysicalDeviceFormatProperties
            
            TODO: Ensure this is true
            */
            std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

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

            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R8_UINT;
            attributeDescriptions[4].offset = offsetof(Vertex, colourTexture);

            VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.vertexAttributeDescriptionCount = gsl::narrow<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor = {};
            scissor.offset = { 0, 0 };
            scissor.extent = swapChainExtent;

            VkPipelineViewportStateCreateInfo viewportState = {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer = {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling = {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo colorBlending = {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            std::array<VkDynamicState, 1> dynamicStates{
                VK_DYNAMIC_STATE_VIEWPORT
            };
            VkPipelineDynamicStateCreateInfo dynamicState = {};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pNext = nullptr;
            dynamicState.flags = 0;
            dynamicState.dynamicStateCount = gsl::narrow<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();


            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
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

            {
                auto const res{vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create graphics pipeline!"));
                }
            }
        }
        VulkanGraphicsPipeline(VulkanGraphicsPipeline const&) = delete;
        VulkanGraphicsPipeline(VulkanGraphicsPipeline &&o)
            : device(o.device)
            , graphicsPipeline(std::exchange(o.graphicsPipeline, VkPipeline{VK_NULL_HANDLE}))
        {}
        VulkanGraphicsPipeline &operator=(VulkanGraphicsPipeline const&) = delete;
        VulkanGraphicsPipeline &operator=(VulkanGraphicsPipeline &&o) {
            std::swap(device, o.device);
            std::swap(graphicsPipeline, o.graphicsPipeline);
            return *this;
        }
        ~VulkanGraphicsPipeline() noexcept {
            vkDestroyPipeline(device, graphicsPipeline, nullptr);
        }
        VkDevice device;
        VkPipeline graphicsPipeline;
    };
}
#endif // !HG_VULKANGRAPHICSPIPELINE_H
