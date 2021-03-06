//
//  ShapeDrawer.h
//  Vortex2D
//

#include <vector>
#include <glm/vec2.hpp>

void DrawEllipse(int width,
                 int height,
                 std::vector<float>& data,
                 const glm::vec2& centre,
                 const glm::vec2& radius,
                 float rotation = 0.0f);

void DrawCircle(int width,
                int height,
                std::vector<float>& data,
                const glm::vec2& centre,
                float radius);

template<typename T>
void DrawSquare(int width, int /*height*/, std::vector<T>& data, const glm::vec2& centre, const glm::vec2& size, T value)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int x = static_cast<int>(i + centre.x);
            int y = static_cast<int>(j + centre.y);
            data[x + y * width] = value;
        }
    }
}

