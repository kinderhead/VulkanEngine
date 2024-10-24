#pragma once

#include "utils.hpp"
#include "Renderer.hpp"

class BaseModel
{
protected:
    Renderer* renderer;

public:
    inline BaseModel() {}

    virtual void bind(vki::CommandBuffer& cmds) = 0;
    virtual void draw(vki::CommandBuffer& cmds) = 0;
};
