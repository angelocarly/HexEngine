#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

void main() {
    //    outColor = vec4(fragColor, 1.0);
    outColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
}