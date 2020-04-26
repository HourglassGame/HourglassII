#include "VulkanUtilPhysicalDevice.h"

#include "hg/Util/util.h"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/throw_exception.hpp>
#include <system_error>

namespace hg {
    namespace {

        struct SwapChainSupportDetails final {
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        std::optional<QueueFamiliesForUses> findQueueFamilies(VkPhysicalDevice const physicalDevice, VkSurfaceKHR const surface) {
            std::vector<VkQueueFamilyProperties> const queueFamilies{[&]{
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
                queueFamilies.resize(queueFamilyCount);
                return queueFamilies;
            }()};

            std::optional<uint32_t> graphicsFamily{};
            std::optional<uint32_t> presentFamily{};

            uint32_t i{0};
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueCount > 0) {
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        graphicsFamily = i;
                    }

                    VkBool32 presentSupport{VK_FALSE};
                    {
                        auto const res{vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport)};
                        if (res != VK_SUCCESS) {
                            BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read present support for queue"));
                        }
                    }

                    if (presentSupport) {
                        presentFamily = i;
                    }
                }
                if (graphicsFamily && presentFamily) {
                    QueueFamiliesForUses indices{};
                    indices.graphicsFamily = *graphicsFamily;
                    indices.presentFamily = *presentFamily;
                    return {indices};
                }
                ++i;
            }

            return {};
        }

        bool checkDeviceExtensionSupport(VkPhysicalDevice const device) {
            uint32_t extensionCount{0};
            {
                auto const res{vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr)};
                if (!(res== VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't count Device Extensions vkEnumerateDeviceExtensionProperties"));
                }
            }
        
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            {
                auto const res{vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data())};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read Device Extensions vkEnumerateDeviceExtensionProperties"));
                }
                availableExtensions.resize(extensionCount);
            }
            std::vector<const char*> deviceExtensionsSortUnique(deviceExtensions.begin(), deviceExtensions.end());
            boost::erase(deviceExtensionsSortUnique, boost::unique(boost::sort(deviceExtensionsSortUnique, strcmporder), strcmporder));

            std::vector<const char*> availableExtensionsSortUnique;
            boost::push_back(availableExtensionsSortUnique, availableExtensions | boost::adaptors::transformed([](VkExtensionProperties const &a) {return a.extensionName; }));
            boost::erase(availableExtensionsSortUnique, boost::unique(boost::sort(availableExtensionsSortUnique, strcmporder), strcmporder));

            return boost::range::includes(
                availableExtensionsSortUnique,
                deviceExtensionsSortUnique,
                strcmporder);
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice const device, VkSurfaceKHR const surface) {
            SwapChainSupportDetails details{};

            uint32_t formatCount{};
            {
                auto const res{vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr)};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read surface formats count"));
                }
            }

            details.formats.resize(formatCount);
            {
                auto const res{ vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data())};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read surface formats"));
                }
            }
            details.formats.resize(formatCount);

            uint32_t presentModeCount{};
            {
                auto const res{vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr)};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read presentModes count"));
                }
            }

            details.presentModes.resize(presentModeCount);
            {
                auto const res{ vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data())};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read presentModes"));
                }
            }
            details.presentModes.resize(presentModeCount);

            return details;
        }


        bool checkSwapChainSupport(SwapChainSupportDetails const &swapChainSupport) {
            return !(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty());
        }


        VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &availableFormats) {
            if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
                return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
            }

            for (auto const &availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }

                /*
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }*/
            }

            return availableFormats[0];
        }


        VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const &availablePresentModes) {
            VkPresentModeKHR bestMode{VK_PRESENT_MODE_FIFO_KHR};

            for (auto const &availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
                else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                    bestMode = availablePresentMode;
                }
            }

            return bestMode;
        }
    }

    std::vector<PossiblePhysicalDevice> enumerateSuitablePhysicalDevices(VkInstance const instance, VkSurfaceKHR const surface) {
        std::vector<VkPhysicalDevice> const physicalDevices{[&]{
            uint32_t physicalDeviceCount{0};
            {
                auto const res{vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr)};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read physical device count"));
                }
            }

            std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
            {
                auto const res{vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data())};
                if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read physical devices"));
                }
            }
            physicalDevices.resize(physicalDeviceCount);

            return physicalDevices;
        }()};

        std::vector<PossiblePhysicalDevice> possibleDevices{};
        possibleDevices.reserve(physicalDevices.size());

        for (const auto physicalDevice : physicalDevices) {
            auto const queueIndices{findQueueFamilies(physicalDevice, surface)};
            auto const swapChainSupport{querySwapChainSupport(physicalDevice, surface)};
            if (!queueIndices
             || !checkDeviceExtensionSupport(physicalDevice)
             || !checkSwapChainSupport(swapChainSupport)) { continue; }

            VkPhysicalDeviceProperties deviceProperties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

            PossiblePhysicalDevice deviceEvaluation{};
            deviceEvaluation.physicalDevice = physicalDevice;
            deviceEvaluation.queueIndices = *queueIndices;
            deviceEvaluation.surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            deviceEvaluation.presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

            deviceEvaluation.score = 1 + (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0);

            possibleDevices.push_back(deviceEvaluation);
        }
        boost::sort(possibleDevices, [](PossiblePhysicalDevice const &l, PossiblePhysicalDevice const &r){return l.score > r.score;/*Descending*/});
        return possibleDevices;
    }
}
