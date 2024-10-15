#include "TestWindow.hpp"



void TestWindow::update()
{
}

void TestWindow::render()
{
    BasicUBO ubo{mat4(), lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 0, 1)), ortho(-2, 2, -2, 2)};
    ubo.proj[1][1] *= -1;

    renderer->basicPipeline->setUBO(&ubo);
}
