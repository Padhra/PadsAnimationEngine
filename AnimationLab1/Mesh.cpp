#include "Mesh.h"

Mesh::Mesh(glm::vec3 position, glm::vec3 rotationAxis, float rotationDegrees, glm::vec3 scale, const char* file_name)
{
	worldProperties.translation = position;
	worldProperties.rotationAxis = rotationAxis;
	worldProperties.rotationDegrees = rotationDegrees;
	worldProperties.scale = scale;

	glGenVertexArrays (1, &vao);
	vertexCount = 0;

	Load(file_name);
}

Mesh::~Mesh()
{
	
}

bool Mesh::Load(const char* file_name)
{
	scene = aiImportFile (file_name, aiProcess_Triangulate);
	
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		fprintf (stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	else {
		
	}
			
	printf ("  %i animations\n", scene->mNumAnimations);
	printf ("  %i cameras\n", scene->mNumCameras);
	printf ("  %i lights\n", scene->mNumLights);
	printf ("  %i materials\n", scene->mNumMaterials);
	printf ("  %i textures\n", scene->mNumTextures);
	printf ("  %i meshes\n", scene->mNumMeshes);
	
	/* get first mesh in file only */
	mesh = scene->mMeshes[0];

	printf ("    %i vertices in mesh[0]\n", mesh->mNumVertices);
	printf ("    %i UV components in mesh[0]\n", mesh->mNumUVComponents);
	printf ("    %i anim meshes in mesh[0]\n", mesh->mNumAnimMeshes);
	printf ("    %i bones in mesh[0]\n", mesh->mNumBones);
	printf ("    %i faces in mesh[0]\n", mesh->mNumFaces);
	
	vertexCount = mesh->mNumVertices;
	
	//glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);

	//BUFFER THE DATA
	glGenBuffers(NUM_VBs, buffers);

	if (mesh->HasPositions ())
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, 3 * vertexCount * sizeof (GLfloat), &mesh->mVertices[0], GL_STATIC_DRAW);
   
		glEnableVertexAttribArray(POS_VB);
		glVertexAttribPointer(POS_VB, 3, GL_FLOAT, GL_FALSE, 0, 0);    
	}
	
	if (mesh->HasNormals ()) 
	{
		glBindBuffer (GL_ARRAY_BUFFER, buffers[NORMAL_VB]);
		glBufferData (GL_ARRAY_BUFFER, 3 * vertexCount * sizeof (GLfloat), &mesh->mNormals[0], GL_STATIC_DRAW);
		
		glVertexAttribPointer (NORMAL_VB, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (NORMAL_VB);
	}
	
	if (mesh->HasTextureCoords (0)) 
	{
		glBindBuffer (GL_ARRAY_BUFFER, buffers[TEXCOORD_VB]);
		glBufferData (GL_ARRAY_BUFFER, 2 * vertexCount * sizeof (GLfloat), &mesh->mTextureCoords[0], GL_STATIC_DRAW);
		
		glVertexAttribPointer (TEXCOORD_VB, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (TEXCOORD_VB);
	}

	if (mesh->HasBones())
	{
		aiNode* aiRoot = scene->mRootNode;

		skeleton = new Skeleton(mesh);

		printf ("\nBoneHierarchy\n");

		if (!skeleton->ImportBoneHierarchy(aiRoot, (Bone*)malloc (sizeof (Bone)), nullptr))
			fprintf (stderr, "\nERROR: could not import node tree from mesh\n");

		//vector<VertexBoneData> vertexBoneData;
		//vertexBoneData.resize(mesh->mNumVertices);

		/*vector<glm::vec4> boneIDs;
		boneIDs.resize(mesh->mNumVertices);*/
		/*vector<glm::vec4> weightAmounts;
		weightAmounts.resize(mesh->mNumVertices);*/

		//vertexBoneIDs = (float*)malloc (mesh->mNumVertices * sizeof (float)); 

		glm::vec4* boneIDs = (glm::vec4*)malloc (mesh->mNumVertices * 4 * sizeof(float));
		glm::vec4* weightAmounts = (glm::vec4*)malloc (mesh->mNumVertices * 4 * sizeof(float));

		//default weight amounts to 0
		for(int v = 0; v < mesh->mNumVertices; v++)
			for(int w = 0; w < 4; w++)
				weightAmounts[v][w] = 0.0f;

		for (int i = 0; i < (int)mesh->mNumBones; i++) 
		{
			const aiBone* bone = mesh->mBones[i];

			for (int j = 0; j < (int)bone->mNumWeights; j++) 
			{
				aiVertexWeight weight = bone->mWeights[j];
				
				//if (weight.mWeight >= 0.5f) //ignore weight if less than 0.5 factor
					//boneIDs[weight.mVertexId][0] = i; //vertexBoneID for this weight

				//if (weight.mWeight > 0.0f) //ignore weight if 0
					//vertexBoneData[(int)weight.mVertexId].AddBoneData(i, weight.mWeight);

				//if(weight.mWeight > 0.0f)
				//{
					for(int k = 0; k < 4; k++)
					{
						if(weightAmounts[weight.mVertexId][k] == 0.0f)
						{
							boneIDs[weight.mVertexId][k] = i;
							weightAmounts[weight.mVertexId][k] = weight.mWeight;
						}
					}
				//}*/
			}
		}

		/*glBindBuffer (GL_ARRAY_BUFFER, buffers[BONE_VB]);
		glBufferData (GL_ARRAY_BUFFER, vertexCount * sizeof (GLfloat), vertexBoneIDs, GL_STATIC_DRAW);
		glVertexAttribPointer (BONE_VB, 1, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (BONE_VB);*/

		glBindBuffer (GL_ARRAY_BUFFER, buffers[BONE_VB]);
		glBufferData (GL_ARRAY_BUFFER, 4 * vertexCount * sizeof (GLfloat), &boneIDs[0], GL_STATIC_DRAW);
		glVertexAttribPointer (BONE_VB, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (BONE_VB);

		glBindBuffer (GL_ARRAY_BUFFER, buffers[WEIGHT_VB]);
		glBufferData (GL_ARRAY_BUFFER, 4 * vertexCount * sizeof (GLfloat), &weightAmounts[0], GL_STATIC_DRAW);
		glVertexAttribPointer (WEIGHT_VB, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (WEIGHT_VB);
	}

	// Populate the index buffer
	if(mesh->HasFaces())
	{
		for (int i = 0 ; i < mesh->mNumFaces ; i++) 
		{
			const aiFace& Face = mesh->mFaces[i];
			assert(Face.mNumIndices == 3);
			
			Indices.push_back(Face.mIndices[0]);
			Indices.push_back(Face.mIndices[1]);
			Indices.push_back(Face.mIndices[2]);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_VB]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
	}

	if(scene->HasAnimations())
		printf("hello");

	aiReleaseImport (scene);
	printf ("\nMesh loaded.\n");
	
	return true;
}