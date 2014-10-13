#include "Skeleton.h"

Skeleton::Skeleton(const aiMesh* p_mesh)
{
	mesh = p_mesh;

	if(mesh == nullptr)
	{
		printf("Skeleton needs mesh data\n");
		return;
	}


	for (int i = 0; i < (int)mesh->mNumBones; i++) 
	{
		const aiBone* bone = mesh->mBones[i];
		strcpy (boneNames[i], bone->mName.data);
	}

	rootFlag = false;
}

Skeleton::~Skeleton()
{

}

bool Skeleton::ImportBoneHierarchy(aiNode* aiBone, Bone* bone, Bone* parent)
{
	bone->name = aiBone->mName.C_Str();
	bone->parent = parent; 

	for (int i = 0; i < (int)aiBone->mNumChildren; i++) 
	{
		Bone* child = new Bone;

		//int one = mesh->mBones[0]->mNumWeights;
		////mesh->mBones[0]->mWeights[
		//int two = mesh->mBones[1]->mNumWeights;
		//int three = mesh->mBones[2]->mNumWeights;

		if (ImportBoneHierarchy (aiBone->mChildren[i], child, bone)) //depth first
		{
			for(int j = 0; j < mesh->mNumBones; j++) //The ID is the order of mNumBones
			{
				if (strcmp (boneNames[j], child->name) == 0) //If my child is a bone
				{
					bone->children.push_back(child); //add the child to my children					
					bones.push_back(child); //also add it to the skeleton

					child->offset = convert_assimp_matrix(mesh->mBones[j]->mOffsetMatrix);
					child->id = j;

					root = child; //root will be the last bone to run this line
					break;
				}
			}
		}
	}

	return true;
}

void Skeleton::GetGlobalTransforms(Bone* bone, glm::mat4 parent_mat, glm::mat4* bone_animation_mats) 
{	
	glm::mat4 our_mat = parent_mat * glm::inverse(bone->offset) * TEMP_local_anims[bone->id] * bone->offset;
	
	bone_animation_mats[bone->id] = our_mat;
	
	for (int i = 0; i < bone->children.size(); i++) 
		GetGlobalTransforms (bone->children[i], our_mat, bone_animation_mats);
}