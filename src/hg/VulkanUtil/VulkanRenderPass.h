#ifndef HG_VULKANRENDERPASS_H
#define HG_VULKANRENDERPASS_H

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>

namespace hg {
    class VulkanRenderPass final {
    public:
        explicit VulkanRenderPass(VkDevice const device)
            : device(device)
            , renderPass(VK_NULL_HANDLE)
        {}
        explicit VulkanRenderPass(
            VkDevice const device,
            VkRenderPassCreateInfo const &renderPassInfo
        ) : device(device)
          , renderPass([&]{
                VkRenderPass rp{VK_NULL_HANDLE};
                {
                    auto const res{ vkCreateRenderPass(device, &renderPassInfo, nullptr, &rp) };
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create render pass!"));
                    }
                }
                return rp;
            }())
        {
        }
        VulkanRenderPass(VulkanRenderPass const&) = delete;
        VulkanRenderPass(VulkanRenderPass &&o) noexcept
            : device(o.device)
            , renderPass(std::exchange(o.renderPass, VkRenderPass{VK_NULL_HANDLE}))
        {}
        VulkanRenderPass &operator=(VulkanRenderPass const&) = delete;
        VulkanRenderPass &operator=(VulkanRenderPass &&o) noexcept {
            std::swap(device, o.device);
            std::swap(renderPass, o.renderPass);
            return *this;
        }
        ~VulkanRenderPass() noexcept {
            vkDestroyRenderPass(device, renderPass, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkRenderPass renderPass;
    };
}
#endif // !HG_VULKANRENDERPASS_H
