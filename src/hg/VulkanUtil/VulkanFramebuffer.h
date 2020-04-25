#ifndef HG_VULKANFRAMEBUFFER_H
#define HG_VULKANFRAMEBUFFER_H

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>

namespace hg {
    class VulkanFramebuffer final {
    public:
        explicit VulkanFramebuffer(
            VkDevice const device,
            VkFramebufferCreateInfo const &createInfo
        ) : device(device)
          , framebuffer([&]{
                VkFramebuffer fb{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateFramebuffer(device, &createInfo, nullptr, &fb)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create framebuffer!"));
                    }
                }
                return fb;
            }())
        {
        }
        VulkanFramebuffer(VulkanFramebuffer const&) = delete;
        VulkanFramebuffer(VulkanFramebuffer &&o) noexcept
            : device(o.device)
            , framebuffer(std::exchange(o.framebuffer, VkFramebuffer{VK_NULL_HANDLE}))
        {
        }
        VulkanFramebuffer &operator=(VulkanFramebuffer const&) = delete;
        VulkanFramebuffer &operator=(VulkanFramebuffer &&o) noexcept {
            std::swap(device, o.device);
            std::swap(framebuffer, o.framebuffer);
            return *this;
        }
        ~VulkanFramebuffer() noexcept {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkFramebuffer framebuffer;
    };
}
#endif // !HG_VULKANFRAMEBUFFER_H
