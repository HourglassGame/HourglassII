#ifndef HG_VULKANDEVICEHG_H
#define HG_VULKANDEVICEHG_H

#include "VulkanUtilPhysicalDevice.h"

#include "hg/VulkanUtil/VulkanDevice.h"
#include "hg/VulkanUtil/VulkanExceptions.h"
#include "hg/VulkanUtil/VulkanUtil.h"

#include "hg/GlobalConst.h"

#include <gsl/gsl_util>
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <array>

namespace hg {

    class VulkanDeviceHG final {
    public:
        explicit VulkanDeviceHG(
            PossiblePhysicalDevice const &physicalDevice,
            VkSurfaceKHR const surface
        )
          : device(
                physicalDevice.physicalDevice,
                [&](auto const &queueCreateInfos){

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
                        queueCreateInfo.queueFamilyIndex = physicalDevice.queueIndices.graphicsFamily;
                        queueCreateInfo.queueCount = gsl::narrow<uint32_t>(queuePriorities.size());
                        queueCreateInfo.pQueuePriorities = queuePriorities.data();

                        return std::array{queueCreateInfo};
                    }([]{return std::array{1.0f};}())
                )
            )
          , graphicsQueue([&]{
                VkQueue q{VK_NULL_HANDLE};
                vkGetDeviceQueue(device.device, physicalDevice.queueIndices.graphicsFamily, 0, &q);
                return q;
            }())
          , presentQueue([&]{
                VkQueue q{VK_NULL_HANDLE};
                vkGetDeviceQueue(device.device, physicalDevice.queueIndices. presentFamily, 0, &q);
                return q;
            }())
        {
        }
        VulkanDeviceHG(VulkanDeviceHG const&) = delete;
        VulkanDeviceHG(VulkanDeviceHG &&) = delete;
        VulkanDeviceHG &operator=(VulkanDeviceHG const&) = delete;
        VulkanDeviceHG &operator=(VulkanDeviceHG &&) = delete;

        VkDevice h() const {
            return device.device;
        }
        VkQueue graphicsQ() const {
            return graphicsQueue;
        }
        VkQueue presentQ() const {
            return presentQueue;
        }
    private:
        VulkanDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    };
}
#endif // !HG_VULKANDEVICEHG_H
