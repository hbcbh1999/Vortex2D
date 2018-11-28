//
//  Buffer.h
//  Vortex2D
//

#ifndef Vortex2D_Buffer_h
#define Vortex2D_Buffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Utils/vk_mem_alloc.h>

namespace Vortex2D { namespace Renderer {

class Texture;
class Device;

/**
 * @brief A vulkan buffer which can be on the host or the device.
 */
class GenericBuffer
{
public:
    VORTEX2D_API GenericBuffer(const Device& device,
                              vk::BufferUsageFlags usageFlags,
                              VmaMemoryUsage memoryUsage,
                              vk::DeviceSize deviceSize,
                              const char* name = nullptr);

    VORTEX2D_API virtual ~GenericBuffer();

    VORTEX2D_API GenericBuffer(GenericBuffer&& other);

    /**
     * @brief Copy a buffer to this buffer
     * @param commandBuffer command buffer to run the copy on.
     * @param srcBuffer the source buffer.
     */
    VORTEX2D_API void CopyFrom(vk::CommandBuffer commandBuffer, GenericBuffer& srcBuffer);

    /**
     * @brief Copy a texture to this buffer
     * @param commandBuffer command buffer to run the copy on.
     * @param srcTexture the source texture
     */
    VORTEX2D_API void CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcTexture);

    /**
     * @brief The vulkan handle
     */
    VORTEX2D_API vk::Buffer Handle() const;

    /**
     * @brief The size in bytes of the buffer
     */
    VORTEX2D_API vk::DeviceSize Size() const;

    /**
     * @brief Inserts a barrier for this buffer
     * @param commandBuffer the command buffer to run the barrier
     * @param oldAccess old access
     * @param newAccess new access
     */
    VORTEX2D_API void Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess);

    /**
     * @brief Clear the buffer with 0
     * @param commandBuffer the command buffer to clear on
     */
    VORTEX2D_API void Clear(vk::CommandBuffer commandBuffer);

    // Template friend functions for copying to and from buffers
    template<template<typename> class BufferType, typename T>
    friend void CopyFrom(BufferType<T>&, const T&);
    template<template<typename> class BufferType, typename T>
    friend void CopyTo(BufferType<T>&, T&);

    template<template<typename> class BufferType, typename T>
    friend void CopyTo(BufferType<T>&, std::vector<T>&);
    template<template<typename> class BufferType, typename T>
    friend void CopyFrom(BufferType<T>&, const std::vector<T>&);

protected:
    VORTEX2D_API void CopyFrom(const void* data);
    VORTEX2D_API void CopyTo(void* data);

    const Device& mDevice;
    vk::DeviceSize mSize;
    VkBuffer mBuffer;
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
};

/**
 * @brief a vertex buffer type of buffer
 */
template<typename T>
class VertexBuffer : public GenericBuffer
{
public:
    VertexBuffer(const Device& device,
                 std::size_t size,
                 VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                 const char* name = nullptr)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, memoryUsage, sizeof(T) * size, name)
    {
    }
};

/**
 * @brief a uniform buffer type of buffer
 */
template<typename T>
class UniformBuffer : public GenericBuffer
{
public:
    UniformBuffer(const Device& device,
                  VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                  const char* name = nullptr)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, memoryUsage, sizeof(T), name)
    {
    }
};

/**
 * @brief a storage buffer type of buffer
 */
template<typename T>
class Buffer : public GenericBuffer
{
public:
    Buffer(const Device& device,
           std::size_t size = 1,
           VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
           const char* name = nullptr)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, memoryUsage, sizeof(T) * size, name)
    {
    }
};

/**
 * @brief an indirect buffer type of buffer, used for compute indirect dispatch
 */
template<typename T>
class IndirectBuffer : public GenericBuffer
{
public:
    IndirectBuffer(const Device& device,
                   VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
                   const char* name = nullptr)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer, memoryUsage, sizeof(T), name)
    {
    }
};

/**
 * @brief Copy the content of a buffer in an object
 */
template<template<typename> class BufferType, typename T>
void CopyTo(BufferType<T>& buffer, T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyTo(&t);
}

/**
 * @brief Copy the content of a buffer to a vector. Vector needs to have the correct size already.
 */
template<template<typename> class BufferType, typename T>
void CopyTo(BufferType<T>& buffer, std::vector<T>& t)
{
    if (sizeof(T) * t.size() != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyTo(t.data());
}

/**
 * @brief Copy the content of an object to the buffer.
 */
template<template<typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyFrom(&t);
}

/**
 * @brief Copy the content of a vector to the buffer
 */
template<template<typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const std::vector<T>& t)
{
    if (sizeof(T) * t.size() != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyFrom(t.data());
}

/**
 * @brief Inserts a barrier for the given buffer, command buffer and access.
 * @param buffer the vulkan buffer handle
 * @param commandBuffer the command buffer to inserts the barrier
 * @param oldAccess old access
 * @param newAccess new access
 */
void BufferBarrier(vk::Buffer buffer, vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess);

}}

#endif
