#ifndef HG_VULKAN_RENDERER_H
#define HG_VULKAN_RENDERER_H

#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/find.hpp>
#include <atomic>
namespace hg {
    /*
    The renderer thread wants to:
      Call renderFrame and updateSwapChainData, as appropriate.
      Forward renderFrame and updateSwapChainData calls to the appropriate Scene.

      Ensure that a Scene's vulkan objects are not deleted while they are still in the render pipeline.

      Ensure that calls to the Scene only occur within the Scene's lifetime.

      Seamlessly transition between scenes, without flushing the render pipeline.

    The UI thread wants to:
      Maybe be able to single-step the rendering.
      Maybe manage the lifetime of the scenes, but definitely not any vulkan objects in the scene that are required for rendering.
       (or alternatively, also manage the lifetimes of the vulkan objects, via notifications... so it can choose to destroy or not destroy,
        depending on whether they will be reused elsewhere)
      Possibly ownership of the vulkan data needs to be shared between the ui thread and the renderer thread (e.g via shared_ptr)
       with primary ownerwhip being by the ui thread, and the renderer thread acquiring ownership just to extend the lifetime until
       vulkan is done with the relevant objects.
      Be able to switch between scenes.
      Be able to supply frame data to scenes.
      Not block, if at all possible.
      Be notified of errors, and have some way to recover.


    */
    class VulkanDataKeepAlive {
    public:
        template<typename T>
        explicit VulkanDataKeepAlive(std::shared_ptr<T> const &v) :
            keeper(v)
        {}
        VulkanDataKeepAlive() = delete;
        VulkanDataKeepAlive(VulkanDataKeepAlive const &) = delete;
        VulkanDataKeepAlive &operator=(VulkanDataKeepAlive const &) = delete;
        VulkanDataKeepAlive(VulkanDataKeepAlive &&) = default;
        VulkanDataKeepAlive &operator=(VulkanDataKeepAlive &&) = default;
    private:
        std::shared_ptr<void> keeper;
    };

    class SceneRenderer {
    public:
        virtual VulkanDataKeepAlive getSharedVulkanData()=0;
        virtual VulkanDataKeepAlive getFrameVulkanData(std::size_t const currentFrame)=0;

        virtual void updateSwapChainData(
            VkRenderPass const renderPass,
            VkExtent2D const &swapChainExtent) = 0;

        virtual std::vector<VkCommandBuffer> renderFrame(
            std::size_t const currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
            VkFramebuffer const targetFrameBuffer)=0;
    protected:
        ~SceneRenderer() {}
    };
    struct SceneLock {
        bool shouldDraw() const {
            return hasScene;
        }
        mutable std::unique_lock<std::mutex> l;
        bool hasScene;
    };
    class VulkanRenderer {
    public:
        VulkanRenderer() :
            sceneMutex()
           ,scene()
           ,frameKeepAlives(MAX_FRAMES_IN_FLIGHT)
           ,sceneKeepAlives()
        {}
        void updateSwapChainData(
            VkRenderPass const renderPass,
            VkExtent2D const &swapChainExtent)
        {
            Expects(scene);
            scene->updateSwapChainData(renderPass, swapChainExtent);
        }
        //TODO: add some way to wait on the frame fences without drawing another frame;
        //so that in between scenes the old scene data can be released quickly.
        std::vector<VkCommandBuffer> renderFrame(
            std::size_t const currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
            VkFramebuffer const targetFrameBuffer)
        {
            Expects(scene);
            Expects(currentFrame < MAX_FRAMES_IN_FLIGHT);
            {
                std::scoped_lock l{ keepAliveMutex };
                frameEndedNoLock(currentFrame);
                currentSceneKeepAlive->framesInUse.insert(currentFrame);
                frameKeepAlives[currentFrame] = std::optional(scene->getFrameVulkanData(currentFrame));
            }
            return scene->renderFrame(currentFrame, targetFrameBuffer);
        }
        void frameEnded(std::size_t const frame)
        {
            std::scoped_lock l{keepAliveMutex};
            frameEndedNoLock(frame);
        }
#if 0
        void frameDone(std::size_t const currentFrame) {
            //TODO??
            //release resources used by currentFrame
        }
#endif
        SceneLock lockScene() {
            bool const hasScene{sceneAlive};
            if (hasScene) {
                std::unique_lock l{ sceneMutex };
                return { std::move(l), gsl::narrow_cast<bool>(scene) };
            }
            else {
                return { {}, false };
            }
        }

        void StartScene(SceneRenderer &newScene) {
            std::lock_guard l{sceneMutex};
            std::lock_guard l2{keepAliveMutex};
            scene = &newScene;
            currentSceneKeepAlive = SceneKeepAlive{std::set<std::size_t>{}, scene->getSharedVulkanData()};
            sceneAlive = true;
        }
        //TODO: Make helper object that calls EndScene in its destructor?
        void EndScene() {
            sceneAlive = false;
            std::lock_guard l{sceneMutex};
            {
                std::lock_guard l2{ keepAliveMutex };
                scene = nullptr;
                sceneKeepAlives.emplace_back(std::move(*currentSceneKeepAlive));
                currentSceneKeepAlive.reset();
            }
        }
        bool hasNoKeepAlives(){
            std::lock_guard l{keepAliveMutex};
            return boost::find_if(frameKeepAlives, [](auto const&a) {return a.has_value(); }) == boost::end(frameKeepAlives)
                && sceneKeepAlives.empty()
                && !currentSceneKeepAlive.has_value();
        }

        ~VulkanRenderer() {
            //TODO: use non-locking version of hasNoKeepAlives, for performance?
            while (!hasNoKeepAlives()) {}
        }
    private:
        void frameEndedNoLock(std::size_t const frame)
        {
            frameKeepAlives[frame].reset();
            boost::for_each(sceneKeepAlives, [frame](auto& s) {s.framesInUse.erase(frame); });
            boost::remove_erase_if(sceneKeepAlives, [](auto const& s) {return s.framesInUse.empty(); });
        }
        mutable std::mutex sceneMutex;
        std::atomic<bool> sceneAlive;
        SceneRenderer *scene;

        mutable std::mutex keepAliveMutex;
        std::vector<std::optional<VulkanDataKeepAlive>> frameKeepAlives;
        struct SceneKeepAlive{
            std::set<std::size_t> framesInUse;
            VulkanDataKeepAlive sceneKeepAlive;
        };

        //TODO: Use (circular?) queue rather than vector?
        std::optional<SceneKeepAlive> currentSceneKeepAlive;
        std::vector<SceneKeepAlive> sceneKeepAlives;
    };
}

#endif // !HG_VULKAN_RENDERER_H
