//
//  VulkanHeaders.h
//  VulkanMacos
//
//  Created by Miguel Lopes on 15/04/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE ///The perspective projection matrix generated by GLM will use the OpenGL depth range of -1.0 to 1.0 by default. We need to configure it to use the Vulkan range of 0.0 to 1.0 using the GLM_FORCE_DEPTH_ZERO_TO_ONE definition.#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

