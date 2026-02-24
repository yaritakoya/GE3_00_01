#pragma once
struct Vector3 {
	float x;
	float y;
	float z;
};
struct Vector2 {
    float x;
    float y;

    Vector2& operator+=(const Vector2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vector2 operator+(const Vector2& rhs) const {
        return { x + rhs.x, y + rhs.y };
    }
};


struct Vector4 {
	float x;
	float y; 
	float z;
	float w;
};

// 球  
struct Sphere {
	Vector3 center; //!< 中心点  
	float radius;   //!< 半径  
};

