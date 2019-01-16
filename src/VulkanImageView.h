#ifndef HG_VULKAN_IMAGE_VUEW_H
#define HG_VULKAN_IMAGE_VUEW_H
#include <vulkan/vulkan.h>
#include <utility>
namespace hg {
    class VulkanImageView final {
    public:
        explicit VulkanImageView(
            VkDevice const device
        ) : device(device)
          , imageView(VK_NULL_HANDLE)
        {}
        explicit VulkanImageView(
            VkDevice const device,
            VkImageViewCreateInfo const &createInfo
        ) : device(device)
        {
            if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::exception("failed to create image view!");
            }
        }
        VulkanImageView(VulkanImageView const&) = delete;
        VulkanImageView(VulkanImageView &&o) noexcept
            : device(o.device)
            , imageView(std::exchange(o.imageView, VkImageView{VK_NULL_HANDLE}))
        {
        }
        VulkanImageView &operator=(VulkanImageView const&) = delete;
        VulkanImageView &operator=(VulkanImageView &&o) noexcept {
            std::swap(device, o.device);
            std::swap(imageView, o.imageView);
            return *this;
        }
        ~VulkanImageView() noexcept {
            vkDestroyImageView(device, imageView, nullptr);
        }
        VkDevice device;
        VkImageView imageView;
    };
}
#endif // !HG_VULKAN_IMAGE_VUEW_H
