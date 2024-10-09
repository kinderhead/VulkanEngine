#include "Window.hpp"



Window::Window(string title, int width, int height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    renderer = make_unique<Renderer>(title, window);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        update();
        render();
    }
}
