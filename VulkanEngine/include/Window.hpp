#pragma once

#include "utils.hpp"
#include "Renderer.hpp"

class Window
{
    GLFWwindow* window;

    unique_ptr<Renderer> renderer;
public:
    Window(string title, int width, int height);
    ~Window();

    void run();

protected:
    virtual void update() = 0;
    virtual void render() = 0;
};
