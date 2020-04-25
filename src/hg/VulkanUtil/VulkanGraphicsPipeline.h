#ifndef HG_VULKANGRAPHICSPIPELINE_H
#define HG_VULKANGRAPHICSPIPELINE_H

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>

namespace hg {
    class VulkanGraphicsPipeline final {
    public:
        explicit VulkanGraphicsPipeline(VkDevice const device) :
            device(device),
            graphicsPipeline(VK_NULL_HANDLE)
        {}
        explicit VulkanGraphicsPipeline(
            VkDevice const device,
            VkGraphicsPipelineCreateInfo const &pipelineInfo
        ) : device(device)
          , graphicsPipeline([&]{
                VkPipeline p{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create graphics pipeline!"));
                    }
                }
                return p;
            }())
        {
        }
        VulkanGraphicsPipeline(VulkanGraphicsPipeline const&) = delete;
        VulkanGraphicsPipeline(VulkanGraphicsPipeline &&o) noexcept
            : device(o.device)
            , graphicsPipeline(std::exchange(o.graphicsPipeline, VkPipeline{VK_NULL_HANDLE}))
        {}
        VulkanGraphicsPipeline &operator=(VulkanGraphicsPipeline const&) = delete;
        VulkanGraphicsPipeline &operator=(VulkanGraphicsPipeline &&o) noexcept {
            std::swap(device, o.device);
            std::swap(graphicsPipeline, o.graphicsPipeline);
            return *this;
        }
        ~VulkanGraphicsPipeline() noexcept {
            vkDestroyPipeline(device, graphicsPipeline, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkPipeline graphicsPipeline;
    };
}
#endif // !HG_VULKANGRAPHICSPIPELINE_H
