//
//  PrefixScan.cpp
//  Vortex2D
//

#include "PrefixScan.h"

namespace Vortex2D { namespace Fluid {

namespace
{
    int GetWorkGroupSize(int n, int localSize)
    {
        return (n + (localSize * 2 - 1)) / (localSize * 2);
    }

    Renderer::ComputeSize MakeComputeSize(int size)
    {
        Renderer::ComputeSize computeSize(Renderer::ComputeSize::Default1D());

        auto localSize = Renderer::ComputeSize::GetLocalSize1D();
        computeSize.DomainSize = glm::ivec2(size, 1);
        computeSize.LocalSize = glm::ivec2(localSize, 1);
        computeSize.WorkSize = glm::ceil(glm::vec2(size, 1.0f) / (glm::vec2(localSize, 1.0f) * glm::vec2(2.0f, 1.0f)));

        return computeSize;
    }
}

PrefixScan::PrefixScan(const Renderer::Device& device, const glm::ivec2& size)
    : mSize(size.x*size.y)
    , mAddWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/PreScanAdd.comp.spv",
      {vk::DescriptorType::eStorageBuffer,
       vk::DescriptorType::eStorageBuffer})
    , mPreScanWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/PreScan.comp.spv",
      {vk::DescriptorType::eStorageBuffer,
       vk::DescriptorType::eStorageBuffer})
    , mPreScanStoreSumWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/PreScanStoreSum.comp.spv",
     {vk::DescriptorType::eStorageBuffer,
      vk::DescriptorType::eStorageBuffer,
      vk::DescriptorType::eStorageBuffer})
{

    auto localSize = Renderer::ComputeSize::GetLocalSize1D();
    int workGroupSize = mSize;

    while ((workGroupSize = GetWorkGroupSize(workGroupSize, localSize)) > 1)
    {
        mPartialSums.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float) * workGroupSize);
    }

    assert(workGroupSize);
}

void PrefixScan::BindRecursive(std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                               std::vector<Renderer::Work::Bound>& bound,
                               Renderer::Buffer& input,
                               Renderer::Buffer& output,
                               Renderer::ComputeSize computeSize,
                               int level)
{
    if (computeSize.WorkSize.x > 1)
    {
        assert(level < mPartialSums.size());
        auto& partialSums = mPartialSums[level];

        bound.emplace_back(mPreScanStoreSumWork.Bind(computeSize, {input, output, partialSums}));
        bufferBarriers.emplace_back([&](vk::CommandBuffer commandBuffer)
        {
            output.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            partialSums.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        });

        BindRecursive(bufferBarriers,
                      bound,
                      partialSums,
                      partialSums,
                      MakeComputeSize(computeSize.WorkSize.x),
                      level + 1);

        bound.emplace_back(mAddWork.Bind(computeSize, {partialSums, output}));
        bufferBarriers.emplace_back([&](vk::CommandBuffer commandBuffer)
        {
            output.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        });
    }
    else
    {
        bound.emplace_back(mPreScanWork.Bind(computeSize, {input, output}));
        bufferBarriers.emplace_back([&](vk::CommandBuffer commandBuffer)
        {
            output.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        });
    }
}

PrefixScan::Bound PrefixScan::Bind(Renderer::Buffer& input, Renderer::Buffer& output)
{
    std::vector<Renderer::CommandBuffer::CommandFn> bufferBarriers;
    std::vector<Renderer::Work::Bound> bounds;

    BindRecursive(bufferBarriers,
                  bounds,
                  input,
                  output,
                  MakeComputeSize(mSize),
                  0);

    return Bound(bufferBarriers, std::move(bounds));
}

PrefixScan::Bound::Bound(const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                         std::vector<Renderer::Work::Bound>&& bounds)
    : mBufferBarriers(bufferBarriers)
    , mBounds(std::move(bounds))
{
}

void PrefixScan::Bound::Record(vk::CommandBuffer commandBuffer)
{
    for (int i = 0; i < mBounds.size(); i++)
    {
        mBounds[i].Record(commandBuffer);
        mBufferBarriers[i](commandBuffer);
    }
}

}}