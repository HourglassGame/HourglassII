#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) flat in uint useTexture;
layout(location = 3) flat in uint colourTexture;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(fragColor, 1.0);
    if (useTexture == 1) {
        if (colourTexture == 0) {
            outColor = texture(texSampler, texCoord);
        }
        else {
            //Hack for drawing bitmapped font that uses white rather than alpha for background.
            outColor = texture(texSampler, texCoord) == vec4(1.0,1.0,1.0,1.0) ? vec4(fragColor, 0.0) : vec4(fragColor, 1.0);
        }
    }
    else {
        outColor = vec4(fragColor, 1.0);
    }
}
