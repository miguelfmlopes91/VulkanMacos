//
//  Vertex.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 15/04/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once

#include "VulkanHeaders.h"

#include <glm/gtx/hash.hpp>
#include <array>
#include <unordered_map>

struct Vertex {
    static VkVertexInputBindingDescription getBindingDescription();
    ///describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    
    //For hash table imp
    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
    
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;///the actual coordinates for each vertex. The coordinates determine how the image is actually mapped to the geometry.
};

///A hash function for Vertex is implemented by specifying a template specialization for std::hash<T>. Hash functions are a complex topic, but cppreference.com recommends the following approach combining the fields of a struct to create a decent quality hash function:
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
