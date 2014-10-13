#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <glm\glm.hpp>

#define MAX_BONES 32 
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

glm::mat4 convert_assimp_matrix (aiMatrix4x4 m);
