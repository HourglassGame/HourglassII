#ifndef HG_VULKANENGINE_H
#define HG_VULKANENGINE_H
#include "VulkanSwapChain.h"
#include "VulkanImageView.h"
#include "VulkanSurface.h"
#include "VulkanRenderPass.h"
#include "VulkanInstance.h"
#include "VulkanDebugCallback.h"
#include "VulkanUtil.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSetLayout.h"
#include "RunningGameSceneRenderer.h"
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
#include "VulkanExceptions.h"
namespace hg {
    inline auto const strcmporder{ [](char const * const a, char const * const b) {return strcmp(a, b) < 0; } };
    inline auto const strcmpeq{ [](char const * const a, char const * const b) {return strcmp(a, b) == 0; } };

    inline bool checkValidationLayerSupport() {
        uint32_t layerCount = 0;
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read vulkan layer count"));
            }
        }
        std::vector<VkLayerProperties> availableLayers(layerCount);
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read vulkan layers"));
            }
        }
        availableLayers.resize(layerCount);

        std::vector<const char*> validationLayersSortUnique(validationLayers);
        boost::erase(validationLayersSortUnique, boost::unique<boost::return_found_end>(boost::sort(validationLayersSortUnique, strcmporder), strcmpeq));

        std::vector<const char*> availableLayersSortUnique;
        boost::push_back(availableLayersSortUnique, availableLayers | boost::adaptors::transformed([](VkLayerProperties const &a) {return a.layerName; }));
        boost::erase(availableLayersSortUnique, boost::unique<boost::return_found_end>(boost::sort(availableLayersSortUnique, strcmporder), strcmpeq));

        return boost::range::includes(
            availableLayersSortUnique,
            validationLayersSortUnique,
            strcmporder);
    }

    std::vector<const char*> getRequiredExtensions() {
        if (!glfwVulkanSupported()) {
            BOOST_THROW_EXCEPTION(std::exception("Vulkan support insufficient to run in GLFW"));
        }
        uint32_t glfwExtensionCount = 0;
        const char ** const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (!glfwExtensions) {
            BOOST_THROW_EXCEPTION(std::exception("Couldn't load GLFW vulkan extension list (glfwGetRequiredInstanceExtensions)"));
        }
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    inline VkInstanceCreateInfo makeInstanceCreateInfo(VkApplicationInfo const * const pApplicationInfo, std::vector<char const *> const &extensions) {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            BOOST_THROW_EXCEPTION(std::exception("validation layer(s) requested by not supported"));
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
            BOOST_THROW_EXCEPTION(std::exception("Vulkan support insufficient to run in GLFW"));
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
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        VkFormat allFormats[] =
        {
            VK_FORMAT_UNDEFINED,
            VK_FORMAT_R4G4_UNORM_PACK8,
            VK_FORMAT_R4G4B4A4_UNORM_PACK16,
            VK_FORMAT_B4G4R4A4_UNORM_PACK16,
            VK_FORMAT_R5G6B5_UNORM_PACK16,
            VK_FORMAT_B5G6R5_UNORM_PACK16,
            VK_FORMAT_R5G5B5A1_UNORM_PACK16,
            VK_FORMAT_B5G5R5A1_UNORM_PACK16,
            VK_FORMAT_A1R5G5B5_UNORM_PACK16,
            VK_FORMAT_R8_UNORM,
            VK_FORMAT_R8_SNORM,
            VK_FORMAT_R8_USCALED,
            VK_FORMAT_R8_SSCALED,
            VK_FORMAT_R8_UINT,
            VK_FORMAT_R8_SINT,
            VK_FORMAT_R8_SRGB,
            VK_FORMAT_R8G8_UNORM,
            VK_FORMAT_R8G8_SNORM,
            VK_FORMAT_R8G8_USCALED,
            VK_FORMAT_R8G8_SSCALED,
            VK_FORMAT_R8G8_UINT,
            VK_FORMAT_R8G8_SINT,
            VK_FORMAT_R8G8_SRGB,
            VK_FORMAT_R8G8B8_UNORM,
            VK_FORMAT_R8G8B8_SNORM,
            VK_FORMAT_R8G8B8_USCALED,
            VK_FORMAT_R8G8B8_SSCALED,
            VK_FORMAT_R8G8B8_UINT,
            VK_FORMAT_R8G8B8_SINT,
            VK_FORMAT_R8G8B8_SRGB,
            VK_FORMAT_B8G8R8_UNORM,
            VK_FORMAT_B8G8R8_SNORM,
            VK_FORMAT_B8G8R8_USCALED,
            VK_FORMAT_B8G8R8_SSCALED,
            VK_FORMAT_B8G8R8_UINT,
            VK_FORMAT_B8G8R8_SINT,
            VK_FORMAT_B8G8R8_SRGB,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_R8G8B8A8_SNORM,
            VK_FORMAT_R8G8B8A8_USCALED,
            VK_FORMAT_R8G8B8A8_SSCALED,
            VK_FORMAT_R8G8B8A8_UINT,
            VK_FORMAT_R8G8B8A8_SINT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_B8G8R8A8_SNORM,
            VK_FORMAT_B8G8R8A8_USCALED,
            VK_FORMAT_B8G8R8A8_SSCALED,
            VK_FORMAT_B8G8R8A8_UINT,
            VK_FORMAT_B8G8R8A8_SINT,
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_FORMAT_A8B8G8R8_UNORM_PACK32,
            VK_FORMAT_A8B8G8R8_SNORM_PACK32,
            VK_FORMAT_A8B8G8R8_USCALED_PACK32,
            VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
            VK_FORMAT_A8B8G8R8_UINT_PACK32,
            VK_FORMAT_A8B8G8R8_SINT_PACK32,
            VK_FORMAT_A8B8G8R8_SRGB_PACK32,
            VK_FORMAT_A2R10G10B10_UNORM_PACK32,
            VK_FORMAT_A2R10G10B10_SNORM_PACK32,
            VK_FORMAT_A2R10G10B10_USCALED_PACK32,
            VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
            VK_FORMAT_A2R10G10B10_UINT_PACK32,
            VK_FORMAT_A2R10G10B10_SINT_PACK32,
            VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            VK_FORMAT_A2B10G10R10_SNORM_PACK32,
            VK_FORMAT_A2B10G10R10_USCALED_PACK32,
            VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
            VK_FORMAT_A2B10G10R10_UINT_PACK32,
            VK_FORMAT_A2B10G10R10_SINT_PACK32,
            VK_FORMAT_R16_UNORM,
            VK_FORMAT_R16_SNORM,
            VK_FORMAT_R16_USCALED,
            VK_FORMAT_R16_SSCALED,
            VK_FORMAT_R16_UINT,
            VK_FORMAT_R16_SINT,
            VK_FORMAT_R16_SFLOAT,
            VK_FORMAT_R16G16_UNORM,
            VK_FORMAT_R16G16_SNORM,
            VK_FORMAT_R16G16_USCALED,
            VK_FORMAT_R16G16_SSCALED,
            VK_FORMAT_R16G16_UINT,
            VK_FORMAT_R16G16_SINT,
            VK_FORMAT_R16G16_SFLOAT,
            VK_FORMAT_R16G16B16_UNORM,
            VK_FORMAT_R16G16B16_SNORM,
            VK_FORMAT_R16G16B16_USCALED,
            VK_FORMAT_R16G16B16_SSCALED,
            VK_FORMAT_R16G16B16_UINT,
            VK_FORMAT_R16G16B16_SINT,
            VK_FORMAT_R16G16B16_SFLOAT,
            VK_FORMAT_R16G16B16A16_UNORM,
            VK_FORMAT_R16G16B16A16_SNORM,
            VK_FORMAT_R16G16B16A16_USCALED,
            VK_FORMAT_R16G16B16A16_SSCALED,
            VK_FORMAT_R16G16B16A16_UINT,
            VK_FORMAT_R16G16B16A16_SINT,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R32_UINT,
            VK_FORMAT_R32_SINT,
            VK_FORMAT_R32_SFLOAT,
            VK_FORMAT_R32G32_UINT,
            VK_FORMAT_R32G32_SINT,
            VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32B32_UINT,
            VK_FORMAT_R32G32B32_SINT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32A32_UINT,
            VK_FORMAT_R32G32B32A32_SINT,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_R64_UINT,
            VK_FORMAT_R64_SINT,
            VK_FORMAT_R64_SFLOAT,
            VK_FORMAT_R64G64_UINT,
            VK_FORMAT_R64G64_SINT,
            VK_FORMAT_R64G64_SFLOAT,
            VK_FORMAT_R64G64B64_UINT,
            VK_FORMAT_R64G64B64_SINT,
            VK_FORMAT_R64G64B64_SFLOAT,
            VK_FORMAT_R64G64B64A64_UINT,
            VK_FORMAT_R64G64B64A64_SINT,
            VK_FORMAT_R64G64B64A64_SFLOAT,
            VK_FORMAT_B10G11R11_UFLOAT_PACK32,
            VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_X8_D24_UNORM_PACK32,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_BC1_RGB_UNORM_BLOCK,
            VK_FORMAT_BC1_RGB_SRGB_BLOCK,
            VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
            VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
            VK_FORMAT_BC2_UNORM_BLOCK,
            VK_FORMAT_BC2_SRGB_BLOCK,
            VK_FORMAT_BC3_UNORM_BLOCK,
            VK_FORMAT_BC3_SRGB_BLOCK,
            VK_FORMAT_BC4_UNORM_BLOCK,
            VK_FORMAT_BC4_SNORM_BLOCK,
            VK_FORMAT_BC5_UNORM_BLOCK,
            VK_FORMAT_BC5_SNORM_BLOCK,
            VK_FORMAT_BC6H_UFLOAT_BLOCK,
            VK_FORMAT_BC6H_SFLOAT_BLOCK,
            VK_FORMAT_BC7_UNORM_BLOCK,
            VK_FORMAT_BC7_SRGB_BLOCK,
            VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
            VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
            VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
            VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
            VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
            VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
            VK_FORMAT_EAC_R11_UNORM_BLOCK,
            VK_FORMAT_EAC_R11_SNORM_BLOCK,
            VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
            VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
            VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
            VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
            VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
            VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
            VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
            VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
            VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
            VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
            VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
            VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
            VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
            VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
            VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
            VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
            VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
            VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
            VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
            VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
            VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
            VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
            VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
            VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
            VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
            VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
            VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
            VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
            VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
            VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
            VK_FORMAT_G8B8G8R8_422_UNORM,
            VK_FORMAT_B8G8R8G8_422_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
            VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
            VK_FORMAT_R10X6_UNORM_PACK16,
            VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
            VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
            VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
            VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
            VK_FORMAT_R12X4_UNORM_PACK16,
            VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
            VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
            VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
            VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
            VK_FORMAT_G16B16G16R16_422_UNORM,
            VK_FORMAT_B16G16R16G16_422_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
            VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
            VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
            VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
            VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
            VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
            VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
            VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
            VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
            VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
            VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
                /*
            VK_FORMAT_G8B8G8R8_422_UNORM_KHRVK_FORMAT_G8B8G8R8_422_UNORM,
            VK_FORMAT_B8G8R8G8_422_UNORM_KHRVK_FORMAT_B8G8R8G8_422_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHRVK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHRVK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHRVK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
            VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHRVK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHRVK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
            VK_FORMAT_R10X6_UNORM_PACK16_KHRVK_FORMAT_R10X6_UNORM_PACK16,
            VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHRVK_FORMAT_R10X6G10X6_UNORM_2PACK16,
            VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHRVK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
            VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHRVK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
            VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHRVK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHRVK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHRVK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHRVK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHRVK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHRVK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
            VK_FORMAT_R12X4_UNORM_PACK16_KHRVK_FORMAT_R12X4_UNORM_PACK16,
            VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHRVK_FORMAT_R12X4G12X4_UNORM_2PACK16,
            VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHRVK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
            VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHRVK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
            VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHRVK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHRVK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHRVK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHRVK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHRVK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHRVK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
            VK_FORMAT_G16B16G16R16_422_UNORM_KHRVK_FORMAT_G16B16G16R16_422_UNORM,
            VK_FORMAT_B16G16R16G16_422_UNORM_KHRVK_FORMAT_B16G16R16G16_422_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHRVK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
            VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHRVK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHRVK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
            VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHRVK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHRVK_FORMAT_G16_B16_R16_3PLANE_444_UNORM
            */
        };
        std::map<VkFormat, VkFormatProperties> supportedFormats;
        for (auto format : allFormats) {
            vkGetPhysicalDeviceFormatProperties(
                device,
                format,
                &supportedFormats[format]);
        }

        std::map<VkFormat, VkFormatProperties> vertexBufferFormats;
        for (auto [format, flags] : supportedFormats) {
            if (flags.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) {
                vertexBufferFormats[format] = flags;
            }
        }

        return findQueueFamilies(device, surface).isComplete()
            && checkDeviceExtensionSupport(device)
            && checkSwapChainSupport(querySwapChainSupport(device, surface));
    }

    inline VkPhysicalDevice pickPhysicalDevice(VkInstance const &instance, VkSurfaceKHR const surface) {
        uint32_t deviceCount = 0;
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read physical device count"));
            }
        }
        if (deviceCount == 0) {
            BOOST_THROW_EXCEPTION(std::exception("failed to find GPUs with Vulkan support!"));
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data())};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "failed to read physical devices"));
            }
        }
        devices.resize(deviceCount);
        auto const suitableDeviceIt{boost::find_if(devices, [&surface](VkPhysicalDevice device) {return isDeviceSuitable(device, surface);})};
        if (suitableDeviceIt == std::end(devices)) {
            BOOST_THROW_EXCEPTION(std::exception("failed to find a suitable GPU!"));
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
                BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read swapchain image count"));
            }
        }
        swapChainImages.resize(imageCount);
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read swapchain images"));
            }
        }
        swapChainImages.resize(imageCount);
        return swapChainImages;
    }
    inline std::vector<VulkanImageView> createSwapChainImageViews(
        VkDevice const device, VkFormat const imageFormat, std::vector<VkImage> const &swapChainImages) {
        std::vector<VulkanImageView> swapChainImageViews;
        auto rng{ swapChainImages | boost::adaptors::transformed([device, imageFormat](auto const &image) {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = image;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            return VulkanImageView{device, createInfo};
        }) };
        swapChainImageViews.assign(std::make_move_iterator(std::begin(rng)), std::make_move_iterator(std::end(rng)));

        return swapChainImageViews;
    }
    inline std::vector<VulkanFramebuffer> createSwapchainFramebuffers(
        VkDevice const device,
        VkRenderPass const renderPass,
        VkExtent2D const swapChainExtent,
        std::vector<VulkanImageView> const &swapChainImageViews)
    {
        std::vector<VulkanFramebuffer> framebuffers;
        framebuffers.reserve(swapChainImageViews.size());
        for (auto const &swapChainImageView : swapChainImageViews) {
            framebuffers.emplace_back(device, swapChainImageView.imageView, renderPass, swapChainExtent);
        }
        return framebuffers;
    }
    /*
    inline std::vector<VkCommandBuffer> createCommandBuffers(
        std::vector<VulkanFramebuffer> const &swapChainFramebuffers,
        VkCommandPool const commandPool,
        VkDevice const device,
        VkRenderPass const renderPass,
        VkExtent2D const swapChainExtent,
        VkPipeline const graphicsPipeline
    )
    {
        std::vector<VkCommandBuffer> commandBuffers(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::exception("failed to allocate command buffers!");
        }

        for (std::size_t i{0}, end{commandBuffers.size()}; i < end; ++i) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::exception("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i].framebuffer;
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;

            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::exception("failed to record command buffer!");
            }
        }
        return commandBuffers;
    }
    */
    /*
    inline std::vector<VkCommandBuffer> createCommandBuffers2(
        VkCommandPool const commandPool,
        VkDevice const device
    )
    {
        std::vector<VkCommandBuffer> commandBuffers(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::exception("failed to allocate command buffers!");
        }

        return commandBuffers;
    }
    */
    inline std::vector<VulkanSemaphore> createImageAvailableSemaphores(VkDevice const device) {
        std::vector<VulkanSemaphore> s;
        for (auto i{0}; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            s.emplace_back(device);
        }
        return s;
    }
    inline std::vector<VulkanSemaphore> createRenderFinishedSemaphores(VkDevice const device) {
        std::vector<VulkanSemaphore> s;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            s.emplace_back(device);
        }
        return s;
    }
    inline std::vector<VulkanFence> createInFlightFences(VkDevice const device) {
        std::vector<VulkanFence> f;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            f.emplace_back(device);
        }
        return f;
    }
    /*
    inline std::vector<VulkanBuffer> createVertexBuffers(VkDevice const device) {
        std::vector<VulkanBuffer> b;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            b.emplace_back(device);
        }
        return b;
    }*/
    /*
    inline std::vector<VulkanMemory> createVertexMemories(VkDevice const device) {
        std::vector<VulkanMemory> m;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            m.emplace_back(device);
        }
        return m;
    }*/
    inline VkExtent2D getFramebufferSize(GLFWwindow &w) {
        int width, height;
        glfwGetFramebufferSize(&w, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }
    class VulkanEngine final {
        static void framebufferResizeCallback(GLFWwindow* const window, int const width, int const height) {
            auto eng = static_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            std::lock_guard<std::mutex> lock(eng->frameBufferSizeMutex);
            //eng->framebufferResized = true;
            eng->newFramebufferSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        }

    public:
        VulkanEngine(
            GLFWwindow &w
        ) : w(&w)
          , instance(makeInstanceCreateInfo(nullptr, getRequiredExtensions()))
          , oldFramebufferSize(getFramebufferSize(w))
          , newFramebufferSize(getFramebufferSize(w))
          , debugCallback(instance.i)
          , surface(instance.i, &w)
          , physicalDevice(pickPhysicalDevice(instance.i, surface.surface))
          , logicalDevice(physicalDevice, surface.surface)
          , swapChain(physicalDevice, logicalDevice.device, surface.surface, oldFramebufferSize, VK_NULL_HANDLE)
          , swapChainImages(createSwapChainImages(logicalDevice.device, swapChain.swapChain, swapChain.imageCount))
          , swapChainImageViews(createSwapChainImageViews(logicalDevice.device, swapChain.surfaceFormat.format, swapChainImages))
            
          , renderPass(logicalDevice.device, swapChain.surfaceFormat.format)
            /*
          , descriptorSetLayout(logicalDevice.device, makeDescriptorSetLayoutCreateInfo(makeUboLayoutBinding()))
          , pipelineLayout(logicalDevice.device, swapChain.extent, descriptorSetLayout.descriptorSetLayout)
          , graphicsPipeline(logicalDevice.device, swapChain.extent, pipelineLayout.pipelineLayout, renderPass.renderPass)
          */
          , swapChainFramebuffers(createSwapchainFramebuffers(logicalDevice.device, renderPass.renderPass, swapChain.extent, swapChainImageViews))
          //, commandPool(logicalDevice.device, physicalDevice, surface.surface)
          //, commandBuffers(createCommandBuffers(swapChainFramebuffers, commandPool.commandPool, logicalDevice.device, renderPass.renderPass, swapChain.extent, graphicsPipeline.graphicsPipeline))
          //, commandBuffers2(createCommandBuffers2(commandPool.commandPool, logicalDevice.device))
          //, vertexBufferMemories(createVertexMemories(logicalDevice.device))
         // , vertexBuffers(createVertexBuffers(logicalDevice.device))
          , imageAvailableSemaphores(createImageAvailableSemaphores(logicalDevice.device))
          , renderFinishedSemaphores(createRenderFinishedSemaphores(logicalDevice.device))
          , inFlightFences(createInFlightFences(logicalDevice.device))
          , currentFrame(0)
          //, framebufferResized(false)
        {
            glfwSetWindowUserPointer(&w, this);
            glfwSetFramebufferSizeCallback(&w, framebufferResizeCallback);
            //glfwSetWindowRefreshCallback(&w, windowRefreshCallback); //TODO
        }


        void recreateSwapChain(RunningGameSceneRenderer &renderer){
            vkDeviceWaitIdle(logicalDevice.device);

            VulkanSwapChain newSwapChain(physicalDevice, logicalDevice.device, surface.surface, oldFramebufferSize, swapChain.swapChain);
            std::vector<VkImage> newSwapChainImages(createSwapChainImages(logicalDevice.device, newSwapChain.swapChain, newSwapChain.imageCount));
            std::vector<VulkanImageView> newSwapChainImageViews(createSwapChainImageViews(logicalDevice.device, newSwapChain.surfaceFormat.format, newSwapChainImages));
            VulkanRenderPass newRenderPass(logicalDevice.device, newSwapChain.surfaceFormat.format);
            std::vector<VulkanFramebuffer> newSwapChainFramebuffers(createSwapchainFramebuffers(logicalDevice.device, newRenderPass.renderPass, newSwapChain.extent, newSwapChainImageViews));

            renderer.updateSwapChainData(newRenderPass.renderPass, newSwapChain.extent);

            swapChain = std::move(newSwapChain);
            swapChainImages = std::move(swapChainImages);
            swapChainImageViews = std::move(newSwapChainImageViews);
            renderPass = std::move(newRenderPass);
            swapChainFramebuffers = std::move(newSwapChainFramebuffers);
        }
        bool framebufferResizedCheck() {
            std::lock_guard<std::mutex> lock(frameBufferSizeMutex);
            if (oldFramebufferSize.width != newFramebufferSize.width || oldFramebufferSize.height != newFramebufferSize.height) {
                oldFramebufferSize = newFramebufferSize;
                return true;
            }
            return false;
        }
        void drawFrame(RunningGameSceneRenderer &renderer) {
            int width, height;
            glfwGetFramebufferSize(w, &width, &height);
            if (width == 0 || height == 0) {
                return;
            }
            {
                auto const res{vkWaitForFences(logicalDevice.device, 1, &inFlightFences[currentFrame].fence, VK_TRUE, std::numeric_limits<uint64_t>::max())};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't wait for fence"));
                }
            }
            uint32_t imageIndex;

            while (true) {
                auto const res{ vkAcquireNextImageKHR(logicalDevice.device, swapChain.swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame].semaphore, VK_NULL_HANDLE, &imageIndex)};
                if (res == VK_ERROR_OUT_OF_DATE_KHR) {
                    recreateSwapChain(renderer);
                    continue;
                }
                else if (!(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't Acquire next Image"));
                }
                break;
            }
            auto const renderedCommandBuffers{renderer.renderFrame(currentFrame, swapChainFramebuffers[imageIndex].framebuffer)};

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].semaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = gsl::narrow<uint32_t>(renderedCommandBuffers.size());
            submitInfo.pCommandBuffers = renderedCommandBuffers.data();

            VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].semaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            {
                auto const res{vkResetFences(logicalDevice.device, 1, &inFlightFences[currentFrame].fence)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't reset fence"));
                }
            }
            {
                auto const res{vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame].fence)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to submit draw command buffer!"));
                }
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { swapChain.swapChain };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;

            presentInfo.pImageIndices = &imageIndex;

            {
                auto const res{vkQueuePresentKHR(logicalDevice.presentQueue, &presentInfo)};
                if (res == VK_ERROR_OUT_OF_DATE_KHR || framebufferResizedCheck()) {
                    recreateSwapChain(renderer);
                }
                else if (!(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't present queue"));
                }
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
#if 0
        void drawFrame(std::vector<vec2<float>> const &vertices = std::vector<vec2<float>>{{0.f,-.5f},{0.5f,0.5f},{-0.5f,0.5f},{-1.f,0.f},{-1.f,-1.f},{0.f,-1.f} }) {
            if (vkWaitForFences(logicalDevice.device, 1, &inFlightFences[currentFrame].fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != VK_SUCCESS) {
                throw std::exception("Couldn't wait for fence");
            }
            if (vkResetFences(logicalDevice.device, 1, &inFlightFences[currentFrame].fence) != VK_SUCCESS) {
                throw std::exception("Couldn't reset fence");
            }

            uint32_t imageIndex;
            if (vkAcquireNextImageKHR(logicalDevice.device, swapChain.swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame].semaphore, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) {
                throw std::exception("Couldn't Acquire next Image");
            }

            auto &vertexBuffer{ vertexBuffers[currentFrame] };
            //TODO: Don't allocate memory every frame; reuse buffers between frames if they are still large enough!
            vertexBuffer = VulkanBuffer(logicalDevice.device);
            {
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = sizeof(vertices[0]) * vertices.size();
                bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                vertexBuffer = VulkanBuffer(logicalDevice.device, bufferInfo);
            }
            auto &vertexBufferMemory{vertexBufferMemories[currentFrame]};
            vertexBufferMemory = VulkanMemory(logicalDevice.device);
            {
                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(logicalDevice.device, vertexBuffer.buffer, &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vertexBufferMemory = VulkanMemory(logicalDevice.device, allocInfo);
            }
            if (vkBindBufferMemory(logicalDevice.device, vertexBuffer.buffer, vertexBufferMemory.memory, 0) != VK_SUCCESS) {
                throw std::exception("Couldn't bind buffer memory");
            }

            void* data;
            if (vkMapMemory(logicalDevice.device, vertexBufferMemory.memory, 0, sizeof(vertices[0]) * vertices.size(), 0, &data) != VK_SUCCESS){
                throw std::exception("Couldn't map memory for vertex buffer");
            }
            memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
            vkUnmapMemory(logicalDevice.device, vertexBufferMemory.memory);

            auto const &commandBuffer{commandBuffers2[currentFrame]};
            {
                if (vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                    throw std::exception("Couldn't reset command buffer!");
                }

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

                if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::exception("failed to begin recording command buffer!");
                }

                VkRenderPassBeginInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass.renderPass;
                renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex].framebuffer;
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = swapChain.extent;

                VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clearColor;

                vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);

                VkBuffer vertexBufferArr[] = { vertexBuffers[currentFrame].buffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBufferArr, offsets);

                vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

                vkCmdEndRenderPass(commandBuffer);

                if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                    throw std::exception("failed to record command buffer!");
                }
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].semaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].semaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame].fence) != VK_SUCCESS) {
                throw std::exception("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { swapChain.swapChain };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;

            presentInfo.pImageIndices = &imageIndex;

            if (vkQueuePresentKHR(logicalDevice.presentQueue, &presentInfo) != VK_SUCCESS) {
                throw std::exception("Couldn't present queue");
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
#endif
        VulkanEngine(VulkanEngine const&) = delete;
        VulkanEngine(VulkanEngine &&) = delete;
        VulkanEngine &operator=(VulkanEngine const&) = delete;
        VulkanEngine &operator=(VulkanEngine &&) = delete;
        ~VulkanEngine() noexcept {
            vkDeviceWaitIdle(logicalDevice.device); //Something like this is needed, but maybe not this exact implementation
        }
        GLFWwindow *w;
        std::mutex frameBufferSizeMutex;
        VkExtent2D oldFramebufferSize;
        VkExtent2D newFramebufferSize;
        VulkanInstance instance;
        VulkanDebugCallback debugCallback;
        VulkanSurface surface;
        VkPhysicalDevice physicalDevice;
        VulkanLogicalDevice logicalDevice;
        VulkanSwapChain swapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VulkanImageView> swapChainImageViews;
        VulkanRenderPass renderPass;
        /*
        VulkanDescriptorSetLayout descriptorSetLayout;
        VulkanPipelineLayout pipelineLayout;
        VulkanGraphicsPipeline graphicsPipeline;
        */
        std::vector<VulkanFramebuffer> swapChainFramebuffers;
        //VulkanCommandPool commandPool;
        //std::vector<VkCommandBuffer> commandBuffers;
        //std::vector<VkCommandBuffer> commandBuffers2;
        //std::vector<VulkanMemory> vertexBufferMemories;
        //std::vector<VulkanBuffer> vertexBuffers;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkAcquireNextImageKHR has finished/vkQueueSubmit can start
        std::vector<VulkanSemaphore> imageAvailableSemaphores;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkQueueSubmit has finished/vkQueuePresentKHR can start
        std::vector<VulkanSemaphore> renderFinishedSemaphores;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkQueueSubmit has finished/vkAcquireNextImageKHR for next frame can start
        //(to limit frames-in-flight to MAX_FRAMES_IN_FLIGHT)
        std::vector<VulkanFence> inFlightFences;
        std::size_t currentFrame;
        //bool framebufferResized;
    private:
    };
}
#endif // !HG_VULKANENGINE_H
