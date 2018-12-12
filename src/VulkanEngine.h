#ifndef HG_VULKANENGINE_H
#define HG_VULKANENGINE_H
#include "VulkanSwapChain.h"
#include "VulkanSwapChainImageView.h"
#include "VulkanSurface.h"
#include "VulkanInstance.h"
#include "VulkanDebugCallback.h"
#include "VulkanUtil.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPipelineLayout.h"
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

    inline auto const strcmporder{ [](char const * const a, char const * const b) {return strcmp(a, b) < 0; } };

    inline bool checkValidationLayerSupport() {
        uint32_t layerCount = 0;
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read vulkan layer count", res);
            }
        }
        std::vector<VkLayerProperties> availableLayers(layerCount);
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read vulkan layers", res);
            }
        }
        availableLayers.resize(layerCount);

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
    inline bool checkDeviceExtensionSupport(VkPhysicalDevice const device) {
        uint32_t extensionCount{0};
        {
            auto const res{vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr)};
            if (!(res== VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't count Device Extensions vkEnumerateDeviceExtensionProperties");
            }
        }
        
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        {
            auto const res{vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data())};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't read Device Extensions vkEnumerateDeviceExtensionProperties");
            }
            availableExtensions.resize(extensionCount);
        }

        std::vector<const char*> deviceExtensionsSortUnique(deviceExtensions);
        boost::erase(deviceExtensionsSortUnique, boost::unique(boost::sort(deviceExtensionsSortUnique, strcmporder), strcmporder));

        std::vector<const char*> availableExtensionsSortUnique;
        boost::push_back(availableExtensionsSortUnique, availableExtensions | boost::adaptors::transformed([](VkExtensionProperties const &a) {return a.extensionName; }));
        boost::erase(availableExtensionsSortUnique, boost::unique(boost::sort(availableExtensionsSortUnique, strcmporder), strcmporder));

        return boost::range::includes(
            availableExtensionsSortUnique,
            deviceExtensionsSortUnique,
            strcmporder);
    }

    inline bool checkSwapChainSupport(SwapChainSupportDetails const &swapChainSupport) {
        return !(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty());
    }

    inline bool isDeviceSuitable(VkPhysicalDevice const device, VkSurfaceKHR const surface) {
        return findQueueFamilies(device, surface).isComplete()
            && checkDeviceExtensionSupport(device)
            && checkSwapChainSupport(querySwapChainSupport(device, surface));
    }

    inline VkPhysicalDevice pickPhysicalDevice(VkInstance const &instance, VkSurfaceKHR const surface) {
        uint32_t deviceCount = 0;
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read physical device count", res);
            }
        }
        if (deviceCount == 0) {
            throw std::exception("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data())};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("failed to read physical devices", res);
            }
        }
        devices.resize(deviceCount);
        auto const suitableDeviceIt{boost::find_if(devices, [&surface](VkPhysicalDevice device) {return isDeviceSuitable(device, surface);})};
        if (suitableDeviceIt == std::end(devices)) {
            throw std::exception("failed to find a suitable GPU!");
        }
        return *suitableDeviceIt;
    }
    inline std::vector<VkImage> createSwapChainImages(
        VkDevice const device,
        VkSwapchainKHR const swapChain,
        uint32_t &imageCount)
    {
        std::vector<VkImage> swapChainImages;
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't read swapchain image count");
            }
        }
        swapChainImages.resize(imageCount);
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't read swapchain images");
            }
        }
        swapChainImages.resize(imageCount);
        return swapChainImages;
    }
    inline std::vector<VulkanSwapChainImageView> createSwapChainImageViews(
        VkDevice const device, VkFormat const imageFormat, std::vector<VkImage> const &swapChainImages) {
        std::vector<VulkanSwapChainImageView> swapChainImageViews;
        auto rng{ swapChainImages | boost::adaptors::transformed([device, imageFormat](auto const &i) {return VulkanSwapChainImageView{device, imageFormat, i}; }) };
        swapChainImageViews.assign(std::make_move_iterator(std::begin(rng)), std::make_move_iterator(std::end(rng)));

        return swapChainImageViews;
    }

    class VulkanEngine final {
    public:
        VulkanEngine(
            GLFWwindow &w
        ) : w(&w)
          , instance(makeInstanceCreateInfo(nullptr, getRequiredExtensions()))
          , debugCallback(instance.i)
          , surface(instance.i, &w)
          , physicalDevice(pickPhysicalDevice(instance.i, surface.surface))
          , logicalDevice(physicalDevice, surface.surface)
          , swapChain(physicalDevice, logicalDevice.device, surface.surface)
          , swapChainImages(createSwapChainImages(logicalDevice.device, swapChain.swapChain, swapChain.imageCount))
          , swapChainImageViews(createSwapChainImageViews(logicalDevice.device, swapChain.surfaceFormat.format, swapChainImages))
          , pipelineLayout(logicalDevice.device, swapChain.extent)
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
        VulkanSurface surface;
        VkPhysicalDevice physicalDevice;
        VulkanLogicalDevice logicalDevice;
        VulkanSwapChain swapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VulkanSwapChainImageView> swapChainImageViews;
        VulkanPipelineLayout pipelineLayout;
    };
}
#endif // !HG_VULKANENGINE_H
