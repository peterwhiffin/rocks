#version 460 core
layout(location = 0) out vec4 fragColor;

layout(location = 6) in vec2 fragPos;

layout(location = 8) uniform vec3 color;
layout(location = 9) uniform float time;

void main() {
    vec2 rolling = vec2(sin(time), cos(time));
    float sinT = sin(time);
    float cosT = cos(time);

    vec3 finalColor = vec3(max(0.1, abs(sinT)), 1.0 - distance(fragPos, rolling), max(0.1, abs(cosT)));
    vec3 normColor = normalize(finalColor);

    float mag = distance(finalColor, vec3(0.0f, 0.0f, 0.0f));
    mag = max(mag, 0.8f);
    finalColor = normColor * mag;
    fragColor = vec4(finalColor, 1.0);
}
