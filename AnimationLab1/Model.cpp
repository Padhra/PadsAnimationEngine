#include "Model.h"

Model::Model(glm::vec3 position, glm::mat4 orientation, glm::vec3 scale, const char* file_name, GLuint p_shaderProgramID)
{
	hasSkeleton = false;

	worldProperties.translation = position;
	worldProperties.orientation = orientation;
	worldProperties.scale = scale;

	glGenVertexArrays (1, &vao);
	vertexCount = 0;

	Load(file_name);

	shaderProgramID = p_shaderProgramID;
}

Model::~Model()
{
	
}

bool Model::Load(const char* file_name)
{
	const aiScene* scene = aiImportFile (file_name, aiProcess_Triangulate);
	
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
	{
		fprintf (stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	
	printf("LOADING MODEL...\n");
	printf("%i animations\n", scene->mNumAnimations);
	printf("%i cameras\n", scene->mNumCameras);
	printf("%i lights\n", scene->mNumLights);
	printf("%i materials\n", scene->mNumMaterials);
	printf("%i textures\n", scene->mNumTextures);
	printf("%i meshes\n", scene->mNumMeshes);
	
	//TODO - multiple mesh support
	//const aiMesh* mesh = scene->mMeshes[1];

	
	
	//vertexCount = mesh->mNumVertices;
	
	//glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	GLuint *buffers = new GLuint [NUM_VBs];
	
	//1. Grab all the data from the submeshes
	vector<glm::vec3> positions;
	vector<glm::vec3> normals;
	vector<glm::vec2> texcoords;

	bool modelHasBones = false;

	int indexCount = 0;

	for(int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++)
	{
		aiMesh* mesh = scene->mMeshes[meshIdx];
		printf("LOADING MESH[%i]\n", meshIdx);
		printf("    %i vertices\n", mesh->mNumVertices);
		printf("    %i UV components\n", mesh->mNumUVComponents);
		printf("    %i anim meshes\n", mesh->mNumAnimMeshes);
		printf("    %i bones\n", mesh->mNumBones);
		printf("    %i faces\n", mesh->mNumFaces);
		printf("	%i normals\n", mesh->HasNormals());

		MeshEntry meshEntry;
		meshEntry.BaseIndex = indexCount;
		meshEntry.BaseVertex = vertexCount;
		meshEntry.NumIndices = mesh->mNumFaces * 3;
		MeshEntries.push_back(meshEntry);

		vertexCount += mesh->mNumVertices;
		indexCount += meshEntry.NumIndices;

		for(int vertIdx = 0; vertIdx < mesh->mNumVertices; vertIdx++)
		{
			if (mesh->HasPositions ())
				positions.push_back(glm::vec3(mesh->mVertices[vertIdx].x, mesh->mVertices[vertIdx].y, mesh->mVertices[vertIdx].z)); 
			if (mesh->HasNormals ())
				normals.push_back(glm::vec3(mesh->mNormals[vertIdx].x, mesh->mNormals[vertIdx].y, mesh->mNormals[vertIdx].z));
			if (mesh->HasTextureCoords (0)) 
				texcoords.push_back(glm::vec2(mesh->mTextureCoords[0][vertIdx].x, mesh->mTextureCoords[0][vertIdx].y));
		}

		if(mesh->HasFaces())
		{
			for (int i = 0 ; i < mesh->mNumFaces ; i++) 
			{
				const aiFace& Face = mesh->mFaces[i];
				assert(Face.mNumIndices == 3);
			
				indices.push_back(Face.mIndices[0]);
				indices.push_back(Face.mIndices[1]);
				indices.push_back(Face.mIndices[2]);
			}
		}

		if(mesh->HasBones())
			modelHasBones = true;
	}

	//2. BUFFER THE DATA
	glGenBuffers(NUM_VBs, buffers);

	if(positions.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, /*3 * vertexCount * sizeof (GLfloat)*/sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
   
		glEnableVertexAttribArray(POS_VB);
		glVertexAttribPointer(POS_VB, 3, GL_FLOAT, GL_FALSE, 0, 0);  
	}

	if(normals.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_VB]);
		glBufferData(GL_ARRAY_BUFFER, 3 * vertexCount * sizeof (GLfloat), &normals[0], GL_STATIC_DRAW);
   
		glEnableVertexAttribArray(NORMAL_VB);
		glVertexAttribPointer(NORMAL_VB, 3, GL_FLOAT, GL_FALSE, 0, 0);  
	}
	
	if (texcoords.size() > 0) 
	{
		glBindBuffer (GL_ARRAY_BUFFER, buffers[TEXCOORD_VB]);
		glBufferData (GL_ARRAY_BUFFER, 2 * vertexCount * sizeof (GLfloat), &texcoords[0], GL_STATIC_DRAW);
		
		glVertexAttribPointer (TEXCOORD_VB, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (TEXCOORD_VB);
	}

	if (indices.size() > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_VB]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
	}

	if (modelHasBones)
	{
		aiMesh* mesh = scene->mMeshes[0];

		skeleton = new Skeleton(this, mesh, scene->HasAnimations());
		hasSkeleton = true;

		//skeleton->globalInverse = convert_assimp_matrix(scene->mRootNode->mTransformation.Inverse());

		printf ("\nBoneHierarchy\n");
		
		/*if (!*/skeleton->ImportAssimpBoneHierarchy(scene, scene->mRootNode, nullptr);//)
			//fprintf (stderr, "\nERROR: could not import node tree from mesh\n");

		//std::string roon = skeleton->root->name;
		//skeleton->PrintHeirarchy(skeleton->root);

		/*vector<glm::vec4> boneIDs;
		boneIDs.resize(mesh->mNumVertices);*/
		/*vector<glm::vec4> weightAmounts;
		weightAmounts.resize(mesh->mNumVertices);*/

		glm::vec4* boneIDs = (glm::vec4*)malloc (vertexCount * 4 * sizeof(float));
		glm::vec4* weightAmounts = (glm::vec4*)malloc (vertexCount * 4 * sizeof(float));

		//default weight amounts to 0
		for(int v = 0; v < vertexCount; v++)
		{
			for(int w = 0; w < 4; w++)
			{
				weightAmounts[v][w] = 0.0f;
				boneIDs[v][w] = 0.0f;
			}
		}


		/*for (int boneIdx = 0; boneIdx < skeleton->GetBones().size(); boneIdx++) 
		{
			const aiBone* bone = skeleton->GetBone(boneIdx)->bone;;

			for (int j = 0; j < (int)bone->mNumWeights; j++) 
			{
				aiVertexWeight weight = bone->mWeights[j];
				
				for(int k = 0; k < 4; k++)
				{
					if(weightAmounts[weight.mVertexId][k] == 0.0f)
					{
						boneIDs[weight.mVertexId][k] = boneIdx;
						weightAmounts[weight.mVertexId][k] = weight.mWeight;
					}
				}
			}
		}*/

		for (int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
		{
			for(int boneIdx = 0; boneIdx < scene->mMeshes[meshIndex]->mNumBones; boneIdx++)
			{
				const aiBone* bone = scene->mMeshes[meshIndex]->mBones[boneIdx]; //For every bone in the model

				for (int j = 0; j < (int)bone->mNumWeights; j++) //loop through its weights
				{
					aiVertexWeight weight = bone->mWeights[j];

					int vertID = MeshEntries[meshIndex].BaseVertex + weight.mVertexId; //the vertex id + offset 
				
					for(int k = 0; k < 4; k++)
					{
						if(weightAmounts[vertID][k] == 0.0f)
						{
							GLfloat id = skeleton->GetBone(bone->mName.C_Str())->id;

							boneIDs[vertID][k] = id;
							weightAmounts[vertID][k] = weight.mWeight;
						}
					}
				}
			}
		}
		

		glBindBuffer (GL_ARRAY_BUFFER, buffers[BONE_VB]);
		glBufferData (GL_ARRAY_BUFFER, 4 * vertexCount * sizeof (GLfloat), &boneIDs[0], GL_STATIC_DRAW);
		glVertexAttribPointer (BONE_VB, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (BONE_VB);

		glBindBuffer (GL_ARRAY_BUFFER, buffers[WEIGHT_VB]);
		glBufferData (GL_ARRAY_BUFFER, 4 * vertexCount * sizeof (GLfloat), &weightAmounts[0], GL_STATIC_DRAW);
		glVertexAttribPointer (WEIGHT_VB, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (WEIGHT_VB);
	}

	if(scene->HasAnimations())
	{
		//TODO- multiple animation support
		aiAnimation* anim = scene->mAnimations[0];

		/*printf ("animation name: %s\n", anim->mName.C_Str ());
		printf ("animation has %i node channels\n", anim->mNumChannels);
		printf ("animation has %i mesh channels\n", anim->mNumMeshChannels);
		printf ("animation duration %f\n", anim->mDuration);
		printf ("ticks per second %f\n", anim->mTicksPerSecond);*/

		skeleton->SetAnimDuration(anim->mDuration);

		//printf ("anim duration is %f\n", anim->mDuration);
			
		// get the node channels
		for (int i = 0; i < (int)anim->mNumChannels; i++) 
		{
			aiNodeAnim* chan = anim->mChannels[i];
			
			Bone* bone = skeleton->GetBone(chan->mNodeName.C_Str ());
				
			if (!bone) 
			{
				fprintf (stderr, "\nWARNING: did not find node named %s in skeleton."
					"animation broken.\n", chan->mNodeName.C_Str ());
				continue;
			}

			// add position keys to node
			for (int i = 0; i < chan->mNumPositionKeys; i++) 
			{
				aiVectorKey key = chan->mPositionKeys[i];

				PosKeyFrame* pkf = new PosKeyFrame;

				pkf->position = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
				pkf->time = key.mTime; //TODO - check if time varies for each variable??

				bone->posKeyframes.push_back(pkf);
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

				bone->rotKeyframes.push_back(rkf);
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
		} 
	}
	else 
	{
		fprintf (stderr, "WARNING: no animations found in mesh file\n");
	}

	aiReleaseImport (scene);
	printf ("\nMesh loaded.\n");

	delete[] buffers;
	
	return true;
}