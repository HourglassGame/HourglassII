#ifndef HG_VULKANENGINE_H
#define HG_VULKANENGINE_H
#include "hg/Util/util.h"
#include "hg/VulkanUtil/VulkanSwapChain.h"
#include "hg/VulkanUtil/VulkanImageView.h"
#include "hg/VulkanUtil/VulkanSurface.h"
#include "hg/VulkanUtilHG/VulkanRenderPassHG.h"
#include "hg/VulkanUtil/VulkanInstance.h"
#include "hg/VulkanUtilHG/VulkanDebugCallbackHG.h"
#include "hg/VulkanUtil/VulkanUtil.h"
#include "hg/VulkanUtilHG/VulkanLogicalDeviceHG.h"
#include "hg/VulkanUtil/VulkanPipelineLayout.h"
#include "hg/VulkanUtilHG/VulkanFramebufferHG.h"
#include "hg/VulkanUtilHG/VulkanCommandPoolHG.h"
#include "hg/VulkanUtilHG/VulkanGraphicsPipelineHG.h"
#include "hg/VulkanUtilHG/VulkanFenceHG.h"
#include "hg/VulkanUtil/VulkanSemaphore.h"
#include "hg/VulkanUtil/VulkanMemory.h"
#include "hg/VulkanUtil/VulkanBuffer.h"
#include "hg/VulkanUtil/VulkanDescriptorSetLayout.h"
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
#include "hg/GlobalConst.h"
#include "hg/VulkanUtil/VulkanExceptions.h"
#include "VulkanRenderer.h"
#include <tbb/queuing_mutex.h>
#include <boost/throw_exception.hpp>
#include <system_error>
namespace hg {

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
#if 0
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
#if 0
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        auto const allFormats{std::array{
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
        }};

        /*
        std::map<VkFormat, VkFormatProperties> supportedFormats;
        for (auto const format : allFormats) {
            vkGetPhysicalDeviceFormatProperties(
                device,
                format,
                &supportedFormats[format]);
        }
        */
        /*
        std::map<VkFormat, VkFormatProperties> vertexBufferFormats;
        for (auto [format, flags] : supportedFormats) {
            if (flags.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) {
                vertexBufferFormats[format] = flags;
            }
        }
        */
#endif
        return findQueueFamilies(device, surface).isComplete()
            && checkDeviceExtensionSupport(device)
            && checkSwapChainSupport(querySwapChainSupport(device, surface));
    }
    inline VkPhysicalDevice pickPhysicalDevice(VkInstance const &instance, VkSurfaceKHR const surface) {
        uint32_t deviceCount{0};
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
#endif
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
    inline std::vector<VulkanFramebufferHG> createSwapchainFramebuffers(
        VkDevice const device,
        VkRenderPass const renderPass,
        VkExtent2D const swapChainExtent,
        std::vector<VulkanImageView> const &swapChainImageViews)
    {
        std::vector<VulkanFramebufferHG> framebuffers;
        framebuffers.reserve(swapChainImageViews.size());
        for (auto const &swapChainImageView : swapChainImageViews) {
            framebuffers.emplace_back(device, swapChainImageView.imageView, renderPass, swapChainExtent);
        }
        return framebuffers;
    }
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
    inline std::vector<VulkanFenceHG> createInFlightFences(VkDevice const device) {
        std::vector<VulkanFenceHG> f;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            f.emplace_back(device);
        }
        return f;
    }
    inline VkExtent2D getFramebufferSize(GLFWwindow &w) {
        int width, height;
        glfwGetFramebufferSize(&w, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }
    class VulkanEngine final {
        static void framebufferResizeCallback(GLFWwindow* const window, int const width, int const height) {
            auto const eng{static_cast<VulkanEngine*>(glfwGetWindowUserPointer(window))};
            decltype(frameBufferSizeMutex)::scoped_lock lock(eng->frameBufferSizeMutex);
            eng->newFramebufferSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            //TODO: There may be a race condition here between the time this lock is released and when the control returns to the event loop.
            //Need to add some additional locks to make sure the renderer doesn't run during this period of time.
            //(My understanding is that the framebuffer gets resized once the control has returned to the event loop;
            // so ideally we would wait until the next event is processed by the loop, to be sure that this processing has completed)


            //https://github.com/KhronosGroup/Vulkan-Docs/issues/1144
            //https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1340
            //https://github.com/KhronosGroup/Vulkan-Docs/issues/388
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
          , physicalDevice(
              [&]{
                  auto const possiblePhysicalDevices{enumerateSuitablePhysicalDevices(instance.i, surface.surface)};
                  if (possiblePhysicalDevices.empty()) {
                      BOOST_THROW_EXCEPTION(std::exception("Gailed to find GPU supporting features required for Hourglass II"));
                  }
                  return possiblePhysicalDevices.front();
              }())
          , logicalDevice(physicalDevice, surface.surface)
            //TODO: if frameBufferWidth or Height is 0, don't create the swapChain; rather than
            //making an invalid swapChain!
            //Attempting to make a swapChain with width or height 0 violates the vulkan spec!
          , swapChain(physicalDevice, logicalDevice.h(), surface.surface, oldFramebufferSize, VK_NULL_HANDLE)
          , swapChainImages(createSwapChainImages(logicalDevice.h(), swapChain.swapChain, swapChain.imageCount))
          , swapChainImageViews(createSwapChainImageViews(logicalDevice.h(), swapChain.surfaceFormat.format, swapChainImages))
          , renderPass(logicalDevice.h(), swapChain.surfaceFormat.format)
          , swapChainFramebuffers(createSwapchainFramebuffers(logicalDevice.h(), renderPass.h(), swapChain.extent, swapChainImageViews))
          , imageAvailableSemaphores(createImageAvailableSemaphores(logicalDevice.h()))
          , renderFinishedSemaphores(createRenderFinishedSemaphores(logicalDevice.h()))
          , inFlightFences(createInFlightFences(logicalDevice.h()))
          , currentFrame(0)
        {
            glfwSetWindowUserPointer(&w, this);
            glfwSetFramebufferSizeCallback(&w, framebufferResizeCallback);
            //glfwSetWindowRefreshCallback(&w, windowRefreshCallback); //TODO
        }


        void recreateSwapChain(VulkanRenderer &renderer){
            vkDeviceWaitIdle(logicalDevice.h());
            framebufferResizedCheck();

            auto const maxImageExtent{querySwapChainMaxImageExtent(physicalDevice.physicalDevice, surface.surface)};

            if (oldFramebufferSize.width == 0 || oldFramebufferSize.height == 0
             || maxImageExtent.width == 0 || maxImageExtent.height == 0) {
                //Can't render into 0-size swapchain.
                return;
            }
            VulkanSwapChain newSwapChain(physicalDevice, logicalDevice.h(), surface.surface, oldFramebufferSize, swapChain.swapChain);
            std::vector<VkImage> newSwapChainImages(createSwapChainImages(logicalDevice.h(), newSwapChain.swapChain, newSwapChain.imageCount));
            std::vector<VulkanImageView> newSwapChainImageViews(createSwapChainImageViews(logicalDevice.h(), newSwapChain.surfaceFormat.format, newSwapChainImages));
            VulkanRenderPassHG newRenderPass(logicalDevice.h(), newSwapChain.surfaceFormat.format);
            std::vector<VulkanFramebufferHG> newSwapChainFramebuffers(createSwapchainFramebuffers(logicalDevice.h(), newRenderPass.h(), newSwapChain.extent, newSwapChainImageViews));

            renderer.updateSwapChainData(newRenderPass.h(), newSwapChain.extent);

            swapChain = std::move(newSwapChain);
            swapChainImages = std::move(swapChainImages);
            swapChainImageViews = std::move(newSwapChainImageViews);
            renderPass = std::move(newRenderPass);
            swapChainFramebuffers = std::move(newSwapChainFramebuffers);
        }
        bool framebufferResizedCheck() {
            if (oldFramebufferSize.width != newFramebufferSize.width || oldFramebufferSize.height != newFramebufferSize.height) {
                oldFramebufferSize = newFramebufferSize;
                return true;
            }
            return false;
        }
        void drawFrame(VulkanRenderer &renderer) {
            decltype(frameBufferSizeMutex)::scoped_lock lock(frameBufferSizeMutex);
            {
                auto maxImageExtent{querySwapChainMaxImageExtent(physicalDevice.physicalDevice, surface.surface)};
                //Can't render into 0-size surface
                if (maxImageExtent.width == 0 || maxImageExtent.height == 0) return;
            }
            {
                auto const fences{std::array{inFlightFences[currentFrame].h()}};
                auto const res{vkWaitForFences(logicalDevice.h(), fences.size(), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max())};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't wait for fence"));
                }
            }
            uint32_t imageIndex{};
            while (true) {
                auto const res{ vkAcquireNextImageKHR(logicalDevice.h(), swapChain.swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame].semaphore, VK_NULL_HANDLE, &imageIndex)};
                if (res == VK_ERROR_OUT_OF_DATE_KHR) {
                    recreateSwapChain(renderer);
                    continue;
                }
                else if (!(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't Acquire next Image"));
                }
                break;
            }
            auto const renderedCommandBuffers{renderer.renderFrame(currentFrame, swapChainFramebuffers[imageIndex].h())};

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
                auto const fences{std::array{inFlightFences[currentFrame].h()}};
                auto const res{vkResetFences(logicalDevice.h(), fences.size(), fences.data())};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't reset fence"));
                }
            }
            {
                auto const res{vkQueueSubmit(logicalDevice.graphicsQ(), 1, &submitInfo, inFlightFences[currentFrame].h())};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to submit draw command buffer!"));
                }
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            auto const swapChains{std::array{swapChain.swapChain}};
            presentInfo.swapchainCount = gsl::narrow<uint32_t>(swapChains.size());
            presentInfo.pSwapchains = swapChains.data();

            presentInfo.pImageIndices = &imageIndex;

            {
                auto const res{vkQueuePresentKHR(logicalDevice.presentQ(), &presentInfo)};
                if (res == VK_ERROR_OUT_OF_DATE_KHR || framebufferResizedCheck()) {
                    recreateSwapChain(renderer);
                }
                else if (!(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't present queue"));
                }
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
        void idle(VulkanRenderer &vkRenderer) {
            for(int f{0}; f != MAX_FRAMES_IN_FLIGHT; ++f) {
                {
                    auto const res{vkGetFenceStatus(logicalDevice.h(), inFlightFences[f].h())};
                    if (res == VK_SUCCESS) {
                        vkRenderer.frameEnded(f);
                    }
                    else if (!(res == VK_SUCCESS || res == VK_NOT_READY)) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "Idle getFenceStatus failed"));
                    }
                }
            }
        }
        bool waitForFences(std::vector<int> const &frameNumbers) {
            //TODO
        }
        VulkanEngine(VulkanEngine const&) = delete;
        VulkanEngine(VulkanEngine &&) = delete;
        VulkanEngine &operator=(VulkanEngine const&) = delete;
        VulkanEngine &operator=(VulkanEngine &&) = delete;
        ~VulkanEngine() noexcept {
            vkDeviceWaitIdle(logicalDevice.h()); //Something like this is needed, but maybe not this exact implementation
        }
        GLFWwindow *w;

        tbb::queuing_mutex frameBufferSizeMutex;
        VkExtent2D oldFramebufferSize;
        VkExtent2D newFramebufferSize;
        VulkanInstance instance;
        VulkanDebugCallbackHG debugCallback;
        VulkanSurface surface;
        PossiblePhysicalDevice physicalDevice;
        VulkanLogicalDeviceHG logicalDevice;
        VulkanSwapChain swapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VulkanImageView> swapChainImageViews;
        VulkanRenderPassHG renderPass;

        std::vector<VulkanFramebufferHG> swapChainFramebuffers;

        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkAcquireNextImageKHR has finished/vkQueueSubmit can start
        std::vector<VulkanSemaphore> imageAvailableSemaphores;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkQueueSubmit has finished/vkQueuePresentKHR can start
        std::vector<VulkanSemaphore> renderFinishedSemaphores;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkQueueSubmit has finished/vkAcquireNextImageKHR for next frame can start
        //(to limit frames-in-flight to MAX_FRAMES_IN_FLIGHT)
        std::vector<VulkanFenceHG> inFlightFences;
        std::size_t currentFrame;
    private:
    };
}
#endif // !HG_VULKANENGINE_H
