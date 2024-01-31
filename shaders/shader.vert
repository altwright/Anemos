#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out uint baseInstance;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant, std430) uniform pc{
    mat4 viewProjection;
};

layout(binding = 0) uniform UniformBuffer{
    mat4 model[2];
} ub;

void main() {
    gl_Position = viewProjection * ub.model[gl_BaseInstance] * vec4(inPosition, 1.0);
    baseInstance = gl_BaseInstance;
    fragTexCoord = inTexCoord;
}