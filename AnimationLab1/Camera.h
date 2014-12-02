#ifndef _CAMERA_H                // Prevent multiple definitions if this 
#define _CAMERA_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

struct ViewProperties 
{
	glm::vec3 position;
	glm::vec3 forward; 
	glm::vec3 up;
};

class Camera
{
	private:

	public:

		ViewProperties viewProperties;

		float horizontalAngle;
		float verticalAngle;

		float turnSpeed;
		float moveSpeed;
		
		void Init(float p_turnSpeed = 0.0005f, float p_moveSpeed = 0.01f ) { turnSpeed =  p_turnSpeed; moveSpeed = p_moveSpeed; }

		void ProcessMouse(int x, int y, int deltaTime, int winw, int winh)
		{
			horizontalAngle += turnSpeed * deltaTime * float(winw/2 - x);
			verticalAngle += turnSpeed * deltaTime * float(winh/2 - y);

			viewProperties.forward = glm::vec3(cos(verticalAngle) * sin(horizontalAngle),
											   sin(verticalAngle),
											   cos(verticalAngle) * cos(horizontalAngle));

			glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.14f/2.0f),
										0,
										cos(horizontalAngle - 3.14f/2.0f));

			viewProperties.up = glm::cross(right, viewProperties.forward);
		}
};

#endif