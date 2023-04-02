#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec2 uvCoord;
varying vec3 fragPosition;

// Input uniform values
uniform sampler2D texture0;

uniform float texsize;
uniform float texscale;
uniform float gridsize;

void main() {
    vec2 coord = mod(fragTexCoord / texscale, texsize) 
                 + (floor(uvCoord / gridsize) * gridsize);
    
    vec3 color = texture2D(texture0, coord).rgb;

    // Calculate final fragment color
    gl_FragColor = vec4(color, 1.0f);
}