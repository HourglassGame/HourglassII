#ifndef HG_VULKANLOGICALDEVICE_H
#define HG_VULKANLOGICALDEVICE_H
#include "GlobalConst.h"
#include "VulkanUtil.h"
#include <vulkan/vulkan.h>
namespace hg {

    class VulkanLogicalDevice final {
    public:
        VulkanLogicalDevice(
            VkPhysicalDevice const physicalDevice,
            VkSurfaceKHR const surface
        )
        {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

            if (!indices.graphicsFamily) {
                throw std::exception("No graphics queue on physical device, should not have been detected as a valid device selection");
            }
            if (!indices.presentFamily) {
                throw std::exception("No present queue on physical device, should not have been detected as a valid device selection");
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

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else {
                createInfo.enabledLayerCount = 0;
                createInfo.ppEnabledLayerNames = nullptr;
            }

            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::exception("failed to create logical device!");
            }

            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
            vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
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
        VkQueue presentQueue;
    };
}
#endif // !HG_VULKANLOGICALDEVICE_H
