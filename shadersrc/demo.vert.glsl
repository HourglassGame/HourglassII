#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint inUseTexture;
layout(location = 4) in uint inColourTexture;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint fragUseTexture;
layout(location = 3) out uint fragColourTexture;

void main() {
/*
    vec2 center = vec2(512.0, 304.0);
    vec2 size = vec2(1036.93152, 608.000000);
    float a =  2.f / size[0]; //scale x
    float b =  2.f / size[1]; //scale y
    float c = -a * center[0]; //translate x
    float d = -b * center[1]; //translate y

    mat4 proj = mat4(
 //Out  x    y    z    v
        a  , 0.0, 0.0, 0.0,//In x
        0.0, b  , 0.0, 0.0,//In y
        0.0, 0.0, 1.0, 0.0,//In z
        c  , d  , 0.0, 1.0 //In v
    );
*/
    gl_Position = ubo.proj * vec4(inPosition, 0.0, 1.0);
    fragColour = inColour;
    //fragColour = vec3(1.0, 0.0, 1.0);
    fragTexCoord = inTexCoord;//vec2(0.5, 0.5);
    fragUseTexture = inUseTexture;
    fragColourTexture = inColourTexture;
}