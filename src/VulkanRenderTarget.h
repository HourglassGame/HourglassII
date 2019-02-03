#ifndef HG_VULKAN_RENDER_TARGET_H
#define HG_VULKAN_RENDER_TARGET_H
#include "VulkanBuffer.h"
#include "VulkanMemory.h"
#include "VulkanMappedMemory.h"
#include "VulkanUtil.h"
#include "VulkanDescriptorPool.h"
#include "Maths.h"
#include "move_function.h"
#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <vector>

#include <vulkan/vulkan.h>
#include <gsl/gsl>
namespace hg {
struct UniformBufferObject {
    float projMatrix[16];
};
//Small initial size for now, to put realloc logic to the test.
//TODO: Investigate a typical size and preallocate to it from the start.
inline int const initialVertexBufSize{ 1 };
inline int const initialUniformBufSize{ sizeof(UniformBufferObject) };
struct VulkanRenderTarget {

    VulkanRenderTarget(
        VkPhysicalDevice const physicalDevice,
        VkDevice const device,

        VkPipelineLayout const pipelineLayout,
        VkDescriptorSetLayout const descriptorSetLayout,
        VkCommandBuffer const preDrawCommandBuffer,
        VkCommandBuffer const drawCommandBuffer)
        : physicalDevice(physicalDevice)
        , device(device)
        , pipelineLayout(pipelineLayout)
        , descriptorSetLayout(descriptorSetLayout)
        , preDrawCommandBuffer(preDrawCommandBuffer)
        , drawCommandBuffer(drawCommandBuffer)

        , endPositionInCurrentVertexBuffer(0)
        , vertexBuffers()
    {
    }


    VulkanRenderTarget(VulkanRenderTarget const&) = delete;
    VulkanRenderTarget(VulkanRenderTarget &&o) noexcept = default;
    VulkanRenderTarget &operator=(VulkanRenderTarget const&) = delete;
    VulkanRenderTarget &operator=(VulkanRenderTarget &&o) noexcept = default;

    void enqueueBufferCopy() {
        //push fence wait to queue
        //Unneeded, since we don't submit the queue until all the writes to the source buffer are complete?
        //vkCmdWaitEvents

        //push memory barrier and buffer transfer commands to queue

        std::array<VkBufferCopy, 1> vertexCopyRegions{
            {
                0,//srcOffset
                0,//dstOffset
                vertexBuffers.back().size
            }
        };

        vkCmdCopyBuffer(
            preDrawCommandBuffer,
            vertexBuffers.back().transferSrcVertexBuffer.buffer,
            vertexBuffers.back().deviceVertexBuffer.buffer,
            gsl::narrow<uint32_t>(vertexCopyRegions.size()),
            vertexCopyRegions.data());


        //reset endPositionInCurrentVertexBuffer
        endPositionInCurrentVertexBuffer = 0;

        //Bind vertex buffer
        std::array<VkDeviceSize, 1> offsets{
            {0}
        };
        std::array<VkBuffer, 1> buffers{
            {vertexBuffers.back().deviceVertexBuffer.buffer}
        };
        vkCmdBindVertexBuffers(drawCommandBuffer, 0, 1, buffers.data(), offsets.data());
    }

    void enqueueUniformBufferCopy() {
        //push fence wait to queue
        //Unneeded, since we don't submit the queue until all the writes to the source buffer are complete?
        //vkCmdWaitEvents

        //push memory barrier and buffer transfer commands to queue

        std::array<VkBufferCopy, 1> uniformCopyRegions{
            {
                0,//srcOffset
                0,//dstOffset
                uniformBuffers.back().size
            }
        };

        vkCmdCopyBuffer(
            preDrawCommandBuffer,
            uniformBuffers.back().transferSrcUniformBuffer.buffer,
            uniformBuffers.back().deviceUniformBuffer.buffer,
            gsl::narrow<uint32_t>(uniformCopyRegions.size()),
            uniformCopyRegions.data());


        //reset endPositionInCurrentVertexBuffer
        //endPositionInCurrentUniformBuffer = 0;

        //Bind vertex buffer
        /*
        std::array<VkDeviceSize, 1> offsets{
            {0}
        };
        std::array<VkBuffer, 1> buffers{
            {uniformBuffers.back().deviceUniformBuffer.buffer}
        };
        vkCmdBindVertexBuffers(drawCommandBuffer, 0, 1, buffers.data(), offsets.data());
        */
    }

    void prepareBufferAndTransfer(std::size_t const size) {
        VulkanBuffer transferSrcVertexBuffer(
            device,
            VkBufferCreateInfo{
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,//sType
                nullptr,//pNext
                0,//flags
                size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,//usage
                VK_SHARING_MODE_EXCLUSIVE,//sharingMode
                0,//queueFamilyIndexCount
                nullptr//pQueueFamilyIndices
            });

        VkMemoryRequirements transferSrcMemRequirements;
        vkGetBufferMemoryRequirements(device, transferSrcVertexBuffer.buffer, &transferSrcMemRequirements);

        //TODO:
        //Check maxMemoryAllocationCount and
        //VkPhysicalDeviceMaintenance3Properties::maxMemoryAllocationSize
        //and adjust behaviour accordingly?

        VulkanMemory transferSrcMemory(
            device,
            VkMemoryAllocateInfo{
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,//sType
                nullptr,//pNext
                transferSrcMemRequirements.size,
                findMemoryType(physicalDevice, transferSrcMemRequirements.size, transferSrcMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT /*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //memoryTypeIndex
            });

        if (vkBindBufferMemory(device, transferSrcVertexBuffer.buffer, transferSrcMemory.memory, 0/*memoryOffset*/) != VK_SUCCESS) {
            throw std::exception("vkBindBufferMemory failed");
        }

        VulkanMappedMemory transferSrcMappedRegion(
            device,
            transferSrcMemory.memory,
            0,//offset
            size,
            0//flags
        );

        //TODO: Don't have separate deviceMemory/deviceVertexBuffer, if srcMemory is device local
        VulkanBuffer deviceVertexBuffer(
            device,
            VkBufferCreateInfo{
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,//sType
                nullptr,//pNext
                0,//flags
                size,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,//usage
                VK_SHARING_MODE_EXCLUSIVE,//sharingMode
                0,//queueFamilyIndexCount
                nullptr//pQueueFamilyIndices
            }
        );
        VkMemoryRequirements deviceVertexBufferMemRequirements;
        vkGetBufferMemoryRequirements(device, deviceVertexBuffer.buffer, &deviceVertexBufferMemRequirements);
        VulkanMemory deviceMemory(
            device,
            VkMemoryAllocateInfo{
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,//sType
                nullptr,//pNext
                deviceVertexBufferMemRequirements.size,
                findMemoryType(physicalDevice, deviceVertexBufferMemRequirements.size, deviceVertexBufferMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            }
        );

        if (vkBindBufferMemory(device, deviceVertexBuffer.buffer, deviceMemory.memory, 0/*memoryOffset*/) != VK_SUCCESS) {
            throw std::exception("vkBindBufferMemory failed");
        }

        vertexBuffers.push_back(
            SizedStagedVertexBuffer{
                size,
                std::move(transferSrcVertexBuffer),
                std::move(transferSrcMemory),
                std::move(transferSrcMappedRegion),
                std::move(deviceVertexBuffer),
                std::move(deviceMemory)
            }
        );
        enqueueBufferCopy();
    }

    void prepareUniformBufferAndTransfer(std::size_t const size) {
        VulkanBuffer transferSrcUniformBuffer(
            device,
            VkBufferCreateInfo{
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,//sType
                nullptr,//pNext
                0,//flags
                size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,//usage
                VK_SHARING_MODE_EXCLUSIVE,//sharingMode
                0,//queueFamilyIndexCount
                nullptr//pQueueFamilyIndices
            });

        VkMemoryRequirements transferSrcMemRequirements;
        vkGetBufferMemoryRequirements(device, transferSrcUniformBuffer.buffer, &transferSrcMemRequirements);

        //TODO:
        //Check maxMemoryAllocationCount and
        //VkPhysicalDeviceMaintenance3Properties::maxMemoryAllocationSize
        //and adjust behaviour accordingly?

        VulkanMemory transferSrcMemory(
            device,
            VkMemoryAllocateInfo{
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,//sType
                nullptr,//pNext
                transferSrcMemRequirements.size,
                findMemoryType(physicalDevice, transferSrcMemRequirements.size, transferSrcMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT /*| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/) //memoryTypeIndex
            });

        if (vkBindBufferMemory(device, transferSrcUniformBuffer.buffer, transferSrcMemory.memory, 0/*memoryOffset*/) != VK_SUCCESS) {
            throw std::exception("vkBindBufferMemory failed");
        }

        VulkanMappedMemory transferSrcMappedRegion(
            device,
            transferSrcMemory.memory,
            0,//offset
            size,
            0//flags
        );

        //TODO: Don't have separate deviceMemory/deviceVertexBuffer, if srcMemory is device local
        VulkanBuffer deviceUniformBuffer(
            device,
            VkBufferCreateInfo{
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,//sType
                nullptr,//pNext
                0,//flags
                size,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,//usage
                VK_SHARING_MODE_EXCLUSIVE,//sharingMode
                0,//queueFamilyIndexCount
                nullptr//pQueueFamilyIndices
            }
        );
        VkMemoryRequirements deviceUniformBufferMemRequirements;
        vkGetBufferMemoryRequirements(device, deviceUniformBuffer.buffer, &deviceUniformBufferMemRequirements);
        VulkanMemory deviceMemory(
            device,
            VkMemoryAllocateInfo{
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,//sType
                nullptr,//pNext
                deviceUniformBufferMemRequirements.size,
                findMemoryType(physicalDevice, deviceUniformBufferMemRequirements.size, deviceUniformBufferMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            }
        );

        if (vkBindBufferMemory(device, deviceUniformBuffer.buffer, deviceMemory.memory, 0/*memoryOffset*/) != VK_SUCCESS) {
            throw std::exception("vkBindBufferMemory failed");
        }

        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = gsl::narrow<uint32_t>(size/sizeof(UniformBufferObject));

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = gsl::narrow<uint32_t>(size / sizeof(UniformBufferObject));
        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.reserve(size / sizeof(UniformBufferObject));
        uniformBuffers.push_back(
            SizedStagedUniformBuffer{
                size,
                std::move(transferSrcUniformBuffer),
                std::move(transferSrcMemory),
                std::move(transferSrcMappedRegion),
                std::move(deviceUniformBuffer),
                std::move(deviceMemory),
                VulkanDescriptorPool(device, poolInfo),
                std::move(descriptorSets)
            }
        );
        enqueueUniformBufferCopy();
    }

    void flushBuffersToDevice() {
        //Once writes to buffer are complete, signal the fence so that
        //the data is written to the device, and the command queue can continue
        //Not necessary, since the command buffer isnt even submitted to the queue
        //at this point?
        //vkSetEvent


        //flush mapped memory, if VK_MEMORY_PROPERTY_HOST_COHERENT_BIT isn't set.
        std::vector<VkMappedMemoryRange> memoryRanges;
        boost::push_back(memoryRanges,
            vertexBuffers | boost::adaptors::transformed([](auto const &vb){
                return VkMappedMemoryRange{
                    VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                    nullptr, //pNext
                    vb.transferSrcMemory.memory,
                    0,//offset
                    VK_WHOLE_SIZE//size
                };
            })
        );

        boost::push_back(memoryRanges,
            uniformBuffers | boost::adaptors::transformed([](auto const &ub) {
            return VkMappedMemoryRange{
                VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                nullptr, //pNext
                ub.transferSrcMemory.memory,
                0,//offset
                VK_WHOLE_SIZE//size
            };
        })
        );

        if (vkFlushMappedMemoryRanges(device, gsl::narrow<uint32_t>(memoryRanges.size()), memoryRanges.data()) != VK_SUCCESS) {
            throw new std::exception("vkFlushMappedMemoryRanges failed");
        }

        std::array memoryBarriers{
            VkMemoryBarrier{
                VK_STRUCTURE_TYPE_MEMORY_BARRIER,//sType
                nullptr,//pNext
                VK_ACCESS_TRANSFER_WRITE_BIT,//srcAccessMask
                VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT//dstAccessMask
            }
        };

        vkCmdPipelineBarrier(
            preDrawCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,      // srcStageMask
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,  // dstStageMask
            0,                                   // dependencyFlags
            gsl::narrow<uint32_t>(memoryBarriers.size()),  // memoryBarrierCount
            memoryBarriers.data(),               // pMemoryBarriers
            0,                                   // bufferMemoryBarrierCount
            nullptr,                             // pBufferMemoryBarriers
            0,                                   // imageMemoryBarrierCount
            nullptr                              // pImageMemoryBarriers
        );
    }

    void drawVertices(std::vector<Vertex> vertices/*Texture binding, list adjacency format, index buffer*/)
    {
        auto const growthFactor{2};
        
        auto const requiredSizeForVertices{sizeof(vertices[0])*vertices.size()};
        if (vertices.size() == 0) return;
        //find appropriate spot at end of transferSrcVertexBuffers
        //if not enough space, allocate another src/dst buffer

        //TODO: Use leftover space in earlier buffers, if a large allocation requires a new buffer,
        // but there is still space in the earlier buffers for smaller allocations.
        if (vertexBuffers.empty() || (vertexBuffers.back().size-endPositionInCurrentVertexBuffer) < requiredSizeForVertices) {
            prepareBufferAndTransfer(std::max((vertexBuffers.empty() ? initialVertexBufSize : vertexBuffers.back().size*growthFactor), requiredSizeForVertices));
        }
        //write to spot in transferSrcVertexBuffers
        //enqueue vertex draw commands (buffer bind, texture bind, etc)


        memcpy(static_cast<char *>(vertexBuffers.back().transferSrcMappedRegion.mappedMemory)+endPositionInCurrentVertexBuffer, vertices.data(), sizeof(vertices[0]) * vertices.size());
        //renderOps.emplace_back(
        //    [endPos=endPositionInCurrentVertexBuffer, cmdBuf=commandBuffer, vertSize=vertices.size()]{
                vkCmdDraw(
                    drawCommandBuffer,
                    gsl::narrow<uint32_t>(vertices.size()),
                    1,//instanceCount
                    gsl::narrow<uint32_t>(endPositionInCurrentVertexBuffer /sizeof(vertices[0])),//firstVertex
                    0//firstInstance
                );
        //    }
        //);


        endPositionInCurrentVertexBuffer += requiredSizeForVertices;
    }
    void updateUniformBuffer(UniformBufferObject const &newUbo) {
        //TODO

        //prepareUniformBufferAndTransfer
        auto const growthFactor{ 2 };

        auto const requiredSizeForUniformBuffer{sizeof(newUbo)};

        //find appropriate spot at end of transferSrcVertexBuffers
        //if not enough space, allocate another src/dst buffer
        //TODO: Use leftover space in earlier buffers, if a large allocation requires a new buffer,
        // but there is still space in the earlier buffers for smaller allocations.
        if (uniformBuffers.empty() || uniformBuffers.back().size < (uniformBuffers.back().descriptorSets.size() * sizeof(UniformBufferObject))+requiredSizeForUniformBuffer) {
            prepareUniformBufferAndTransfer(std::max((uniformBuffers.empty() ? initialUniformBufSize : uniformBuffers.back().size*growthFactor), requiredSizeForUniformBuffer));
        }
        //memcpy
        memcpy(static_cast<char *>(
            uniformBuffers.back().transferSrcMappedRegion.mappedMemory)
          + (uniformBuffers.back().descriptorSets.size() * sizeof(UniformBufferObject)),
            &newUbo,
            sizeof(newUbo));

        //vkAllocateDescriptorSets
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = uniformBuffers.back().descriptorPool.descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        VkDescriptorSet descriptorSet;
        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        //vkUpdateDescriptorSets
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers.back().deviceUniformBuffer.buffer;
        bufferInfo.offset = uniformBuffers.back().descriptorSets.size() * sizeof(UniformBufferObject);
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);



        uniformBuffers.back().descriptorSets.push_back(descriptorSet);
        //vkCmdBindDescriptorSets

        vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    }
    void newFrame() {
        //If vertexBuffers.size() > 1, reallocate as single contiguous buffer
        if (vertexBuffers.size() != 1) {
            auto const fullSize{std::max(std::size_t{initialVertexBufSize}, boost::accumulate(vertexBuffers | boost::adaptors::transformed([](auto const &vb){return vb.size;}), std::size_t{0}))};
            vertexBuffers.clear();
            prepareBufferAndTransfer(fullSize);
        }
        else {
            enqueueBufferCopy();
        }

        //If uniformBuffers.size() != 1, reallocate as single contiguous buffer
        if (uniformBuffers.size() != 1) {
            auto const fullSize{ std::max(std::size_t{initialUniformBufSize}, boost::accumulate(uniformBuffers | boost::adaptors::transformed([](auto const &buf) {return buf.size; }), std::size_t{0})) };
            uniformBuffers.clear();
            prepareUniformBufferAndTransfer(fullSize);
        }
        else {
            uniformBuffers.back().descriptorSets.clear();
            if (vkResetDescriptorPool(device, uniformBuffers.back().descriptorPool.descriptorPool, 0) != VK_SUCCESS){
                throw std::exception("vkResetDescriptorPool failed");
            }
            enqueueUniformBufferCopy();
        }
    }

    struct SizedStagedVertexBuffer {
        std::size_t size;
        VulkanBuffer transferSrcVertexBuffer;
        VulkanMemory transferSrcMemory;
        VulkanMappedMemory transferSrcMappedRegion;
        VulkanBuffer deviceVertexBuffer;
        VulkanMemory deviceMemory;
    };

    struct SizedStagedUniformBuffer {
        std::size_t size;
        VulkanBuffer transferSrcUniformBuffer;
        VulkanMemory transferSrcMemory;
        VulkanMappedMemory transferSrcMappedRegion;
        VulkanBuffer deviceUniformBuffer;
        VulkanMemory deviceMemory;
        VulkanDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkCommandBuffer preDrawCommandBuffer;
    VkCommandBuffer drawCommandBuffer;

    std::size_t endPositionInCurrentVertexBuffer;
    std::vector<SizedStagedVertexBuffer> vertexBuffers;
    //TODO: Share memory between this and the vertexBuffers,
    //if possible.
    std::vector <SizedStagedUniformBuffer> uniformBuffers;

    //std::vector<move_function<void()>> renderOps;
};

}
#endif //!HG_VULKAN_RENDER_TARGET_H