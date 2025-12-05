#version 460 core
layout(location = 0) out vec4 fragColor;

layout(location = 2) in vec2 texCoord;

layout(binding = 0) uniform sampler2D tex;

layout(location = 3) uniform vec2 res;

int samples = 5;

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float remap(float value, float inputMin, float inputMax, float outputMin, float outputMax) {
    float normalizedValue = (value - inputMin) / (inputMax - inputMin);
    return normalizedValue * (outputMax - outputMin) + outputMin;
}

void main() {
    vec3 finalColor = texture(tex, texCoord).rgb;
    // vec2 texSize = 1.0 / res;
    //
    // for (int i = -samples; i < samples; i++) {
    //     for (int k = -samples; k < samples; k++) {
    //         vec2 coord = vec2(texCoord.x + texSize.x * i, texCoord.y + texSize.y * k);
    //         float dist = distance(coord, texCoord);
    //         vec3 samp = texture(tex, coord).rgb;
    //         finalColor += samp * (0.37 - dist) * (1.0 / (samples * samples));
    //     }
    // }
    // finalColor *= 1.5f;
    fragColor = vec4(finalColor, 1.0);
}
