#ifndef Player
#include <raylib.h>
#include <math.h>

class Player {
    public:
    float gravity = 0.03;
    float friction = 10;
    float jump = 5;
    float speed = 0.1;

    Vector3 position;
    Vector3 velocity = {0, 0, 0};
    Camera3D camera;
    Vector2 rotation = {0, 0};

    Player(Vector3 position) {
        this->position = position;
        this->camera = {0};
        this->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        this->camera.fovy = 45.0f;
        this->camera.projection = CAMERA_PERSPECTIVE;
    }
    Player(){}

    void Update() {
        float deltat = GetFrameTime();

        this->camera.position = {position.x, position.y + 1, position.z};
        this->camera.target = {position.x + sinf(rotation.x) * cos(rotation.y), position.y + 1 + sin(rotation.y), position.z + cosf(rotation.x) * cos(rotation.y)};

        rotation.x -= GetMouseDelta().x * 2 * deltat;
        rotation.y -= GetMouseDelta().y * 2 * deltat;
        
        if(rotation.y > PI/2.1)
            rotation.y = PI/2.1;
        if(rotation.y < -PI/2.1)
            rotation.y = -PI/2.1;

        position.x += velocity.x;
        position.y += velocity.y;
        position.z += velocity.z;

        velocity.x /= 1 + deltat * friction;
        velocity.z /= 1 + deltat * friction;

        velocity.y -= gravity * deltat;
        if(position.y < 5) {
            velocity.y = 0;
            position.y = 5;
        }

        // Controls
        if(IsKeyDown(KEY_W)) {
            velocity.x += speed * sinf(rotation.x) * deltat;
            velocity.z += speed * cosf(rotation.x) * deltat;
        }
        if(IsKeyDown(KEY_S)) {
            velocity.x -= speed * sinf(rotation.x) * deltat;
            velocity.z -= speed * cosf(rotation.x) * deltat;
        }
        if(IsKeyDown(KEY_A)) {
            velocity.x += speed * sinf(rotation.x + PI/2.0) * deltat;
            velocity.z += speed * cosf(rotation.x + PI/2.0) * deltat;
        }
        if(IsKeyDown(KEY_D)) {
            velocity.x -= speed * sinf(rotation.x + PI/2.0) * deltat;
            velocity.z -= speed * cosf(rotation.x + PI/2.0) * deltat;
        }

        if(IsKeyDown(KEY_SPACE) && velocity.y == 0)
            velocity.y = jump * deltat;
        
        friction = velocity.y == 0 ? 10 : 0;
        speed = velocity.y == 0 ? 0.1 : 0.005;
    }
};

#endif