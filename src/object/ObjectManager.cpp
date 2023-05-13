#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#include "object/GameObject.cpp"
#include "player/Player.cpp"

using namespace std;

class ObjectManager {
    private:
    map<string, GameObject> types;
    
    // Objects with partner collision
    vector<GameObject*> collidable;

    public:
    unsigned int object_count = 0;
    vector<GameObject> objects;

    void RegisterType(GameObject object) {
        cout << "INFO: OBJECT: Registered new object type '" << object.type << "'\n";
        types.insert({object.type, object});
    }

    void Create(string type, string name, Vector3 position, Vector3 rotation) {
        // Clone the object template
        GameObject obj = types.at(type);

        // Initialise the values
        obj.name = name;
        obj.position = position;
        obj.rotation = rotation;
        obj.id = object_count;

        // Trigger the OnStart method
        obj.OnStart();

        // Add the obj
        objects.push_back(obj);
        if(obj.collision_level == PARTNER_COLLISION)
            collidable.push_back(&objects[object_count]);

        // Update the object counter
        object_count = objects.size();
    }

    void Delete(unsigned int id) {
        // Delete the element
        objects.at(id).OnDelete();
        objects.erase(objects.begin() - 1 + id);

        // Remove from collidable list
        for(int i = 0; i < collidable.size(); ++i) {
            if(collidable[i]->id == id)
                collidable.erase(collidable.begin() - 1 + i);
        }

        // Update the other element ids
        for(int i = 0; i < objects.size(); ++i)
            objects[i].id = i;

        // Update the object counter
        object_count = objects.size();
    }

    void Update(float deltat, Player * player) {
        bool collision;
        for(int i = 0; i < objects.size(); ++i) {
            objects[i].Update(deltat);

            // Calculate the bounds of the object
            objects[i].bounds = {
                {
                    objects[i].local_bounds.min.x + objects[i].position.x, 
                    objects[i].local_bounds.min.y + objects[i].position.y, 
                    objects[i].local_bounds.min.z + objects[i].position.z
                },
                {
                    objects[i].local_bounds.max.x + objects[i].position.x, 
                    objects[i].local_bounds.max.y + objects[i].position.y, 
                    objects[i].local_bounds.max.z + objects[i].position.z
                }
            };

            // Early continue to save time if no collision
            if(objects[i].collision_level == NO_COLLISION)
                continue;
            
            collision = false;
            GameObject * collide_obj;

            GameObject player_obj;
            player_obj.type = "Player";

            switch(objects[i].collision_level) {
                // Player collision only checks collisions with the player
                case PLAYER_COLLISION:
                    // Create dummy player object
                    collide_obj = &player_obj;

                    // Check feet collider
                    collision = CheckCollisionBoxes(player->feet, objects[i].bounds);
                    if(collision)
                        collide_obj->name = "feet";
                    
                    // Check body collider
                    collision = CheckCollisionBoxes(player->bounds, objects[i].bounds);
                    if(collision)
                        collide_obj->name = "body";
                    break;

                // Partner collision only checks collisions with the same collision level
                case PARTNER_COLLISION:
                    // Check the objects in the collidable buffer
                    for(GameObject * object : collidable) {
                        if(CheckCollisionBoxes(object->bounds, objects[i].bounds)) {
                            collision = true;
                            collide_obj = object;
                            break;
                        }
                    }
                    break;

                // Global collision checks collision with all objects
                case GLOBAL_COLLISION:
                    // Check all the objects
                    for(GameObject object : objects) {
                        if(CheckCollisionBoxes(object.bounds, objects[i].bounds)) {
                            collision = true;
                            collide_obj = &object;
                            break;
                        }
                    }

                    // Global collision also applies to the player
                    if(!collision) {
                        collision = CheckCollisionBoxes(player->bounds, objects[i].bounds);
                        collide_obj = &player_obj;
                    }
                    break;
            }

            // Trigger collide methods
            if(collision && collide_obj->type == "Player") {
                // Trigger player collisions
                if(collide_obj->name == "feet") {
                    // Make the player grounded
                    player->grounded = true;
                    if(objects[i].bounds.max.y > objects[i].bounds.min.y)
                        player->position.y = objects[i].bounds.max.y + 0.1f;
                    else
                        player->position.y = objects[i].bounds.min.y + 0.1f;
                }
                else
                    player->OnCollide(objects[i].bounds);
                
                // Trigger the object collision
                objects[i].OnCollide(player);
            }
            else if(collision)
                objects[i].OnCollide(collide_obj);
        }
    }

    void Render(float deltat, Renderer * renderer) {
        for(int i = 0; i < objects.size(); ++i) {
            objects[i].Render(deltat, renderer);
        }
    }
};