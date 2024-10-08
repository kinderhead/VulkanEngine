#pragma once

#include "utils.hpp"

class Window
{
    GLFWwindow* window;
public:
    Window(string title, int width, int height);
    ~Window();

    void run();

protected:
    virtual void update() = 0;
    virtual void render() = 0;
};
