#ifndef HG_VULKAN_IMAGE_H
#define HG_VULKAN_IMAGE_H
#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>
namespace hg {
    class VulkanImage final {
    public:
        explicit VulkanImage(VkDevice const device)
            : device(device), image(VK_NULL_HANDLE)
        {}
        explicit VulkanImage(
            VkDevice const device,
            VkImageCreateInfo const &imageInfo
        ) : device(device)
        {
            auto const res{vkCreateImage(device, &imageInfo, nullptr, &image)};
            if (res != VK_SUCCESS) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "vkCreateImage failed"));
            }
        }
        VulkanImage(VulkanImage const&) = delete;
        VulkanImage(VulkanImage &&o) noexcept
            : device(o.device)
            , image(std::exchange(o.image, VkImage{VK_NULL_HANDLE}))
        {
        }
        VulkanImage &operator=(VulkanImage const&) = delete;
        VulkanImage &operator=(VulkanImage &&o) noexcept {
            std::swap(device, o.device);
            std::swap(image, o.image);
            return *this;
        }
        ~VulkanImage() noexcept {
            vkDestroyImage(device, image, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkImage image;
    };
}
#endif // !HG_VULKAN_IMAGE_H
