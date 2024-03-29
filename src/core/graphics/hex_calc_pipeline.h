//
// Created by magnias on 04/04/2021.
//

#ifndef _HEX_CALC_PIPELINE_H_
#define _HEX_CALC_PIPELINE_H_

#include <vks/vks_util.h>
#include "render_pipeline.h"
#include <lodepng.h>
#include <vks/VulkanInitializers.h>

class HexCalcPipeline : public IRenderPipeline
{
public:

    struct hex_node
    {
        glm::vec3 color;
    };

    struct hex_operation
    {
        int operation;
    };

    HexCalcPipeline(VksDevice &device, VksSwapChain &swapChain, VkDescriptorPool &descriptorPool)
            : _device(device), _descriptorPool(descriptorPool), _swapChain(swapChain)
    {
        init();
    }

    void init()
    {

        _computeQueue = _device.getComputeQueue();

        createDescriptorSetLayout();
        createPipelineLayout();
        createPipeline();
        createHexBuffer();
        createHexOperationBuffer();
        createDescriptorSets();

        starttime =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                );
    }

    void begin(VkCommandBuffer &commandBuffer, int frame) override
    {
        _currentCommandBuffer = commandBuffer;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &_descriptorSet, 0,
                                NULL);

    };

    void end() override
    {
    }

    void updateBuffers(Camera camera)
    {}

    VkDescriptorBufferInfo getHexBuffer()
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _hexBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = _hexBufferSize;
        return bufferInfo;
    }

    VkDescriptorBufferInfo getHexOperationsBuffer()
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _hexOperationBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = _hexOperationBufferSize;
        return bufferInfo;
    }

    void execute()
    {
        HexPushConstants constants;
        std::chrono::milliseconds time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                );
        constants.time = std::chrono::duration<float, std::milli>(time - starttime).count() / 1000.0f;
        vkCmdPushConstants(_currentCommandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                           sizeof(HexPushConstants), &constants);

        vkCmdDispatch(_currentCommandBuffer, (uint32_t) ceil(WIDTH / float(WORKGROUP_SIZE * CLUSTER_SIZE)), (uint32_t) ceil(
                HEIGHT / float(WORKGROUP_SIZE * CLUSTER_SIZE)), 1);
    }

    void setBuffers(VkDescriptorBufferInfo hexBuffer, VkDescriptorBufferInfo hexOperationsBuffer)
    {
        updateDescriptorSets(hexBuffer, hexOperationsBuffer);
    }

private:

    struct HexPushConstants
    {
        float time;
    };

    const int WORKGROUP_SIZE = 16;
    const int CLUSTER_SIZE = 2;
    const int WIDTH = 779;
    const int HEIGHT = 900;

    VksDevice &_device;
    VkDescriptorPool &_descriptorPool;
    VksSwapChain &_swapChain;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet _descriptorSet;
    VkPipeline pipeline;
    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    VkBuffer _hexBuffer;
    VkDeviceMemory _hexBufferMemory;
    VkDeviceSize _hexBufferSize;
    VkBuffer _hexOperationBuffer;
    VkDeviceMemory _hexOperationBufferMemory;
    VkDeviceSize _hexOperationBufferSize;

    VkCommandBuffer _currentCommandBuffer;
    uint32_t _currentFrame;

    VkQueue _computeQueue;

    std::chrono::milliseconds  starttime;

    /**
     * Create a pipeline layout
     * Stores the set of resources that can be used by the pipeline
     */
    void createPipelineLayout()
    {

        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = sizeof(HexPushConstants);
        push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        if (vkCreatePipelineLayout(_device.getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }

    /**
     * Stores the data that will be accessible to the descriptor set
     */
    void createDescriptorSetLayout()
    {

        VkDescriptorSetLayoutBinding binding[2];

        binding[0].binding = 0;
        binding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding[0].descriptorCount = 1;
        binding[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        binding[1].binding = 1;
        binding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding[1].descriptorCount = 1;
        binding[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 2;
        layoutInfo.pBindings = binding;

        if (vkCreateDescriptorSetLayout(_device.getVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(_swapChain.getImageCount(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        if (vkAllocateDescriptorSets(_device.getVkDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        updateDescriptorSets(getHexBuffer(), getHexOperationsBuffer());
    }

    void updateDescriptorSets(VkDescriptorBufferInfo hexBuffer, VkDescriptorBufferInfo hexOperationsBuffer)
    {
        VkDescriptorBufferInfo hexBufferInfo{};
        hexBufferInfo.buffer = _hexBuffer;
        hexBufferInfo.offset = 0;
        hexBufferInfo.range = _hexBufferSize;

        VkDescriptorBufferInfo hexOperationBufferInfo{};
        hexOperationBufferInfo.buffer = _hexOperationBuffer;
        hexOperationBufferInfo.offset = 0;
        hexOperationBufferInfo.range = _hexOperationBufferSize;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &hexBufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &hexOperationBufferInfo;

        vkUpdateDescriptorSets(_device.getVkDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }

    void createPipeline()
    {
//		PipelineConfigInfo pipelineConfigInfo{};
//		VksPipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
//		pipelineConfigInfo.renderPass = _swapChain.getRenderPass();
//		pipelineConfigInfo.pipelineLayout = pipelineLayout;
//		pipeline = std::make_unique<VksPipeline>(_device, _swapChain, pipelineConfigInfo);

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = _device.createShaderModule(VksUtil::readFile("shaders/hex_calc.comp.spv"));
        shaderStageCreateInfo.pName = "main";

        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = sizeof(HexPushConstants);
        push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        /*
        The pipeline layout allows the pipeline to access descriptor sets.
        So we just specify the descriptor set layout we created earlier.
        */
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &push_constant;
        if (vkCreatePipelineLayout(_device.getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout)
            != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline layout");
        }

        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage = shaderStageCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;

        /*
        Now, we finally create the compute pipeline.
        */
        if (vkCreateComputePipelines(
                _device.getVkDevice(), VK_NULL_HANDLE,
                1, &pipelineCreateInfo,
                NULL, &pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline");
        }
    }

    void createHexBuffer()
    {

        std::vector<hex_node> nodes;

        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
            if ( rand() % 80 == 0) {
//            if (true) {
            nodes.push_back({
                                    glm::vec3(
                                            (float) static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 10.0f)),
0.0f,
0.0f
//                                            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 10.0f)),
//                                            static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 10.0f))
                                    )
                            });
            } else {
                nodes.push_back({glm::vec3(0.0f)});
            }
        }

        _hexBufferSize = sizeof(hex_node) * nodes.size();
        _device.
                createBuffer(
                _hexBufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                // Allow the host to write to the memory and keep both values consistent with each other
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                _hexBuffer,
                _hexBufferMemory);

        // Temporarily map the region of the host memory to the device memory and upload the marchdata
        void *data;
        vkMapMemory(_device
                            .
                                    getVkDevice(), _hexBufferMemory,
                    0, _hexBufferSize, 0, &data);
        memcpy(data, nodes.data(),
               static_cast
                       <size_t>(_hexBufferSize)
        );
        vkUnmapMemory(_device
                              .
                                      getVkDevice(), _hexBufferMemory
        );
    }

    void createHexOperationBuffer()
    {

        std::vector<hex_operation> operations;

        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
            operations.push_back({0});
        }

        _hexOperationBufferSize = sizeof(hex_operation) * operations.size();
        _device.
                createBuffer(
                _hexOperationBufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                // Allow the host to write to the memory and keep both values consistent with each other
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                _hexOperationBuffer,
                _hexOperationBufferMemory);

        // Temporarily map the region of the host memory to the device memory and upload the marchdata
        void *data;
        vkMapMemory(_device
                            .
                                    getVkDevice(), _hexOperationBufferMemory,
                    0, _hexOperationBufferSize, 0, &data);
        memcpy(data, operations.data(),
               static_cast
                       <size_t>(_hexOperationBufferSize)
        );
        vkUnmapMemory(_device
                              .
                                      getVkDevice(), _hexOperationBufferMemory
        );
    }

};

#endif //_HEX_CALC_PIPELINE_H_
