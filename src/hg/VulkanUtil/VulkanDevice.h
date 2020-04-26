#ifndef HG_VULKANDEVICE_H
#define HG_VULKANDEVICE_H

#include "hg/GlobalConst.h"
#include "VulkanUtil.h"

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>

namespace hg {

    class VulkanDevice final {
    public:
        explicit VulkanDevice(
            VkPhysicalDevice const physicalDevice,
            VkDeviceCreateInfo const &createInfo
        ) :
            device(
                [&]{
                    VkDevice d{VK_NULL_HANDLE};
                    {
                        auto const res{vkCreateDevice(physicalDevice, &createInfo, nullptr, &d)};
                        if (res != VK_SUCCESS) {
                            BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create Vulkan device!"));
                        }
                    }
                    return d;
                }()
            )
        {
        }
        VulkanDevice(VulkanDevice const&) = delete;
        VulkanDevice(VulkanDevice &&) = delete;
        VulkanDevice &operator=(VulkanDevice const&) = delete;
        VulkanDevice &operator=(VulkanDevice &&) = delete;
        ~VulkanDevice() noexcept {
            vkDestroyDevice(device, nullptr);
        }
        VkDevice device;
    };
}
#endif // !HG_VULKANDEVICE_H
