#include "Helper.h"

glm::mat4 convert_assimp_matrix (aiMatrix4x4 m)
{
	return glm::mat4 (
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		m.a4, m.b4, m.c4, m.d4
	);
}	