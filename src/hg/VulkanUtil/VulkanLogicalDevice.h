#ifndef HG_VULKANLOGICALDEVICE_H
#define HG_VULKANLOGICALDEVICE_H

#include "hg/GlobalConst.h"
#include "VulkanUtil.h"

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>

namespace hg {

    class VulkanLogicalDevice final {
    public:
        explicit VulkanLogicalDevice(
            VkPhysicalDevice const physicalDevice,
            VkDeviceCreateInfo const &createInfo
        ) :
            device(
                [&]{
                    VkDevice d{VK_NULL_HANDLE};
                    {
                        auto const res{ vkCreateDevice(physicalDevice, &createInfo, nullptr, &d) };
                        if (res != VK_SUCCESS) {
                            BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create logical device!"));
                        }
                    }
                    return d;
                }()
            )
        {
        }
        VulkanLogicalDevice(VulkanLogicalDevice const&) = delete;
        VulkanLogicalDevice(VulkanLogicalDevice &&) = delete;
        VulkanLogicalDevice &operator=(VulkanLogicalDevice const&) = delete;
        VulkanLogicalDevice &operator=(VulkanLogicalDevice &&) = delete;
        ~VulkanLogicalDevice() noexcept {
            vkDestroyDevice(device, nullptr);
        }
        VkDevice device;
    };
}
#endif // !HG_VULKANLOGICALDEVICE_H
