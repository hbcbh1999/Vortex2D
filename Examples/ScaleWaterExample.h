//
//  ScaleWaterExample.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

class ScaleWaterExample : public Vortex2D::Renderer::Drawable
{
public:
    ScaleWaterExample(const glm::vec2& size, float dt)
    : gravity(glm::vec2(500))
    , obstacle(glm::vec2(100))
    , dimensions(size, 2.0f)
    , world(dimensions, dt)
    {
        gravity.Colour = {0.0f, -0.5f, 0.0f, 0.0f};

        obstacle.Position = {200, 50};
        obstacle.Colour = green;

        world.Colour = blue;

        auto boundaries = world.DrawBoundaries();

        Vortex2D::Renderer::Rectangle source({300,100});
        source.Position = {100,300};
        source.Colour = glm::vec4(1.0f);

        boundaries.DrawLiquid(source);

        Vortex2D::Renderer::Rectangle area(size - glm::vec2(2.0f));
        area.Position = glm::vec2(1.0f);
        area.Colour = glm::vec4(1.0f);

        boundaries.DrawSolid(area, true);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override
    {
        world.RenderForce(gravity);

        world.Solve();
        world.Advect();

        target.Render(world, transform);
        target.Render(obstacle, transform);
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Renderer::Rectangle obstacle;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Fluid::World world;

};
