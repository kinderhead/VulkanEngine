#pragma once

#include "utils.hpp"

struct VertexDefinition
{
    vk::VertexInputBindingDescription binding;
    vector<vk::VertexInputAttributeDescription> attributes;
};

struct BasicVertex
{
    vec2 pos;
    //vec3 color;

    static VertexDefinition getVertexDefinition()
    {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(BasicVertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};
        attributeDescriptions.resize(1);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset = offsetof(BasicVertex, pos);

        // attributeDescriptions[1].binding = 0;
        // attributeDescriptions[1].location = 1;
        // attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        // attributeDescriptions[1].offset = offsetof(BasicVertex, color);

        return { bindingDescription, attributeDescriptions };
    }
};

struct BasicUBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 color;
};

