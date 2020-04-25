#ifndef HG_VULKANUTIL_H
#define HG_VULKANUTIL_H
#include "VulkanExceptions.h"
#include <vulkan/vulkan.h>
#include "VulkanExceptions.h"
#include <vector>
#include <optional>
#include "hg/Util/Maths.h"
#include <boost/range/algorithm/copy.hpp>
namespace hg {
    struct Vertex {
        vec2<float> pos;
        vec3<float> colour;
        vec2<float> texCoord;
        unsigned char useTexture;
    };

    inline VkExtent2D querySwapChainMaxImageExtent(VkPhysicalDevice const device, VkSurfaceKHR const surface) {
        VkSurfaceCapabilitiesKHR capabilities{};
        {
            auto const res{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities)};
            if (res != VK_SUCCESS) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't Read Surface Capabilities"));
            }
        }
        return capabilities.maxImageExtent;
    }

    inline uint32_t findMemoryType(VkPhysicalDevice const physicalDevice, VkDeviceSize const size, uint32_t const typeFilter, VkMemoryPropertyFlags const properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
            if (
                (typeFilter & (1 << i))
             && (memProperties.memoryTypes[i].propertyFlags & properties) == properties
             //TODO:
             //https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkAllocateMemory.html
             //specifies
             //"""
             //pAllocateInfo->allocationSize must be less than or equal to
             //VkPhysicalDeviceMemoryProperties::memoryHeaps[pAllocateInfo->memoryTypeIndex].size
             //as returned by vkGetPhysicalDeviceMemoryProperties for the VkPhysicalDevice that device was created from.
             //"""
             //I'm assuming this is an error in the spec, and should be
             //VkPhysicalDeviceMemoryProperties::memoryHeaps[memProperties.memoryTypes[i].heapIndex].size
             //But this needs to be checked/reported.
             && (size <= memProperties.memoryHeaps[memProperties.memoryTypes[i].heapIndex].size))
            {
                return i;
            }
        }

        BOOST_THROW_EXCEPTION(std::exception("failed to find suitable memory type!"));
    }
}
#endif // !HG_VULKANUTIL_H
