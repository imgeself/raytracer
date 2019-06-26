#version 450 core

in vec2 texCoord;

uniform sampler2D image;

out vec4 finalColor;

#define Clamp(A, c, B) clamp(c, A, B)

float LinearTosRGB(in float value) {
    value = Clamp(0.0f, value, 1.0f);

    float result = value * 12.92f;
    if (value >= 0.0031308f) {
        result = (1.055f * pow(value, 1.0f / 2.4f)) - 0.055f;
    }

    return result;
}

vec3 LinearVectorTosRGBVector(in vec3 value) {
    vec3 result;

    result.x = LinearTosRGB(value.x);
    result.y = LinearTosRGB(value.y);
    result.z = LinearTosRGB(value.z);

    return result;
}

void main () {
    vec3 rawColor = texture(image, texCoord).xyz;
    finalColor = vec4(LinearVectorTosRGBVector(rawColor), 1.0);
}