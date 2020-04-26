#ifndef HG_VULKANSWAPCHAINHG_H
#define HG_VULKANSWAPCHAINHG_H

#include "VulkanUtilPhysicalDevice.h"

#include "hg/VulkanUtil/VulkanSwapChain.h"
#include "hg/VulkanUtil/VulkanExceptions.h"

#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <array>

namespace hg {

    class VulkanSwapChainHG final {
    public:
        explicit VulkanSwapChainHG(VkDevice const device)
            : swapChain(device)
        {}
        explicit VulkanSwapChainHG(
            PossiblePhysicalDevice const &physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface,
            VkExtent2D const glfwFramebufferExtent,
            VkSwapchainKHR const oldSwapchain
        )
            : VulkanSwapChainHG(
                  physicalDevice,
                  device,
                  surface,
                  glfwFramebufferExtent,
                  oldSwapchain,
                  [&]{
                      VkSurfaceCapabilitiesKHR capabilities{};
                      {
                          auto const res{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.physicalDevice, surface, &capabilities)};
                          if (res != VK_SUCCESS) {
                              BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read surface capabilities"));
                          }
                      }
                      return capabilities;
                  }()
              )
        {
        }
        VulkanSwapChainHG(VulkanSwapChainHG const&) = delete;
        VulkanSwapChainHG(VulkanSwapChainHG &&o) noexcept = default;
        VulkanSwapChainHG &operator=(VulkanSwapChainHG const&) = delete;
        VulkanSwapChainHG &operator=(VulkanSwapChainHG &&o) noexcept = default;

        VkSwapchainKHR h() const {
            return swapChain.swapChain;
        }

        VkExtent2D extent() const {
            return extent_;
        }
    private:
        explicit VulkanSwapChainHG(
            PossiblePhysicalDevice const &physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface,
            VkExtent2D const glfwFramebufferExtent,
            VkSwapchainKHR const oldSwapchain,
            VkSurfaceCapabilitiesKHR const &capabilities
        )
            : extent_([&]() -> VkExtent2D {
                  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                      return capabilities.currentExtent;
                  }
                  else {
                      return {
                          std::clamp<uint32_t>(glfwFramebufferExtent.width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width),
                          std::clamp<uint32_t>(glfwFramebufferExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
                      };
                  }
              }())
            , swapChain(
                  device,

                  [&](auto const &queueFamilyIndices){
                      VkSwapchainCreateInfoKHR createInfo{};
                      createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                      createInfo.surface = surface;

                      createInfo.minImageCount = std::clamp<uint32_t>(
                              capabilities.minImageCount + 1,
                              capabilities.minImageCount,
                              capabilities.maxImageCount > 0 ? capabilities.maxImageCount : std::numeric_limits<uint32_t>::max());

                      createInfo.imageFormat = physicalDevice.surfaceFormat.format;
                      createInfo.imageColorSpace = physicalDevice.surfaceFormat.colorSpace;
                      createInfo.imageExtent = extent_;
                      createInfo.imageArrayLayers = 1;
                      createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

                      if (physicalDevice.queueIndices.graphicsFamily != physicalDevice.queueIndices.presentFamily) {
                          createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                          createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
                          createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
                      }
                      else {
                          createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                      }

                      createInfo.preTransform = capabilities.currentTransform;
                      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                      createInfo.presentMode = physicalDevice.presentMode;
                      createInfo.clipped = VK_TRUE;
                      createInfo.oldSwapchain = oldSwapchain;

                      return createInfo;
                  }(
                      [&]{
                          return std::array{physicalDevice.queueIndices.graphicsFamily, physicalDevice.queueIndices.presentFamily};
                      }()
                  )
              )
        {
        }
        VkExtent2D extent_;
        VulkanSwapChain swapChain;
    };
}
#endif // !HG_VULKANSWAPCHAINHG_H
