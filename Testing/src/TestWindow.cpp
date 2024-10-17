#include "TestWindow.hpp"



void TestWindow::update()
{
}

void TestWindow::render()
{
    renderer->drawRectangle(100, 100, 150, 100, numbers::pi / 4, vec4(1, 1, 0, 1));
    renderer->drawRectangle(300, 300, 50, 50);
    renderer->drawElipse(0, 100, 50, 50);
}
