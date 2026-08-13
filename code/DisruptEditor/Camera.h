#pragma once
#include "glm/glm.hpp"

class Camera {
public:
	// original value of phi: 5.f * 3.14159265f / 8.f
	Camera() : up(0.f, 0.f, 1.f), type(ORBIT), phi(3.14159265f / 2.f), theta(0.f), radius(10.f), lat(0.f), lon(-90.f) {}
	void update(float delta);
	
	glm::vec3 location = glm::vec3(256, 336, 234);
	glm::vec3 lookingAt;
	glm::vec3 up;
	
	enum Type {
		FLYCAM, ORBIT
	};
	Type type;
	
	float phi;
	float theta;
	float radius;
	float lat;
	float lon;
};
