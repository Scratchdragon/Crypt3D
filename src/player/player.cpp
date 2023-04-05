#include <raylib.h>
#include <math.h>

#include <iostream>

using namespace std;

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

    BoundingBox feet;
    BoundingBox bounds;

    Player(Vector3 position) {
        this->position = position;
        this->camera = {0};
        this->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        this->camera.fovy = 45.0f;
        this->camera.projection = CAMERA_PERSPECTIVE;
    }
    Player(){}

    void Update() {
        feet = {
            {position.x - 0.19f, position.y - 0.1f, position.z - 0.19f},
            {position.x + 0.19f, position.y, position.z + 0.19f}
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

        this->camera.position = {position.x, position.y + 1, position.z};
        this->camera.target = {position.x + sinf(rotation.x) * cos(rotation.y), position.y + 1 + sin(rotation.y), position.z + cosf(rotation.x) * cos(rotation.y)};

        rotation.x -= GetMouseDelta().x / 100;
        rotation.y -= GetMouseDelta().y / 100;
        
        if(rotation.y > PI/2.1)
            rotation.y = PI/2.1;
        if(rotation.y < -PI/2.1)
            rotation.y = -PI/2.1;

        velocity.x /= 1 + friction;
        velocity.z /= 1 + friction;

        last_pos = position;

        position.x += velocity.x * deltat;
        position.y += velocity.y * deltat;
        position.z += velocity.z * deltat;

        velocity.y -= gravity;
        
        friction = grounded ? 0.1 : 0.01;
        speed = grounded ? 0.4 : 0.04;
    }

    void OnCollide(BoundingBox box) {
        // The center of the box (x and z only)
        Vector2 center = {(box.min.x + box.max.x) / 2.0f, (box.min.z + box.max.z) / 2.0f};
   
        Vector2 gradient = {-center.x + position.x, -center.y + position.z};
        float angle = atanf(gradient.y / gradient.x); // tan-1(rise/run) == angle
        float d = sqrtf( (velocity.x * velocity.x) + (velocity.z * velocity.z) );

        DrawLine3D(position, {
            position.x + cosf(angle) * d,
            position.y,
            position.z + sinf(angle) * d
        }, GREEN);
    }
};