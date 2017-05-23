//
//  Buffer.cpp
//  Vortex2D
//

#include "Buffer.h"

namespace Vortex2D { namespace Renderer {

Buffer::Buffer(const Device& device,
       vk::BufferUsageFlags usageFlags,
       vk::MemoryPropertyFlags memoryFlags,
       vk::DeviceSize deviceSize)
    : mDevice(device.Handle())
    , mSize(deviceSize)
    , mAccess({}) // TODO should this be the initial access?
{
    auto bufferInfo = vk::BufferCreateInfo()
            .setSize(deviceSize)
            .setUsage(usageFlags)
            .setSharingMode(vk::SharingMode::eExclusive);

    mBuffer = device.Handle().createBufferUnique(bufferInfo);

    auto memoryRequirements = device.Handle().getBufferMemoryRequirements(*mBuffer);

    uint32_t memoryPropertyIndex =
            device.FindMemoryPropertiesIndex(memoryRequirements.memoryTypeBits, memoryFlags);

    auto memoryInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memoryPropertyIndex);

    mMemory = device.Handle().allocateMemoryUnique(memoryInfo);
    device.Handle().bindBufferMemory(*mBuffer, *mMemory, 0);
}

void Buffer::Flush()
{
    auto mappedRange = vk::MappedMemoryRange()
            .setMemory(*mMemory)
            .setSize(mSize);

    mDevice.flushMappedMemoryRanges({mappedRange});
}

Buffer::operator vk::Buffer() const
{
    return *mBuffer;
}

vk::DeviceSize Buffer::Size() const
{
    return mSize;
}

void Buffer::CopyFrom(vk::CommandBuffer commandBuffer, Buffer& srcBuffer)
{
    if (mSize != srcBuffer.mSize)
    {
        return;
    }

    srcBuffer.Barrier(commandBuffer, vk::AccessFlagBits::eTransferRead);
    Barrier(commandBuffer, vk::AccessFlagBits::eTransferWrite);

    auto region = vk::BufferCopy()
            .setSize(mSize);

    commandBuffer.copyBuffer(srcBuffer, *mBuffer, region);
}

void Buffer::Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags newAccess)
{
    if (newAccess == mAccess)
    {
        return;
    }

    vk::AccessFlags oldAccess = mAccess;
    mAccess = newAccess;

    auto bufferMemoryBarriers = vk::BufferMemoryBarrier()
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setBuffer(*mBuffer)
            .setSize(mSize)
            .setSrcAccessMask(oldAccess)
            .setDstAccessMask(newAccess);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                                  vk::PipelineStageFlagBits::eAllCommands,
                                  {},
                                  nullptr,
                                  bufferMemoryBarriers,
                                  nullptr);
}

}}
