#include "TestWindow.hpp"



void TestWindow::update()
{
}

void TestWindow::render()
{
    BasicUBO ubo{translate(mat4(1), vec3(50, 50, 0)) * scale(mat4(1), vec3(50)), lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)), ortho(0.0f, (float) renderer->swapChain->extent.width, 0.0f, (float) renderer->swapChain->extent.height, -1000.0f, 1000.0f)};
    
    renderer->basicPipeline->setUBO(&ubo);
}
