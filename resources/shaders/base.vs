#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Input uniform v alues
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
varying vec2 fragTexCoord;
varying vec2 fixedTexCoord;
varying vec3 fragPosition;

// NOTE: Add here your custom variables

void main() {
    // Send vertex attributes to fragment shader
    if(abs(vertexNormal.g) > 0.5f) {
        fixedTexCoord = vec2(
            vertexPosition.x,
            vertexPosition.z
        );
    }
    else {    
        fixedTexCoord = vec2(
            (vertexPosition.x * vertexNormal.b) - (vertexPosition.z * vertexNormal.r),
            vertexPosition.y
        );
    }

    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));;
    fragTexCoord = vertexTexCoord;

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0f);
}