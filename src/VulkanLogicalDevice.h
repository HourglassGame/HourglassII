#ifndef HG_VULKANLOGICALDEVICE_H
#define HG_VULKANLOGICALDEVICE_H
#include "GlobalConst.h"
#include "VulkanUtil.h"
#include <vulkan/vulkan.h>
namespace hg {

    class VulkanLogicalDevice final {
    public:
        VulkanLogicalDevice(
            VkPhysicalDevice physicalDevice
        )
        {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            if (!indices.graphicsFamily.has_value()) {
                throw std::exception("No graphics queue on physical device, should not have been detected as a valid device selection");
            }

            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;

            float const queuePriority[1]{1.0f};
            queueCreateInfo.pQueuePriorities = queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures = {};

            VkDeviceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pNext = nullptr;

            createInfo.pQueueCreateInfos = &queueCreateInfo;
            createInfo.queueCreateInfoCount = 1;

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = 0;
            createInfo.ppEnabledExtensionNames = nullptr;

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::exception("failed to create logical device!");
            }

            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        }
        VulkanLogicalDevice(VulkanLogicalDevice const&) = delete;
        VulkanLogicalDevice(VulkanLogicalDevice &&) = delete;
        VulkanLogicalDevice &operator=(VulkanLogicalDevice const&) = delete;
        VulkanLogicalDevice &operator=(VulkanLogicalDevice &&) = delete;
        ~VulkanLogicalDevice() noexcept {
            vkDestroyDevice(device, nullptr);
        }
        VkDevice device;
        VkQueue graphicsQueue;
    };
}
#endif // !HG_VULKANLOGICALDEVICE_H
