#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant, std430) uniform pc{
    mat4 viewProjection;
};

layout(binding = 0) uniform UniformBuffer{
    mat4 model;
} ub;

void main() {
    gl_Position = viewProjection * ub.model * vec4(inPosition, 1.0);
    fragColor = inPosition;
    fragTexCoord = inTexCoord;
}