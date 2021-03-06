#include <gtest/gtest.h>
#include <Vortex2D/Renderer/Instance.h>
#include <Vortex2D/Renderer/Device.h>

Vortex2D::Renderer::Device* device;

TEST(Vulkan, Init)
{
    auto physicalDevice = device->GetPhysicalDevice();
    auto properties = physicalDevice.getProperties();
    std::cout << "Device name: " << properties.deviceName << std::endl;

    for (int i = 0; i < 3; i++)
    {
        std::cout << "Max local size " << i << " : " << properties.limits.maxComputeWorkGroupSize[i] << std::endl;
    }
    std::cout << "Max local size total: " << properties.limits.maxComputeWorkGroupInvocations << std::endl;
}

int main(int argc, char **argv)
{
    bool debug;

#ifdef NDEBUG
    debug = false;
#else
    debug = true;
#endif

    std::vector<const char*> extensions;
    Vortex2D::Renderer::Instance instance("Tests", extensions, debug);
    Vortex2D::Renderer::Device device_(instance.GetPhysicalDevice());

    device = &device_;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
