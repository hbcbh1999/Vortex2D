//
//  ExtrapolateTests.cpp
//  Vortex2D
//

#include "Verify.h"
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/Extrapolation.h>

using namespace Vortex2D::Fluid;
using namespace Vortex2D::Renderer;

extern Device* device;

void PrintValid(const glm::ivec2& size, Vortex2D::Renderer::Buffer& buffer)
{
    std::vector<glm::ivec2> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (int j = 0; j < size.x; j++)
    {
        for (int i = 0; i < size.y; i++)
        {
            glm::ivec2 value = pixels[i + j * size.x];
            std::cout << "(" << value.x << "," << value.y << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void SetValid(const glm::ivec2& size, FluidSim& sim, Buffer& buffer)
{
    std::vector<glm::ivec2> validData(size.x*size.y);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + j * size.x;
            validData[index].x = sim.u_valid(i, j);
            validData[index].y = sim.v_valid(i, j);
        }
    }

    buffer.CopyFrom(validData);
}

TEST(ExtrapolateTest, Extrapolate)
{
    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));
    SetValid(size, sim, valid);

    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    SetVelocity(*device, size, velocity, sim);

    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);

    extrapolate(sim.u, sim.u_valid);
    extrapolate(sim.v, sim.v_valid);

    Extrapolation extrapolation(*device, size, valid, velocity, solidVelocity, solidPhi);
    extrapolation.Extrapolate();

    device->Queue().waitIdle();

    CheckVelocity(*device, size, velocity, sim);
    CheckValid(size, sim, valid);
}

TEST(ExtrapolateTest, Constrain)
{
    // FIXME Cannot use higher size because of weird float conversions in FluidSim
    glm::vec2 size(20);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    // FIXME should set the scale to size.x
    SetSolidPhi(*device, size, solidPhi, sim);

    extrapolate(sim.u, sim.u_valid);
    extrapolate(sim.v, sim.v_valid);

    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    SetVelocity(*device, size, velocity, sim);

    sim.constrain_velocity();

    Extrapolation extrapolation(*device, size, valid, velocity, solidVelocity, solidPhi);
    extrapolation.ConstrainVelocity();

    device->Queue().waitIdle();

    CheckVelocity(*device, size, velocity, sim, 1e-5f);
}
