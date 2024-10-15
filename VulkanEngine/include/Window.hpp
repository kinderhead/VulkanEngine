#pragma once

#include "utils.hpp"
#include "Renderer.hpp"

class Window
{
    GLFWwindow* window;

public:
    unique_ptr<Renderer> renderer;

    Window(string title, int width, int height);
    ~Window();

    void run();

protected:
    virtual void update() = 0;
    virtual void render() = 0;
};
