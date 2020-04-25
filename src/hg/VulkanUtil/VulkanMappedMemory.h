#ifndef HG_VULKAN_MAPPED_MEMORY_H
#define HG_VULKAN_MAPPED_MEMORY_H
#include "VulkanExceptions.h"
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanMappedMemory final {
    public:
        explicit VulkanMappedMemory(
            VkDevice const device,
            VkDeviceMemory const memory
        ) : device(device)
          , memory(memory)
          , mappedMemory(nullptr)
        {}
        explicit VulkanMappedMemory(
            VkDevice         const device,
            VkDeviceMemory   const memory,
            VkDeviceSize     const offset,
            VkDeviceSize     const size,
            VkMemoryMapFlags const flags
        ) : device(device), memory(memory)
        {
            {
                auto const res{vkMapMemory(device, memory, offset, size, flags, &mappedMemory)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to map memory!"));
                }
            }
        }
        VulkanMappedMemory(VulkanMappedMemory const&) = delete;
        VulkanMappedMemory(VulkanMappedMemory &&o) noexcept
            : device(o.device)
            , memory(o.memory)
            , mappedMemory(std::exchange(o.mappedMemory, nullptr))
        {
        }
        VulkanMappedMemory &operator=(VulkanMappedMemory const&) = delete;
        VulkanMappedMemory &operator=(VulkanMappedMemory &&o) noexcept {
            std::swap(device, o.device);
            std::swap(memory, o.memory);
            std::swap(mappedMemory, o.mappedMemory);
            return *this;
        }
        ~VulkanMappedMemory() noexcept {
            if (mappedMemory) {
                vkUnmapMemory(device, memory);
            }
        }
        VkDevice device;
        VkDeviceMemory memory;
        void *mappedMemory;
    };
}
#endif // !HG_VULKAN_MAPPED_MEMORY_H