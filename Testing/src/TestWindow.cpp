#include "TestWindow.hpp"



void TestWindow::update()
{
}

void TestWindow::render()
{
    // renderer->drawRectangle(100, 100, 150, 100, numbers::pi / 4, vec4(1, 1, 0, 1));
    // renderer->drawRectangle(280, 280, 25, 25, 0, {0, 0, 1, 1});
    // renderer->drawRectangle(300, 300, 50, 50);
    // renderer->drawElipse(0, 100, 50, 50);

    renderer->drawPolygon({ {10, 20}, {30, 40}, {50, 60}, {70, 80}, {90, 100} });
}
