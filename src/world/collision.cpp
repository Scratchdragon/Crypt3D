#include <raylib.h>

#include <vector>
#include <iostream>

Vector3 Vec3FromString(std::string str) {
    // Split each number by ','
    int count = 0;
    const char ** values = TextSplit(str.c_str(), ',', &count);

    // Don't continue if the string is invalid
    if(count != 3)
        return {0, 0, 0};
    
    // Return the vector
    return {
        // Convert values to floats
        std::stof(values[0]),
        std::stof(values[1]),
        std::stof(values[2])
    };
}

struct BoundingWall {
    Vector3 min, max;
} typedef;

bool CheckWallCollision(BoundingBox a, BoundingWall b, float accuracy = 50) {
	if(b.min.z == b.max.z || b.min.x == b.max.x)
		return CheckCollisionBoxes(a, {b.min, b.max});

	float dz = ((b.max.z - b.min.z)/(b.max.x - b.min.x));
	for(float d = 0; d < 1; d+=1/accuracy) {
		float x = d*b.max.x;

		BoundingBox slice = {
			{b.min.x + x, b.min.y, b.min.z + x*dz}, 
			{b.min.x + x, b.max.y, b.min.z + x*dz}
		};
        DrawBoundingBox(slice, ORANGE);
		
		if(CheckCollisionBoxes(a, slice))
			return true;
	}
	return false;
}

BoundingBox GetWallCollide(BoundingBox a, BoundingWall b, float accuracy = 50) {
	if(b.min.z == b.max.z || b.min.x == b.max.x) {
		if(CheckCollisionBoxes(a, {b.min, b.max}))
            return a;
    }

	float dz = ((b.max.z - b.min.z)/(b.max.x - b.min.x));
	for(float d = 0; d < 1; d+=1/accuracy) {
		float x = d*b.max.x;

		BoundingBox slice = {
			{b.min.x + x, b.min.y, b.min.z + x*dz}, 
			{b.min.x + x, b.max.y, b.min.z + x*dz}
		};
        DrawBoundingBox(slice, ORANGE);
		
		if(CheckCollisionBoxes(a, slice))
			return slice;
	}
	return {0};
}

// Ensures that the values of "max" in the bounding box is bigger than "min"
BoundingBox OrderBounds(BoundingBox box) {
    int min; // The smaller of the two measurements
    if(box.min.x > box.max.x) {
        // Swap the x values
        min = box.max.x;
        box.max.x = box.min.x;
        box.min.x = min;
    }
    if(box.min.y > box.max.y) {
        // Swap the y values
        min = box.max.y;
        box.max.y = box.min.y;
        box.min.y = min;
    }
    if(box.min.z > box.max.z) {
        // Swap the z values
        min = box.max.z;
        box.max.z = box.min.z;
        box.min.z = min;
    }
    return box;
}

BoundingWall OrderBounds(BoundingWall wall) {
    BoundingBox box = OrderBounds((BoundingBox){wall.min, wall.max});
    return {
        box.min,
        box.max
    };
}

void DrawBoundingWall(BoundingWall wall, Color color) {
    DrawLine3D(
        wall.min,
        {wall.max.x, wall.min.y, wall.max.z},
        color
    );
    DrawLine3D(
        {wall.max.x, wall.min.y, wall.max.z},
        wall.max,
        color
    );
    DrawLine3D(
        wall.max,
        {wall.min.x, wall.max.y, wall.min.z},
        color
    );
    DrawLine3D(
        {wall.min.x, wall.max.y, wall.min.z},
        wall.min,
        color
    );
}

class CollisionMap {
    public:
    std::vector<BoundingBox> bounds;

    void AddBox(BoundingBox box) {
        // Ensure the box is ordered before appending
        bounds.push_back(OrderBounds(box));
    }

    // Null constructor
    CollisionMap(){}

    // Construct from file
    CollisionMap(const char * filename) {
        /* The layout for a collision map file follows the following rules:
            -  1 bounding box per line
            -  Line follows the format:  x,y,z:x,y,z
            -  Whitespace is NOT allowed
        */

        // Raw text file data
        char * content = LoadFileText(filename);
        
        // Split the text file by the \n charcode (meaning 'newline')
        int line_count = 0;
        const char ** lines = TextSplit(content, '\n', &line_count);
        
        // The TextSplit() method has a buffer that is overwrittin each time its used,
        //   for this reason the values got from TextSplit() need to be copied to avoid memory overwrites
        std::string lines_str[line_count];

        // Copy all the lines into a string array
        for(int i = 0 ;i < line_count; ++i) {
            // Assigning to a string is more memory safe than using strcpy() and allows more string modification
            lines_str[i] = lines[i]; 
        }

        for(int i = 0; i < line_count; ++i) {
            // Split the line by ':'
            int count = 0;
            const char ** pair = TextSplit(lines_str[i].c_str(), ':', &count);

            // Skip this iteration if the line is incorrect
            if(count != 2) 
                continue;
            
            // Again, copy the values to strings to avoid memory overwrites
            std::string min = pair[0];
            std::string max = pair[1];

            // Construct the box
            BoundingBox box = {
                Vec3FromString(min),
                Vec3FromString(max)
            };

            // Finally, add the box
            AddBox(box);
        }
    }
};