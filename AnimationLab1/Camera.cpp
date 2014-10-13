#include "Camera.h"

void Camera::ProcessMouse(int x, int y, int deltaTime, int winw, int winh) {

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