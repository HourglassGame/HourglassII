#ifndef HG_VULKANENGINE_H
#define HG_VULKANENGINE_H
#include "hg/Util/util.h"
#include "hg/VulkanUtilHG/VulkanSwapChainHG.h"
#include "hg/VulkanUtil/VulkanImageView.h"
#include "hg/VulkanUtil/VulkanSurface.h"
#include "hg/VulkanUtilHG/VulkanRenderPassHG.h"
#include "hg/VulkanUtil/VulkanInstance.h"
#include "hg/VulkanUtilHG/VulkanDebugCallbackHG.h"
#include "hg/VulkanUtil/VulkanUtil.h"
#include "hg/VulkanUtilHG/VulkanDeviceHG.h"
#include "hg/VulkanUtil/VulkanPipelineLayout.h"
#include "hg/VulkanUtilHG/VulkanFramebufferHG.h"
#include "hg/VulkanUtilHG/VulkanCommandPoolHG.h"
#include "hg/VulkanUtilHG/VulkanGraphicsPipelineHG.h"
#include "hg/VulkanUtilHG/VulkanFenceHG.h"
#include "hg/VulkanUtilHG/VulkanSemaphoreHG.h"
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

    class VulkanEngine final {
        static void framebufferResizeCallback(GLFWwindow* const window, int const width, int const height);

    public:
        VulkanEngine(
            GLFWwindow &w
        );

        void recreateSwapChain(VulkanRenderer &renderer);

        bool framebufferResizedCheck();

        void drawFrame(VulkanRenderer &renderer);

        void idle(VulkanRenderer &vkRenderer);

        bool waitForFences(std::vector<int> const &frameNumbers) {
            //TODO
        }
        VulkanEngine(VulkanEngine const&) = delete;
        VulkanEngine(VulkanEngine &&) = delete;
        VulkanEngine &operator=(VulkanEngine const&) = delete;
        VulkanEngine &operator=(VulkanEngine &&) = delete;
        ~VulkanEngine() noexcept {
            vkDeviceWaitIdle(device.h()); //Something like this is needed, but maybe not this exact implementation
        }
    private:
        GLFWwindow *w;

        tbb::queuing_mutex frameBufferSizeMutex;
        VkExtent2D oldFramebufferSize;
        VkExtent2D newFramebufferSize;
        VulkanInstance instance;
        VulkanDebugCallbackHG debugCallback;
    public:
        VulkanSurface surface;
        PossiblePhysicalDevice physicalDevice;
        VulkanDeviceHG device;
        VulkanSwapChainHG swapChain;
    private:
        std::vector<VkImage> swapChainImages;
        std::vector<VulkanImageView> swapChainImageViews;
    public:
        VulkanRenderPassHG renderPass;
    private:
        std::vector<VulkanFramebufferHG> swapChainFramebuffers;

        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkAcquireNextImageKHR has finished/vkQueueSubmit can start
        std::vector<VulkanSemaphoreHG> imageAvailableSemaphores;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkQueueSubmit has finished/vkQueuePresentKHR can start
        std::vector<VulkanSemaphoreHG> renderFinishedSemaphores;
        //1 per acquired image (up to MAX_FRAMES_IN_FLIGHT),
        //vkQueueSubmit has finished/vkAcquireNextImageKHR for next frame can start
        //(to limit frames-in-flight to MAX_FRAMES_IN_FLIGHT)
        std::vector<VulkanFenceHG> inFlightFences;
        std::size_t currentFrame;
    };
}
#endif // !HG_VULKANENGINE_H
