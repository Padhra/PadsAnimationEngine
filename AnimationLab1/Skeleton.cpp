#include "Skeleton.h"
#include <sstream>
#include <iostream>
#include "Model.h"

Skeleton::Skeleton(Model* p_myModel, const aiMesh* p_mesh, bool keyframes)
{
	animationTimer = 0.0;

	hasKeyframes = keyframes;

	model = p_myModel;
}

Skeleton::~Skeleton()
{
	//release bone memory
}

//skeleton->ImportAssimpBoneHierarchy(scene, scene->mRootNode, (Bone*)malloc (sizeof (Bone)), nullptr))
bool Skeleton::ImportAssimpBoneHierarchy(const aiScene* scene, aiNode* aiBone, Bone* parent)
{
	Bone* bone = new Bone;
	strcpy(bone->name ,aiBone->mName.C_Str());

	for (int i = 0; i < (int)aiBone->mNumChildren; i++) 
		ImportAssimpBoneHierarchy (scene, aiBone->mChildren[i], bone); //depth first

	for(int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) //For every mesh
	{
		for(int boneIdx = 0; boneIdx < scene->mMeshes[meshIdx]->mNumBones; boneIdx++) //For every bone in said mesh
		{
			//if(bone->name == "head")
				//int breaki = 0;

			if (strcmp (scene->mMeshes[meshIdx]->mBones[boneIdx]->mName.data, bone->name) == 0) //Is this node actually a bone?
			{
				if(std::find(bonesAdded.begin(), bonesAdded.end(), bone->name) == bonesAdded.end())
				{
					bone->id = bones.size();
					bones[bone->id] = bone;

					bonesAdded.push_back(bone->name);
					boneNameToID[bone->name] = bone->id; //TODO - make this nicer

					bone->aibone = scene->mMeshes[meshIdx]->mBones[boneIdx]; //for grabbing weights

					//TODO - copy out weights too
					/*for(int weightIdx = 0; weightIdx < scene->mMeshes[meshIdx]->mBones[boneIdx]->mNumWeights; weightIdx++)
					{
						aiVertexWeight aiWeight = scene->mMeshes[meshIdx]->mBones[boneIdx]->mWeights[weightIdx];
						Weight weight;
						weight.vertexID = aiWeight.mVertexId;
						weight.weighting = aiWeight.mWeight;

						bone->weights.push_back(weight);
					}*/

					bone->parent = parent;
					bone->parent->children.push_back(bone);
					
					bone->offset = convert_assimp_matrix(scene->mMeshes[meshIdx]->mBones[boneIdx]->mOffsetMatrix);
					//bone->inv_offset = glm::inverse(bone->offset);

					glm::vec3 offsetTranslation = decomposeT(bone->offset);
					glm::vec3 mTransformTranslation = decomposeT(convert_assimp_matrix(aiBone->mTransformation));

					std::stringstream ss;

					ss << "\n\nbone->name: " << bone->name;
					ss << "\nbone->parent: " << bone->parent->name;
					ss << "\nnumberOfChildren: " << bone->children.size();
					ss << "\nbone->offset: (x:" << offsetTranslation.x << ", y: " << offsetTranslation.y << ", z: " << offsetTranslation.z << ")";
					ss << "\nmTransform: (x:" << mTransformTranslation.x << ", y: " << mTransformTranslation.y << ", z: " << mTransformTranslation.z << ")";
					std::cout << ss.str();

					//bone->transform = glm::mat4(1);
					bone->transform = convert_assimp_matrix(aiBone->mTransformation);

					root = bone->parent; //The last guy to get in here is the root, as it is depth first

					break;
				}
			}
		}
	}

	return true;
}

void Skeleton::PrintHeirarchy(Bone* bone)
{
	printf("\nbone->name: %s \n", bone->name);
	//printf("bone->id: %i \n", bone->id);
	printf("bone->parent: %i \n", bone->parent->name);

	printf("numberOfChildren: %i \n", bone->children.size());

	for(int i = 0; i < bone->children.size(); i++)
		PrintHeirarchy(bone->children[i]);
}

void Skeleton::UpdateGlobalTransforms(Bone* bone, glm::mat4 parentTransform) 
{	
	//We will multiply by the bone offset matrix to use the bone as the pivot point for animation
	//Move to root transform, apply local translation, then apply all inherited parent translations

	//The keyframes are given in three separate series of values, one each for position, rotation and scaling. 
	//The transformation matrix computed from these values replaces the node's original transformation matrix at a specific time. 
	//This means all keys are absolute and not relative to the bone default pose.
	bone->globalTransform = parentTransform * bone->transform;
	
	bone->finalTransform = bone->globalTransform * bone->offset;

	for (int i = 0; i < bone->children.size(); i++) 
		UpdateGlobalTransforms (bone->children[i], bone->globalTransform); 
}

void Skeleton::Animate(double deltaTime)
{
	if(!hasKeyframes)
		return;

	animationTimer += deltaTime/1000;

	if (animationTimer >= animationDuration) 
		animationTimer = animationDuration - animationTimer; //animation will loop

	for(int boneidx = 0; boneidx < bones.size(); boneidx++)
	{
		glm::mat4 translation = glm::mat4(1);
		glm::mat4 orientation = glm::mat4(1);

		if (bones[boneidx]->posKeyframes.size() > 0)  // if this bone has keyframes
		{
			int prev_key = 0;
			int next_key = 0;
		
			//Find the two keyframes
			for (int keyidx = 0; keyidx < bones[boneidx]->posKeyframes.size() - 1; keyidx++) 
			{
				prev_key = keyidx;
				next_key = keyidx + 1;

				if (bones[boneidx]->posKeyframes[next_key]->time >= animationTimer) // if the next keyframe is greater than the timer, then we have our two keyframes
					break;
			}
		
			float timeBetweenKeys = bones[boneidx]->posKeyframes[next_key]->time - bones[boneidx]->posKeyframes[prev_key]->time;
			float t = (animationTimer - bones[boneidx]->posKeyframes[prev_key]->time) / timeBetweenKeys; 

			std::stringstream ss;
			ss << "t: "<< t;
			drawText(20,20, ss.str().c_str());

			translation = glm::translate(glm::mat4(1), lerp(bones[boneidx]->posKeyframes[prev_key]->position, bones[boneidx]->posKeyframes[next_key]->position, t));
		}

		if (bones[boneidx]->rotKeyframes.size() > 0)  // if this bone has keyframes
		{
			int prev_key = 0;
			int next_key = 0;
		
			//Find the two keyframes
			for (int keyidx = 0; keyidx < bones[boneidx]->rotKeyframes.size() - 1; keyidx++) 
			{
				prev_key = keyidx;
				next_key = keyidx + 1;

				if (bones[boneidx]->rotKeyframes[next_key]->time >= animationTimer) // if the next keyframe is greater than the timer, then we have our two keyframes
					break;
			}

			float timeBetweenKeys = bones[boneidx]->rotKeyframes[next_key]->time - bones[boneidx]->rotKeyframes[prev_key]->time;
			float t = (animationTimer - bones[boneidx]->rotKeyframes[prev_key]->time) / timeBetweenKeys;
	
			glm::quat interpolatedquat = glm::mix(bones[boneidx]->rotKeyframes[prev_key]->rotation, bones[boneidx]->rotKeyframes[next_key]->rotation, t);
			orientation = glm::toMat4(interpolatedquat);
		}

		bones[boneidx]->transform = translation * orientation; 
	}
}

Bone* Skeleton::GetBone(std::string name)
{
	return bones[boneNameToID[name]];
}

bool Skeleton::ComputeIK(std::string chainName, glm::vec3 T, int steps)
{
	float distanceThreshold = 1.0f;
	
	std::vector<Bone*> links = ikChains[chainName];

	int tries = 0;
	int maxTries = steps * (links.size() - 1); //so a try does one full iteration up the chain

	int effectorIdx = links.size()-1; //Effector is in the last position
	int linkIdx = effectorIdx - 1; //current link one up from effector

	glm::vec3 B, E; //These need to be world positions

	do
	{
		Bone* bone = links[linkIdx]; //Bone we're currently working on

		//glm::mat4 modelMat = model->GetModelMatrix();

		B = glm::vec3(/*modelMat **/ glm::vec4(bone->GetMeshSpacePosition(), 1));
		E = glm::vec3(/*modelMat **/ glm::vec4(links[effectorIdx]->GetMeshSpacePosition(), 1));

		if(vectorSquaredDistance(E, T) > distanceThreshold)
		{
			glm::vec3 BE = E - B; //vector in the direction of the effector
			glm::vec3 BT = T - B; //vector in the direction of the target

			BE = glm::normalize(BE);
			BT = glm::normalize(BT);

			double cosAngle = glm::dot(BT, BE);
			float turnAngle = glm::degrees(glm::acos(cosAngle));

			glm::mat4 rotation = glm::mat4(1);

			if(cosAngle < 0.9999f) // IF THE DOT PRODUCT RETURNS 1.0, I DON'T NEED TO ROTATE AS IT IS 0 DEGREES
			{
				glm::vec3 rotationAxis = glm::normalize(glm::cross(BE,BT));
				rotationAxis = glm::mat3(glm::inverse(bone->finalTransform * glm::inverse(bone->offset))) * rotationAxis;

				rotation = glm::rotate(glm::mat4(1), turnAngle, rotationAxis);

				bone->transform *= rotation;
				UpdateGlobalTransforms(bone, bone->parent->globalTransform);
			}

			if(--linkIdx < 0)
				linkIdx = effectorIdx - 1;

			tries++;	
		}
	}
	while(tries < maxTries && glm::distance(E, T) > distanceThreshold);

	if(tries == maxTries)
		return true;
	else
		return false;	
}

void Skeleton::DefineIKChain(std::string name, std::vector<Bone*> chain)
{
	ikChains[name] = chain;
}

