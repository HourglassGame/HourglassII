#ifndef HG_VULKANSWAPCHAIN_H
#define HG_VULKANSWAPCHAIN_H

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>

namespace hg {

    class VulkanSwapChain final {
    public:
        explicit VulkanSwapChain(VkDevice const device)
            : device(device)
            , swapChain(VK_NULL_HANDLE)
        {}
        explicit VulkanSwapChain(
            VkDevice const device,
            VkSwapchainCreateInfoKHR const &createInfo
        )
          : device(device)
          , swapChain([&]{
                VkSwapchainKHR sc{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateSwapchainKHR(device, &createInfo, nullptr, &sc)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create swap chain!"));
                    }
                }
                return sc;
            }())
        {
        }
        VulkanSwapChain(VulkanSwapChain const&) = delete;
        VulkanSwapChain(VulkanSwapChain &&o) noexcept
            : device(o.device)
            , swapChain(std::exchange(o.swapChain, VkSwapchainKHR{VK_NULL_HANDLE}))
        {
        }
        VulkanSwapChain &operator=(VulkanSwapChain const&) = delete;
        VulkanSwapChain &operator=(VulkanSwapChain &&o) noexcept {
            std::swap(device, o.device);
            std::swap(swapChain, o.swapChain);
            return *this;
        }
        ~VulkanSwapChain() noexcept {
            vkDestroySwapchainKHR(device, swapChain, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkSwapchainKHR swapChain;
    };
}
#endif // !HG_VULKANSWAPCHAIN_H
