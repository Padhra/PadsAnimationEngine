#ifndef _SKELETON_H                // Prevent multiple definitions if this 
#define _SKELETON_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <vector>

#include "helper.h"
#include "Bone.h"

class Skeleton
{
	private:
		char boneNames[256][MAX_BONES]; //for bone hierarchy checking
		//char boneIDs[MAX_BONES]; //for bone id setting

		const aiMesh* mesh;
		
		Bone* root;
		bool rootFlag;

		std::vector<Bone*> bones;

	public:
		Skeleton(const aiMesh* mesh);
		virtual ~Skeleton();

		glm::mat4 TEMP_local_anims[MAX_BONES];

		bool ImportBoneHierarchy(aiNode* aiBone, Bone* bone, Bone* parent);

		//ReadNodeHierachy - recursive

		//You will also need to have a function to calculate the global transformation 
		//of all of the bones in the hierarchy. 
		//This function should be recursive and should traverse the hierarchy, 
		//working out the global transformation of each bone 
		//(using its local orientation and the global transform of its parent)


		//Getters
		std::vector<Bone*> GetBones() { return bones; }
		Bone* GetRootBone() { return root; }

		// recursive animation using hierarchy. animate node, children inherit animation 
		void GetGlobalTransforms(Bone* node, glm::mat4 parent_mat, glm::mat4* bone_animation_mats);

};

#endif