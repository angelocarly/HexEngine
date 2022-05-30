# VulkanEngine

This is my try at learning C++ and Vulkan at the same time. Do not expect much from this project.

## Todo
- [ ] Core
  - [ ] Make sure all pointers are deleted after usage
- [ ]  Refactoring
- [ ] Move rendering specific code out of game.hpp
- [ ] Move implementation specific code out of the rendering logic
- [ ] Remove unused code
- [ ] Implement Vulkan
  - [ ] Use initializers to clean up vulkan code
  - [ ] Load shaders dynamically
- [ ] Build engine on Windows as well

## Installation

Required dependencies
- `yay -S vulkan-devel boost tinyobjloader glm glfw-x11`

Build:
1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. `make`

## Running

Launch the `Vulkanengine` application in build/
