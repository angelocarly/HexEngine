//
// Created by magnias on 31/03/2021.
//

#ifndef VULKANENGINE_RENDER_PIPELINE_H
#define VULKANENGINE_RENDER_PIPELINE_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#include "../../vks/vks_pipeline.h"
#include "../../vks/vks_swap_chain.h"
#include "../camera.h"

using namespace vks;

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

template <typename T>
class IRenderPipeline
{
public:
virtual ~IRenderPipeline()
{
};

virtual void bind(VkCommandBuffer& commandBuffer) = 0;
virtual void execute() = 0;
};


#endif //VULKANENGINE_RENDER_PIPELINE_H
