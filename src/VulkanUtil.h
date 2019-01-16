#ifndef HG_VULKANUTIL_H
#define HG_VULKANUTIL_H
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "Maths.h"
namespace hg {
    struct Vertex {
        vec2<float> pos;
        vec3<float> colour;
        vec2<float> texCoord;
        unsigned char useTexture;
    };

    //typedef vec2<float> Vertex;
    std::vector<const char*> const deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    inline std::vector<uint32_t> reinterpretToUint32Vector(std::vector<char> const &data) {
        static_assert(CHAR_BIT == 8);
        if (data.size() % sizeof(uint32_t) != 0) {
            throw std::runtime_error("File size not divisible by uint32_t size");
        }

        std::vector<uint32_t> out(data.size() / sizeof(uint32_t));
        memcpy(out.data(), data.data(), data.size());
        return out;
    }
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily && presentFamily;
        }
    };

    inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR const surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        queueFamilies.resize(queueFamilyCount);
        uint32_t i{0};
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }
            }
            if (indices.isComplete()) break;
            ++i;
        }

        return indices;
    }

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice const device, VkSurfaceKHR const surface) {
        SwapChainSupportDetails details;

        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities) != VK_SUCCESS) {
            throw std::exception("Couldn't Read Surface Capabilities");
        }

        uint32_t formatCount{};
        {
            auto const res{vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr)};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read surface formats count");
            }
        }

        details.formats.resize(formatCount);
        {
            auto const res{ vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data())};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read surface formats");
            }
        }
        details.formats.resize(formatCount);

        uint32_t presentModeCount{};
        {
            auto const res{vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr)};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read presentModes count");
            }
        }

        details.presentModes.resize(presentModeCount);
        {
            auto const res{ vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data())};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read presentModes");
            }
        }
        details.presentModes.resize(presentModeCount);

        return details;
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

        throw std::exception("failed to find suitable memory type!");
    }
}
#endif // !HG_VULKANUTIL_H
