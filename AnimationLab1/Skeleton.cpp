#include "Skeleton.h"
#include <sstream>
#include <iostream>
#include "Model.h"

bool Skeleton::ConstraintsEnabled = true;
float Skeleton::AnimationSpeedScalar = 1.0f;

Skeleton::Skeleton(Model* p_myModel)
{
	currentAnimationIndex = -1;

	hasKeyframes = false;

	model = p_myModel; // for the model matrix
}

Skeleton::~Skeleton()
{
	//TODO - release bone memory
}

bool Skeleton::ImportAssimpBoneHierarchy(const aiScene* scene, aiNode* aiBone, Bone* parent, bool print)
{
	Bone* bone = new Bone;
	strcpy(bone->name ,aiBone->mName.data);

	for (int i = 0; i < (int)aiBone->mNumChildren; i++) 
		ImportAssimpBoneHierarchy (scene, aiBone->mChildren[i], bone, print); //depth first

	if(print)
	{
		std::stringstream ss;
		ss << "\n\nIs " << bone->name << " a bone?";
		std::cout << ss.str();
	}

	for(int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) //For every mesh
	{
		for(int boneIdx = 0; boneIdx < scene->mMeshes[meshIdx]->mNumBones; boneIdx++) //For every bone in said mesh
		{
			if (strcmp (scene->mMeshes[meshIdx]->mBones[boneIdx]->mName.data, bone->name) == 0) //Is this node actually a bone?
			{
				if(std::find(bonesAdded.begin(), bonesAdded.end(), bone->name) == bonesAdded.end())
				{
					bone->id = bones.size();
					bones[bone->id] = bone;

					bonesAdded.push_back(bone->name);
					boneNameToID[bone->name] = bone->id; //TODO - make this nicer

					//bone->aibone = scene->mMeshes[meshIdx]->mBones[boneIdx]; //for grabbing weights

					#pragma region TODO - Copy out weights
					//TODO - copy out weights too
					/*for(int weightIdx = 0; weightIdx < scene->mMeshes[meshIdx]->mBones[boneIdx]->mNumWeights; weightIdx++)
					{
						aiVertexWeight aiWeight = scene->mMeshes[meshIdx]->mBones[boneIdx]->mWeights[weightIdx];
						Weight weight;
						weight.vertexID = aiWeight.mVertexId;
						weight.weighting = aiWeight.mWeight;

						bone->weights.push_back(weight);
					}*/
					#pragma endregion

					bone->parent = parent;
					bone->parent->children.push_back(bone);
					
					bone->offset = convertAssimpMatrix(scene->mMeshes[meshIdx]->mBones[boneIdx]->mOffsetMatrix);
					bone->inv_offset = glm::inverse(bone->offset);

					bone->transform = convertAssimpMatrix(aiBone->mTransformation);

					root = bone->parent; //The last guy to get in here is the root, as it is depth first

					bone->applyKeyframeFlag = false;

					#pragma region PRINT OUT
					if(print)
					{
						glm::vec3 offsetTranslation = decomposeT(bone->offset);
						glm::vec3 mTransformTranslation = decomposeT(convertAssimpMatrix(aiBone->mTransformation));

						std::stringstream ss;
						ss << "\nYES!";
						ss << "\nbone->name: " << bone->name;
						ss << "\nbone->parent: " << bone->parent->name;
						ss << "\nnumberOfChildren: " << bone->children.size();
						ss << "\nbone->offset: (x:" << offsetTranslation.x << ", y: " << offsetTranslation.y << ", z: " << offsetTranslation.z << ")";
						ss << "\nmTransform: (x:" << mTransformTranslation.x << ", y: " << mTransformTranslation.y << ", z: " << mTransformTranslation.z << ")";
						std::cout << ss.str();
					}
					#pragma endregion
					
					
					break; 
				}
			}
		}
	}

	return true;
}

#pragma region DEBUGGING PRINTOUTS
void Skeleton::PrintHeirarchy(Bone* bone)
{
	printf("\nbone->name: %s \n", bone->name);
	printf("bone->parent: %i \n", bone->parent->name);

	printf("numberOfChildren: %i \n", bone->children.size());

	for(int i = 0; i < bone->children.size(); i++)
		PrintHeirarchy(bone->children[i]);
}

void Skeleton::PrintAiHeirarchy(aiNode* bone)
{
	printf("\nbone->name: %s \n", bone->mName.C_Str());
	printf("bone->parent: %i \n", bone->mParent->mName.C_Str());

	printf("numberOfChildren: %i \n", bone->mNumChildren);

	for(int i = 0; i < bone->mNumChildren; i++)
		PrintAiHeirarchy(bone->mChildren[i]);
}
#pragma endregion

void Skeleton::UpdateGlobalTransforms(Bone* bone, glm::mat4 parentTransform) 
{	
	bone->globalTransform = parentTransform * bone->transform;
	
	bone->finalTransform = bone->globalTransform * bone->offset;

	for (int i = 0; i < bone->children.size(); i++) 
		UpdateGlobalTransforms (bone->children[i], bone->globalTransform); 
}

void Skeleton::Animate(double deltaTime)
{
	if(!hasKeyframes)
		return;

	if(blender.active)
		blender.Blend(deltaTime);

	for(int aniIdx = 0; aniIdx < animations.size(); aniIdx++)
	{
		Animation* animation = animations[aniIdx];

		if(!animation->frozen)
			animation->localClock += deltaTime/1000;

		if (animation->localClock >= animation->duration) 
			animation->localClock = 0.0;

		if (animation->weight > 0)
		{
			for(int boneidx = 0; boneidx < animation->boneAnimations.size(); boneidx++)
			{
				glm::mat4 translation = glm::mat4(1);
				glm::mat4 orientation = glm::mat4(1);

				BoneAnimation* boneAnimation = animation->boneAnimations.at(boneidx);

				if(boneAnimation->posKeyframes.size() > 0)// if this bone has keyframes
				{
					int prev_key = 0;
					int next_key = 0;

					//Find the two keyframes
					for (int keyidx = 0; keyidx < boneAnimation->posKeyframes.size() - 1; keyidx++) 
					{
						prev_key = keyidx;
						next_key = keyidx + 1;

						if (boneAnimation->posKeyframes[next_key]->time >= animation->localClock) // if the next keyframe is greater than the timer, then we have our two keyframes
							break;
					}

					float timeBetweenKeys = boneAnimation->posKeyframes[next_key]->time - boneAnimation->posKeyframes[prev_key]->time;
					float t = (animation->localClock - boneAnimation->posKeyframes[prev_key]->time) / timeBetweenKeys;

					std::stringstream ss;
					ss << "t: "<< t;
					drawText(20,20, ss.str().c_str());

					translation = glm::translate(glm::mat4(1), lerp(boneAnimation->posKeyframes[prev_key]->position, 
						boneAnimation->posKeyframes[next_key]->position, t));
					//translation = glm::translate(glm::mat4(1), animation.boneAnimations[boneidx].posKeyframes[prev_key]->position); //TODO - add mode
				}

				if (boneAnimation->rotKeyframes.size() > 0)  // if this bone has keyframes
				{
					int prev_key = 0;
					int next_key = 0;
		
					//Find the two keyframes
					for (int keyidx = 0; keyidx < boneAnimation->rotKeyframes.size() - 1; keyidx++) 
					{
						prev_key = keyidx;
						next_key = keyidx + 1;

						if (boneAnimation->rotKeyframes[next_key]->time >= animation->localClock) // if the next keyframe is greater than the timer, then we have our two keyframes
							break;
					}

					float timeBetweenKeys = boneAnimation->rotKeyframes[next_key]->time - boneAnimation->rotKeyframes[prev_key]->time;
					float t = (animation->localClock - boneAnimation->rotKeyframes[prev_key]->time) / timeBetweenKeys;
	
					glm::quat interpolatedquat = glm::slerp(boneAnimation->rotKeyframes[prev_key]->rotation, 
						boneAnimation->rotKeyframes[next_key]->rotation, t);
					orientation = glm::toMat4(interpolatedquat);
					//orientation = glm::toMat4(animation.boneAnimations[boneidx].rotKeyframes[prev_key]->rotation);
				}

				if(bones[boneAnimation->boneID]->applyKeyframeFlag == false)
				{
					bones[boneAnimation->boneID]->transform = glm::mat4(1);
					bones[boneAnimation->boneID]->applyKeyframeFlag = true;
				}

				bones[boneAnimation->boneID]->transform *= (translation * orientation) * animation->weight; 
			}
		}
	}

	for(int boneIdx = 0; boneIdx < bones.size(); boneIdx++)
		bones[boneIdx]->applyKeyframeFlag = false;
}

bool Skeleton::ComputeIK(std::string chainName, glm::vec3 T, int steps)
{
	float distanceThreshold = 0.01f;
	
	std::vector<Bone*> links = ikChains[chainName];

	int tries = 0;
	int maxTries = steps * (links.size() - 1); //so a try does one full iteration up the chain

	int effectorIdx = links.size()-1; //Effector is in the last position
	int linkIdx = effectorIdx - 1; //current link one up from effector

	glm::vec3 B, E; //These need to be world positions

	glm::mat4 modelMat = model->GetModelMatrix(); //Grab the model matrix out here as it won't be changing
	Bone* effector = links[effectorIdx];

	do
	{
		Bone* bone = links[linkIdx]; //Bone we're currently working on

		B = glm::vec3(modelMat * glm::vec4(bone->GetMeshSpacePosition(), 1));
		E = glm::vec3(modelMat * glm::vec4(effector->GetMeshSpacePosition(), 1));

		if(glm::distance(E, T) > distanceThreshold)
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
				rotationAxis = glm::mat3(glm::inverse(modelMat * bone->finalTransform * bone->inv_offset)) * rotationAxis;

				rotation = glm::rotate(glm::mat4(1), turnAngle, rotationAxis);

				bone->transform *= rotation;

				if(ConstraintsEnabled)
					ImposeDOFRestrictions(bone);

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

void Skeleton::ImposeDOFRestrictions(Bone* bone)
{
	glm::vec3 translation;
	glm::mat4 rotation; 
	glm::vec3 scaling;
	
	decomposeTRS(bone->transform, translation, rotation, scaling); 

/*      [0] [1] [2] [3]
 * [0] | 1   0   0   T1 | | R11 R12 R13 0 | | a 0 0 0 |   | aR11 bR12 cR13 T1 |
 * [1] | 0   1   0   T2 |.| R21 R22 R23 0 |.| 0 b 0 0 | = | aR21 bR22 cR23 T2 |
 * [2] | 0   0   0   T3 | | R31 R32 R33 0 | | 0 0 c 0 |   | aR31 bR32 cR33 T3 |
 * [3] | 0   0   0   1  | |  0   0   0  1 | | 0 0 0 1 |   |  0    0    0    1 |*/

	glm::vec3 euler = glm::eulerAngles(glm::toQuat(rotation));
	
	if(bone->dofLimits.xAxis) 
	{
		if(euler.x > 180.0f)
			euler.x -= 360.0f;

		euler.x = glm::clamp(euler.x, bone->dofLimits.xMin, bone->dofLimits.xMax);
	}
		
	if(bone->dofLimits.yAxis) 
	{
		if(euler.y > 180.0f)
			euler.y -= 360.0f;
		euler.y = glm::clamp(euler.y, bone->dofLimits.yMin, bone->dofLimits.yMax);
	}
		
	if(bone->dofLimits.zAxis) 
	{
		if(euler.z > 180.0f)
			euler.z -= 360.0f;
		euler.z = glm::clamp(euler.z, bone->dofLimits.zMin, bone->dofLimits.zMax);
	}

	glm::mat4 copy = rotation;
	
    euler = glm::radians(euler);
	rotation = glm::eulerAngleZ(euler.z) * glm::eulerAngleY(euler.y) * glm::eulerAngleX(euler.x);

	bone->transform = glm::translate(glm::mat4(1.0f), translation) 
				* rotation
				* glm::scale(glm::mat4(1.0f), scaling);
}

bool Skeleton::LoadAnimation(const char* file_name)
{
	const aiScene* scene = aiImportFile (file_name, aiProcess_Triangulate | aiProcess_FlipUVs);
	
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
	{
		fprintf (stderr, "ERROR: reading animation %s\n", file_name);
		return false;
	}

	if(scene->HasAnimations())
	{
		hasKeyframes = true;

		aiAnimation* anim = scene->mAnimations[0];

		/*printf ("animation name: %s\n", anim->mName.C_Str ());
		printf ("animation has %i node channels\n", anim->mNumChannels);
		printf ("animation has %i mesh channels\n", anim->mNumMeshChannels);
		printf ("animation duration %f\n", anim->mDuration);
		printf ("ticks per second %f\n", anim->mTicksPerSecond);*/

		Animation* animation = new Animation(animations.size(), anim->mDuration);
		///animationDuration = anim->mDuration;

		//printf ("anim duration is %f\n", anim->mDuration);
			
		// get the node channels
		for (int i = 0; i < (int)anim->mNumChannels; i++) 
		{
			aiNodeAnim* chan = anim->mChannels[i];
			Bone* bone = GetBone(chan->mNodeName.C_Str()); //Grab the bone
	
			if (!bone) 
			{
				fprintf (stderr, "\nWARNING: did not find node named %s in skeleton."
					"animation broken.\n", chan->mNodeName.C_Str ());
				continue;
			}

			BoneAnimation* boneAnimation = new BoneAnimation();
			boneAnimation->boneID = bone->id;

			// add position keys to node
			for (int i = 0; i < chan->mNumPositionKeys; i++) 
			{
				aiVectorKey key = chan->mPositionKeys[i];

				PosKeyFrame* pkf = new PosKeyFrame;

				pkf->position = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
				pkf->time = key.mTime; //TODO - check if time varies for each variable??

				boneAnimation->posKeyframes.push_back(pkf);
			}

			// add rotation keys to node
			for (int i = 0; i < chan->mNumRotationKeys; i++) 
			{
				aiQuatKey key = chan->mRotationKeys[i];

				RotKeyFrame* rkf = new RotKeyFrame;

				rkf->rotation.x = key.mValue.x;
				rkf->rotation.y = key.mValue.y;
				rkf->rotation.z = key.mValue.z;
				rkf->rotation.w = key.mValue.w;

				rkf->time = key.mTime;

				boneAnimation->rotKeyframes.push_back(rkf);
			}

			// add scaling keys to node
			//for (int i = 0; i < sn->num_sca_keys; i++) 
			//{
			//	aiVectorKey key = chan->mScalingKeys[i];
			//	sn->sca_keys[i].v[0] = key.mValue.x;
			//	sn->sca_keys[i].v[1] = key.mValue.y;
			//	sn->sca_keys[i].v[2] = key.mValue.z;
			//	sn->sca_key_times[i] = key.mTime;
			//}

			animation->boneAnimations.push_back(boneAnimation);
		} 

		animations.push_back(animation);
	}
	else 
	{
		fprintf (stderr, "WARNING: no animations found in mesh file\n");
	}

	aiReleaseImport (scene);
	return true;
}

void Skeleton::PrintOuts(int winw, int winh)
{
	int amountActive = 0;

	for(int i = 0; i < animations.size(); i++)
	{
		if(animations[i]->weight > 0)
		{	
			int offset = amountActive*100 + amountActive*20;
			amountActive++;

			std::stringstream ss;
			ss.str(std::string()); // clear
			ss << "AnimationID: " << animations[i]->animationID; 
			drawText(20, 100 + offset, ss.str().c_str());

			ss.str(std::string()); // clear
			ss << "Duration: " << animations[i]->duration; 
			drawText(20, 80 + offset, ss.str().c_str());

			ss.str(std::string()); // clear
			ss << "Frozen: " << animations[i]->frozen;
			drawText(20, 60 + offset, ss.str().c_str());

			ss.str(std::string()); // clear
			ss << "LocalTimer: " << animations[i]->localClock; 
			drawText(20, 40 + offset, ss.str().c_str());

			ss.str(std::string()); // clear
			ss << "Weight: " << animations[i]->weight;
			drawText(20, 20 + offset, ss.str().c_str());
		}
	}

	if(blender.active)
	{
		int offset = amountActive*100 + amountActive*20;

		std::stringstream ss;
		ss.str(std::string()); // clear
		ss << "Blend Duration: " << blender.blendDuration;
		drawText(20, 60 + offset, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "Blend Timer: " << blender.blendTimer;
		drawText(20, 40 + offset, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "t: " << blender.blendTimer / blender.blendDuration;
		drawText(20, 20 + offset, ss.str().c_str());
	}


}
