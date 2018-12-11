#ifndef HG_VULKANSWAPCHAINIMAGEVIEW_H
#define HG_VULKANSWAPCHAINIMAGEVIEW_H
#include <vulkan/vulkan.h>
#include <utility>
namespace hg {
    class VulkanSwapChainImageView final {
    public:
        VulkanSwapChainImageView(
            VkDevice const device,
            VkFormat const imageFormat,
            VkImage const image
        ) : device(device)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = image;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::exception("failed to create image view!");
            }
        }
        VulkanSwapChainImageView(VulkanSwapChainImageView const&) = delete;
        VulkanSwapChainImageView(VulkanSwapChainImageView &&o) noexcept
            : device(o.device)
            , imageView(std::exchange(o.imageView, VkImageView{VK_NULL_HANDLE}))
        {
        }
        VulkanSwapChainImageView &operator=(VulkanSwapChainImageView const&) = delete;
        VulkanSwapChainImageView &operator=(VulkanSwapChainImageView &&o) noexcept {
            std::swap(device, o.device);
            std::swap(imageView, o.imageView);
            return *this;
        }
        ~VulkanSwapChainImageView() noexcept {
            vkDestroyImageView(device, imageView, nullptr);
        }
        VkDevice device;
        VkImageView imageView;
    };
}
#endif // !HG_VULKANSWAPCHAINIMAGEVIEW_H
