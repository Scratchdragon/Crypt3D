#include <raylib.h>
#include <math.h>

#include <iostream>

#define DEBUG false

using namespace std;

float lerp(float a, float b, float f) {
    return a * (1.0 - f) + (b * f);
}

class Player {
    private:
    float deltat;

    public:
    float gravity = 0.1f;
    float friction = 10;
    float jump = 5;
    float speed = 1;

    bool grounded;

    Vector3 position, last_pos;
    Vector3 velocity = {0, 0, 0};
    Camera3D camera;
    Vector2 rotation = {0, 0};

    Vector2 gun_rotation;
    Vector3 gun_position;

    BoundingBox feet;
    BoundingBox bounds;

    Player(Vector3 position) {
        this->position = position;
        this->camera = {0};
        this->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        this->camera.fovy = 60.0f;
        this->camera.projection = CAMERA_PERSPECTIVE;

        if(DEBUG) gravity = 0;
    }
    Player(){}

    void Update() {
        feet = {
            {position.x - 0.15f, position.y - 0.1f, position.z - 0.15f},
            {position.x + 0.15f, position.y, position.z + 0.15f}
        };
        bounds = {
            {position.x - 0.2f, position.y, position.z - 0.2f},
            {position.x + 0.2f, position.y + 1.2f, position.z + 0.2f}
        };

        deltat = GetFrameTime();

        if(IsKeyDown(KEY_SPACE) && grounded)
            velocity.y = jump;
        else if(grounded)
            velocity.y = 0;

        // Controls
        if(IsKeyDown(KEY_W)) {
            velocity.x += speed * sinf(rotation.x);
            velocity.z += speed * cosf(rotation.x);
        }
        if(IsKeyDown(KEY_S)) {
            velocity.x -= speed * sinf(rotation.x);
            velocity.z -= speed * cosf(rotation.x);
        }
        if(IsKeyDown(KEY_A)) {
            velocity.x += speed * sinf(rotation.x + PI/2.0);
            velocity.z += speed * cosf(rotation.x + PI/2.0);
        }
        if(IsKeyDown(KEY_D)) {
            velocity.x -= speed * sinf(rotation.x + PI/2.0);
            velocity.z -= speed * cosf(rotation.x + PI/2.0);
        }
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            velocity.y -= speed;
        }

        this->camera.position = {position.x, position.y + 1, position.z};
        this->camera.target = {position.x + sinf(rotation.x) * cos(rotation.y), position.y + 1 + sin(rotation.y), position.z + cosf(rotation.x) * cos(rotation.y)};

        gun_position = {
            camera.target.x + sin(rotation.y) * 0.4f * sin(rotation.x),
            camera.target.y - cos(rotation.y) * 0.4f,
            camera.target.z + sin(rotation.y) * 0.4f * cos(rotation.x)
        };

        gun_rotation = {
            lerp(gun_rotation.x, rotation.x - (PI/2), deltat / 10 + 0.5), 
            lerp(gun_rotation.y, rotation.y, deltat / 10 + 0.5)
        };

        rotation.x -= GetMouseDelta().x / 100;
        rotation.y -= GetMouseDelta().y / 100;
        
        if(rotation.y > PI/2.1)
            rotation.y = PI/2.1;
        if(rotation.y < -PI/2.1)
            rotation.y = -PI/2.1;

        velocity.x /= 1 + friction;
        velocity.z /= 1 + friction;
        if(DEBUG) {
            velocity.y /= 1 + friction;
            if(IsKeyDown(KEY_SPACE))
                velocity.y += speed;
        };

        last_pos = position;

        position.x += velocity.x * deltat;
        position.y += velocity.y * deltat;
        position.z += velocity.z * deltat;

        velocity.y -= gravity;
        
        friction = grounded ? 0.15 : 0.01;
        speed = grounded ? 0.7 : 0.04;

        if(position.y < -7)
            position = {0, 5, 0};
    }

    void OnCollide(BoundingBox box) {
        if(DEBUG) return;

        // The center of the box (x and z only)
        Vector2 center = {(box.min.x + box.max.x) / 2.0f, (box.min.z + box.max.z) / 2.0f};
   
        // Calculate the direction away from the wall and the net velocity
        float angle = atan2f(-center.y + position.z, -center.x + position.x); // tan-1(rise/run) == angle
        float v = sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)); // Total directional velocity

        // Move in the calculated direction
        position.x += cosf(angle) * v * deltat;
        position.z += sinf(angle) * v * deltat;

        velocity = {
            (cosf(angle) * v + velocity.x) / 2.0f,
            velocity.y,
            (sinf(angle) * v + velocity.z) / 2.0f
        };
    }
};