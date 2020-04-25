#ifndef HG_VULKANLOGICALDEVICEHG_H
#define HG_VULKANLOGICALDEVICEHG_H

#include "hg/VulkanUtil/VulkanLogicalDevice.h"
#include "hg/VulkanUtil/VulkanExceptions.h"
#include "hg/VulkanUtil/VulkanUtil.h"

#include "hg/GlobalConst.h"

#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>

namespace hg {

    class VulkanLogicalDeviceHG final {
    public:
        explicit VulkanLogicalDeviceHG(
            VkPhysicalDevice const physicalDevice,
            VkSurfaceKHR const surface,
            QueueFamilyIndices const &indices //= findQueueFamilies(physicalDevice, surface)/*TODO: Move to be computed only once: when the physical device is selected*/
        ) : logicalDevice(
                physicalDevice,
                [&](auto const &queueCreateInfos){
                    //QueueFamilyIndices const indices{findQueueFamilies(physicalDevice, surface)};

                    if (!indices.graphicsFamily) {
                        BOOST_THROW_EXCEPTION(std::exception("No graphics queue on physical device, should not have been detected as a valid device selection"));
                    }
                    if (!indices.presentFamily) {
                        BOOST_THROW_EXCEPTION(std::exception("No present queue on physical device, should not have been detected as a valid device selection"));
                    }

                    VkDeviceCreateInfo createInfo = {};
                    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                    createInfo.pNext = nullptr;

                    createInfo.pQueueCreateInfos = queueCreateInfos.data();
                    createInfo.queueCreateInfoCount = gsl::narrow<uint32_t>(queueCreateInfos.size());

                    createInfo.pEnabledFeatures = nullptr;

                    if (enableValidationLayers) {
                        createInfo.enabledLayerCount = gsl::narrow<uint32_t>(validationLayers.size());
                        createInfo.ppEnabledLayerNames = validationLayers.data();
                    }
                    else {
                        createInfo.enabledLayerCount = 0;
                        createInfo.ppEnabledLayerNames = nullptr;
                    }

                    createInfo.enabledExtensionCount = gsl::narrow<uint32_t>(deviceExtensions.size());
                    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

                    return createInfo;
                }(
                    [&](auto const &queuePriorities){
                        VkDeviceQueueCreateInfo queueCreateInfo = {};
                        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                        queueCreateInfo.pNext = nullptr;
                        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
                        queueCreateInfo.queueCount = gsl::narrow<uint32_t>(queuePriorities.size());
                        queueCreateInfo.pQueuePriorities = queuePriorities.data();

                        return std::array{queueCreateInfo};
                    }([]{return std::array{1.0f};}())
                )
            )
        {
            vkGetDeviceQueue(logicalDevice.device, indices.graphicsFamily.value(), 0, &graphicsQueue);
            vkGetDeviceQueue(logicalDevice.device, indices. presentFamily.value(), 0, &presentQueue);
        }
        VulkanLogicalDeviceHG(VulkanLogicalDeviceHG const&) = delete;
        VulkanLogicalDeviceHG(VulkanLogicalDeviceHG &&) = delete;
        VulkanLogicalDeviceHG &operator=(VulkanLogicalDeviceHG const&) = delete;
        VulkanLogicalDeviceHG &operator=(VulkanLogicalDeviceHG &&) = delete;

        VkDevice h() const {
            return logicalDevice.device;
        }
        VkQueue graphicsQ() const {
            return graphicsQueue;
        }
        VkQueue presentQ() const {
            return presentQueue;
        }
    private:
        VulkanLogicalDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    };
}
#endif // !HG_VULKANLOGICALDEVICEHG_H
