#pragma once

#include "Window.hpp"

class TestWindow : public Window
{
    using Window::Window;
    
protected:
    virtual void update() override;
    virtual void render() override;
};
