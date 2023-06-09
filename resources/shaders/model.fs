#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fixedTexCoord;
varying vec2 fragTexCoord;
varying vec3 fragPosition;

// Uniform texture samples
uniform sampler2D texture0;

// Fog uniforms
uniform vec3 view;
uniform vec3 fogColor;
uniform float fogAmount;

// Define the light struct
struct Light {
    float active;
    float brightness;
    vec3 position;
};

// Lighting uniforms
uniform Light lights[255];
uniform int lightc;

// Other uniforms
uniform vec3 tint;

void main() {
    // Lighting
    float light = 0.0;
    for(int i = 0; i < lightc; ++i) {
        light += (1.0 / distance(fragPosition, lights[i].position)) * lights[i].brightness * lights[i].active;
    }
    if(light >= 1.0) {
        light = 1.0;
    }

    vec3 color = texture2D(texture0, fragTexCoord).rgb;
    float fogMix = 1.0/pow(distance(view, fragPosition) * fogAmount, 2.0);

    // Calculate final fragment color
    vec3 finalColor = mix(fogColor, color, clamp(fogMix, 0.0, 1.0)) * light;
    float grey = (finalColor.r + finalColor.g + finalColor.b) / 3.0;

    // Limit the colors
    float colors = 30.0;
    grey = floor(grey * colors + 0.5) / colors;
    vec4 colorCap = vec4(tint * grey, 1.0);

    gl_FragColor = mix(vec4(finalColor, 1.0), colorCap, 0.3);
}