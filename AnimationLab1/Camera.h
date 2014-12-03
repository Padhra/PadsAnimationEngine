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

		bool flycam;

		int winw, winh;
		int inputX, inputY;

		//Flycam
		float horizontalXZAngle;
		float verticalAngle;

		float turnSpeed;
		float moveSpeed;

		//Third-person cam
		glm::vec3 target;
		float distance;
		float minDistance, maxDistance;

		float scrollWheelSensivity;

		//float yMaxLimit; //= -40f
		//float yMinLimit; //=70f
		
		void Init(glm::vec3 position, float p_turnSpeed = 0.005f, float p_moveSpeed = 0.01f, bool p_flycam = false ) 
		{ 
			viewProperties.position = position;
			viewProperties.up = glm::vec3(0,1,0);

			turnSpeed =  p_turnSpeed; 
			moveSpeed = p_moveSpeed; 

			flycam = p_flycam;

			distance = 10;
			minDistance = 3;
			maxDistance = 10;

			scrollWheelSensivity = 0.15f;

			target = glm::vec3(0,1,0);
		}

		void MouseRotate(int x, int y, int p_winw, int p_winh)
		{
			inputX = x;
			inputY = y;

			//if third person cam clamp angle

			winh = p_winh;
			winw = p_winw;
		}

		void Zoom(GLfloat amount)
		{
			distance += (amount * scrollWheelSensivity);
			distance = glm::clamp(distance, minDistance, maxDistance);
		}

		void SetTarget(glm::vec3 p_target)
		{
			target = p_target;
		}

		void Update(int deltaTime)
		{
			horizontalXZAngle += turnSpeed * deltaTime * float(winw/2 - inputX);
			verticalAngle += turnSpeed * deltaTime * float(winh/2 - inputY);

			if(flycam)
			{
				viewProperties.forward = glm::vec3(cos(verticalAngle) * sin(horizontalXZAngle),
											   sin(verticalAngle),
											   cos(verticalAngle) * cos(horizontalXZAngle));

				glm::vec3 right = glm::vec3(sin(horizontalXZAngle - 3.14f/2.0f), 0, cos(horizontalXZAngle - 3.14f/2.0f));
				viewProperties.up = glm::cross(right, viewProperties.forward);
			}
			else
			{
				glm::vec3 direction = glm::vec3(0, 0, -distance);

				glm::vec3 right = glm::vec3(sin(horizontalXZAngle - 3.14f/2.0f), 0, cos(horizontalXZAngle - 3.14f/2.0f));
				
				glm::quat rotation = glm::quat(glm::vec3(0, -horizontalXZAngle, 0)); //yaw, pitch
				rotation *= glm::angleAxis(-verticalAngle * 100, right);

				//xz rotation is around y axis
				//y rotation is around arbitary axis

				viewProperties.position = direction * rotation + target; 	
				viewProperties.forward = target - viewProperties.position;	
				viewProperties.up = glm::cross(right, viewProperties.forward);
			}
		}

		glm::mat4 GetViewMatrix()
		{
			return glm::lookAt(viewProperties.position, viewProperties.position + viewProperties.forward, viewProperties.up); 
			//glm::LookAt(
				//cameraPosition, // the position of your camera, in world space
				//cameraTarget,   // where you want to look at, in world space
				//upVector        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
			//);
		}
};

#endif