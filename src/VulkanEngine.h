#ifndef HG_VULKANENGINE_H
#define HG_VULKANENGINE_H
#include "VulkanInstance.h"
#include "VulkanDebugCallback.h"
#include <GLFW/glfw3.h>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <vector>
#include <optional>
#include "GlobalConst.h"
namespace hg {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;

        bool isComplete() {
            return graphicsFamily.has_value();
        }
    };

    inline bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        auto strcmporder{ [](char const * const a, char const * const b) {return strcmp(a, b) < 0; } };

        std::vector<const char*> validationLayersSortUnique(validationLayers);
        boost::erase(validationLayersSortUnique, boost::unique(boost::sort(validationLayersSortUnique, strcmporder), strcmporder));

        std::vector<const char*> availableLayersSortUnique;
        boost::push_back(availableLayersSortUnique, availableLayers | boost::adaptors::transformed([](VkLayerProperties const &a) {return a.layerName; }));

        boost::erase(availableLayersSortUnique, boost::unique(boost::sort(availableLayersSortUnique, strcmporder), strcmporder));

        return boost::range::includes(
            availableLayersSortUnique,
            validationLayersSortUnique,
            strcmporder);
    }

    std::vector<const char*> getRequiredExtensions() {
        if (!glfwVulkanSupported()) {
            throw std::exception("Vulkan support insufficient to run in GLFW");
        }
        uint32_t glfwExtensionCount = 0;
        const char ** const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (!glfwExtensions) {
            throw std::exception("Couldn't load GLFW vulkan extension list (glfwGetRequiredInstanceExtensions)");
        }
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    inline VkInstanceCreateInfo makeInstanceCreateInfo(VkApplicationInfo const * const pApplicationInfo, std::vector<char const *> const &extensions) {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::exception("validation layer(s) requested by not supported");
        }

        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.pApplicationInfo = pApplicationInfo;

        if (enableValidationLayers) {
            info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            info.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            info.enabledLayerCount = 0;
            info.ppEnabledLayerNames = nullptr;
        }

        if (!glfwVulkanSupported()) {
            throw std::exception("Vulkan support insufficient to run in GLFW");
        }

        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
        return info;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    inline bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices{findQueueFamilies(device)};
        return indices.isComplete();
    }

    inline VkPhysicalDevice pickPhysicalDevice(VkInstance const &instance) {
        uint32_t deviceCount = 0;
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read physical device count");
            }
        }
        if (deviceCount == 0) {
            throw std::exception("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data())};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("failed to read physical devices");
            }
        }
        devices.resize(deviceCount);
        auto const suitableDeviceIt{boost::find_if<boost::return_found>(devices, isDeviceSuitable)};
        if (suitableDeviceIt == std::end(devices)) {
            throw std::exception("failed to find a suitable GPU!");
        }
        return *suitableDeviceIt;
    }

    class VulkanEngine final {
    public:
        VulkanEngine(
            GLFWwindow &w
        ) : w(&w)
          , instance(makeInstanceCreateInfo(nullptr, getRequiredExtensions()))
          , debugCallback(instance.i)
          , physicalDevice(pickPhysicalDevice(instance.i))
        {
            //createInstance();
            //setupDebugCallback();
            //createSurface();
            //pickPhysicalDevice();
            //createLogicalDevice();
            //createSwapChain();
            //createImageViews();
            //createRenderPass();
            //createGraphicsPipeline();
            //createFramebuffers();
            //createCommandPool();
            //createCommandBuffers();
            //createSyncObjects();
        }
        VulkanEngine(VulkanEngine const&) = delete;
        VulkanEngine(VulkanEngine &&) = delete;
        VulkanEngine &operator=(VulkanEngine const&) = delete;
        VulkanEngine &operator=(VulkanEngine &&) = delete;
        ~VulkanEngine() noexcept {
            
        }
    private:
        GLFWwindow *w;
        VulkanInstance instance;
        VulkanDebugCallback debugCallback;
        VkPhysicalDevice physicalDevice;
    };
}
#endif // !HG_VULKANENGINE_H
