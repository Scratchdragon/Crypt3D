#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fixedTexCoord;
varying vec2 fragTexCoord;
varying vec3 fragPosition;

// Uniform texture samples
uniform sampler2D texture0;
uniform sampler2D texture1;

// Tiling uniforms
uniform float texsize;
uniform float texscale;
uniform float gridsize;
uniform float pixscale;

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
    // Snap fragpos to grid
    vec3 gridPosition = floor(fragPosition*pixscale)/pixscale;

    // Lighting
    float light = 0.0;
    for(int i = 0; i < lightc; ++i) {
        light += (1.0 / distance(gridPosition, lights[i].position)) * lights[i].brightness * lights[i].active;
    }

    // Texture map
    vec2 coord = mod(fixedTexCoord / texscale, texsize) 
                 + (floor(fragTexCoord / gridsize) * gridsize);
    vec3 color = texture2D(texture0, coord).rgb;
    float fogMix = 1.0/pow(distance(view, gridPosition) * fogAmount, 2.0);

    // Calculate final fragment color
    vec3 finalColor = mix(fogColor, color, clamp(fogMix, 0.0, 1.0)) * light;
    float grey = (finalColor.r + finalColor.g + finalColor.b) / 3.0;

    // Limit the colors
    float colors = 30.0;
    grey = floor(grey * colors + 0.5) / colors;
    vec4 colorCap = vec4(tint * grey, 1.0);

    gl_FragColor = mix(vec4(finalColor, 1.0), colorCap, 0.3);
}