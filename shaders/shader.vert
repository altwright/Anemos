#version 450

layout(location = 0) in vec4 inPosition;

layout(push_constant, std430) uniform pc{
    mat4 mvp;
};

void main() {
    gl_Position = mvp * inPosition;
}