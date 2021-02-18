/*
*
* LveModel Class
*
* Little vulkan engine model class
*
* Copyright (C) 2020 by Blurrypiano - https://github.com/blurrypiano/littleVulkanEngine
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#ifndef VULKANENGINE_VKS_MODEL_H
#define VULKANENGINE_VKS_MODEL_H


#pragma once

#include "vks_device.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // open gl uses -1 to 1, vk is 0 to 1

#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace vks
{

    class VksModel
    {
    public:
        enum class VertexAttribute
        {
            POSITION, NORMAL, COLOR, UV, TANGENT
        };

        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec4 color;
            glm::vec2 uv;
            glm::vec4 tangent;  // w component is -1 or 1 and indicates handedness of the tangent basis

            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }

            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && uv == other.uv &&
                       normal == other.normal && tangent == other.tangent;
            }
        };

        class Builder
        {
        public:
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(std::string filepath);
        };

        VksModel(VksDevice &device, Builder &builder);

        ~VksModel()
        { cleanup(); }

        VksModel(const VksModel &) = delete;

        void operator=(const VksModel &) = delete;

        void draw(VkCommandBuffer commandBuffer);

        void bind(VkCommandBuffer commandBuffer);

        void swapChainReset()
        {};

        static std::unique_ptr<VksModel> loadModelFromFile(VksDevice &device, std::string filepath);

    private:
        VksDevice &device_;

        VkBuffer vertexBuffer_;
        VkDeviceMemory vertexBufferMemory_;
        uint32_t vertexCount_;

        VkBuffer indexBuffer_;
        VkDeviceMemory indexBufferMemory_;
        uint32_t indexCount_;

        void createVertexBuffer(Builder &builder);

        void createIndexBuffer(Builder &builder);

        void cleanup();
    };
}  // namespace lve


#endif //VULKANENGINE_VKS_MODEL_H
