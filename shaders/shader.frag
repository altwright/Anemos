#version 460

layout(location = 0) flat in uint baseInstance;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[2];

void main() {
    outColor = texture(texSampler[baseInstance], fragTexCoord);//vec4(fragColor, 1.0);//
    //outColor = vec4(fragTexCoord, 0.0f, 1.0f);
}