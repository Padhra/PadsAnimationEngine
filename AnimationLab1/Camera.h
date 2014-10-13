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

class Camera
{
	private:

	public:

		struct ViewProperties {
			glm::vec3 position;
			glm::vec3 forward; 
			glm::vec3 up;
		};

		ViewProperties viewProperties;

		float horizontalAngle;
		float verticalAngle;

		float turnSpeed;
		float moveSpeed;
		
		void Init() { turnSpeed =  0.0001f; moveSpeed = .01f; }

		void ProcessMouse(int x, int y, int deltaTime, int winw, int winh);
};

#endif