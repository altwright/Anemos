#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

layout(push_constant, std430) uniform pc{
    mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(inPosition, 1.0);
    fragColor = inPosition.xyz;
}