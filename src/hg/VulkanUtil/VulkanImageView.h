#ifndef HG_VULKAN_IMAGE_VIEW_H
#define HG_VULKAN_IMAGE_VIEW_H
#include "VulkanExceptions.h"
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
            {
                auto const res{vkCreateImageView(device, &createInfo, nullptr, &imageView)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create image view!"));
                }
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
#endif // !HG_VULKAN_IMAGE_VIEW_H
