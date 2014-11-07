#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ShaderManager.h"
#include "Spline.h"

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <map>
#include <vector>
#include <list>

#define MESH_FILE1 "Models/hand_with_animation.dae"
#define MESH_FILE2 "Models/hand.dae"
#define MESH_FILE3 "Models/cubeTri.obj"
#define MESH_FILE4 "Models/boblampclean.md5mesh"
#define MESH_FILE5 "Models/Cones3.dae"

#define PRECISION 3

using namespace std;

void processInput();
void printouts();

//Callbacks
void keyPressed (unsigned char key, int x, int y); 
void keyUp (unsigned char key, int x, int y); 
void passiveMouseMotion(int x, int y);
void update();
void draw();

bool keyStates[256] = {false}; // Create an array of boolean values of length 256 (0-255)

Camera camera;
glm::mat4 projectionMatrix; // Store the projection matrix

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

int oldTimeSinceStart;
double deltaTime;

int fps = 0;
int frameCounterTime = 0;
int frames = 0;
char *text;

ShaderManager shaderManager;
vector<Model*> objectList;

Model* target;
float targetSpeed = 0.01f;

Spline targetPath;

int testAnimBone = 0;

GLuint line_vao;

std::vector<glm::vec3> lines;

int main(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition (100, 100); 
    glutCreateWindow("Directed by M. Pat Mahalaralan");

	glutSetCursor(GLUT_CURSOR_NONE);
	
	// REGISTER GLUT CALLBACKS

	glutDisplayFunc(draw);
	//glutReshapeFunc (reshape);
	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events  
	//glutMouseFunc (MouseButton);
	//glutMotionFunc (MouseMotion);
	glutPassiveMotionFunc(passiveMouseMotion);
	glutIdleFunc (update);

	// A call to glewInit() must be done after glut is initialized!
    GLenum res = glewInit();
	
	#pragma region ERROR CHECKING
	// Check for any errors
    if (res != GLEW_OK) 
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
    }
	#pragma endregion 

	projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

	camera.Init(); //TODO - constructor for camera
	camera.viewProperties.position = glm::vec3(0.0f, 0.0f, 0.0f);
	//camera.viewProperties.forward = glm::vec3(0.f, 0.f, -10.f); //horizontal and vertical control this
	camera.viewProperties.up = glm::vec3(0.0f, 1.0f, 0.0f);

	glClearColor(.5,.5,.5,1);
	glEnable (GL_CULL_FACE); // cull face 
	glCullFace (GL_BACK); // cull back face 
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_DEPTH_TEST);

	shaderManager.CreateShaderProgram("skinned", "Shaders/skinned.vs", "Shaders/skinned.ps");
	shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");

	//LINE
	glGenVertexArrays(1, &line_vao);
	glBindVertexArray (line_vao);
	
	lines.push_back(glm::vec3(0,0,0));
	lines.push_back(glm::vec3(0,100,0));

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer (GL_ARRAY_BUFFER, buf);
	glBufferData (GL_ARRAY_BUFFER, 3 * lines.size() * sizeof (GLfloat), &lines[0], GL_STATIC_DRAW);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);

	int index;

	//KEYFRAMED HAND
	//objectList.push_back(new Model(glm::vec3(0,0,0), glm::mat4(1), glm::vec3(1), MESH_FILE1, shaderManager.GetShaderProgramID("skinned")));
	
	//BOB
	glm::quat q = glm::quat();
		
	q *= glm::angleAxis(-90.0f, glm::vec3(1,0,0));
	q *= glm::angleAxis(270.0f, glm::vec3(0,1,0));
	q *= glm::angleAxis(0.0f, glm::vec3(0,0,1));

	objectList.push_back(new Model(glm::vec3(-5,0,0), glm::toMat4(q), glm::vec3(.1), MESH_FILE4, shaderManager.GetShaderProgramID("skinned")));
	index = objectList.size()-1;
	
	std::vector<Bone*> chain; //just name end effector and number of links to go back!!!!
	
	Bone* fingersL = objectList[index]->GetSkeleton()->GetBone("fingers.L");
	//fingersL->dofLimits.SetXLimits(-90, 18);
	//fingersL->dofLimits.SetYLimits(0,0);
	chain.push_back(fingersL); 
	
	Bone* wristL = fingersL->parent;
	//wristL->dofLimits.SetXLimits(-51, 68);
	//wristL->dofLimits.SetYLimits(0, 0);
	//wristL->dofLimits.SetZLimits(-20, 20);
	chain.push_back(wristL); 

	Bone* forearmL = wristL->parent;
	//forearmL->dofLimits.SetXLimits(0, 0);
	//forearmL->dofLimits.SetZLimits(0, 114);
	chain.push_back(forearmL); 

	Bone* upperArmL = forearmL->parent;
	//upperArmL->dofLimits.SetXLimits(-60, -40);
	//upperArmL->dofLimits.SetYLimits(0, 0);
	//upperArmL->dofLimits.SetZLimits(-80, 50);
	chain.push_back(upperArmL); 
	
	/*for(int i = 0; i < 3; i++)
		chain.push_back(chain[chain.size()-1]->parent);*/
	std::reverse(chain.begin(),chain.end());
	objectList.at(index)->GetSkeleton()->DefineIKChain("chain1", chain);
	
	//CONES
	q = glm::angleAxis(-90.0f, glm::vec3(1,0,0));
	objectList.push_back(new Model(glm::vec3(5,0,0), glm::toMat4(q), glm::vec3(1), MESH_FILE5, shaderManager.GetShaderProgramID("skinned")));
	index = objectList.size()-1;

	std::vector<Bone*> coneChain;

	coneChain.push_back(objectList[index]->GetSkeleton()->GetBones()[3]); 
	coneChain.push_back(objectList[index]->GetSkeleton()->GetBones()[2]); 
	coneChain.push_back(objectList[index]->GetSkeleton()->GetBones()[1]);
	coneChain.push_back(objectList[index]->GetSkeleton()->GetBones()[0]); //effector
	objectList.at(index)->GetSkeleton()->DefineIKChain("chain1", coneChain);

	//TARGET
	target = new Model(glm::vec3(0,0,5), glm::mat4(1), glm::vec3(.1), MESH_FILE3, shaderManager.GetShaderProgramID("diffuse"));
	objectList.push_back(target);

	targetPath.AddKeyframe(-10,		glm::vec3(-5,0,0));
	targetPath.AddKeyframe(0,		glm::vec3(5,2,0));
	targetPath.AddKeyframe(10,		glm::vec3(-5,4,0));
	targetPath.AddKeyframe(20,		glm::vec3(5,6,0));
	targetPath.AddKeyframe(30,		glm::vec3(-5,8,0));
	targetPath.AddKeyframe(40,		glm::vec3(5,10,0));
	targetPath.AddKeyframe(50,		glm::vec3(-5,12,0));
	targetPath.AddKeyframe(60,		glm::vec3(5,14,0));
	targetPath.AddKeyframe(70,		glm::vec3(-5,16,0));

	glutMainLoop();
    
	return 0;
}

// GLUT CALLBACK FUNCTIONS
void update()
{
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = timeSinceStart - oldTimeSinceStart;
    oldTimeSinceStart = timeSinceStart;

	frames++;
	frameCounterTime += deltaTime;
	if(frameCounterTime > 1000)
	{
		fps = frames;
		frames = frameCounterTime = 0;
	}

	//Animation
	for(int i = 0; i< objectList.size(); i++)
	{
		if(objectList[i]->HasSkeleton())
		{
			int numbones = objectList[i]->GetSkeleton()->GetBones().size();
			int testAnimMod = testAnimBone % numbones;
			Bone* bone = objectList[i]->GetSkeleton()->GetBones()[testAnimMod];

			if(keyStates['t'])
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 10.0f, glm::vec3(1,0,0));
			else if(keyStates['y'])
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 10.0f, glm::vec3(-1,0,0));

			if(keyStates['g'])
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 10.0f, glm::vec3(0,1,0));
			else if(keyStates['h'])
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 10.0f, glm::vec3(0,-1,0));

			if(keyStates['v'])
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 10.0f, glm::vec3(0,0,1));
			else if(keyStates['b'])
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 10.0f, glm::vec3(0,0,-1));

			if(objectList[i]->GetSkeleton()->ikChains.size() > 0)
				objectList[i]->GetSkeleton()->ComputeIK("chain1", /*glm::vec3(0,5,0)*/target->worldProperties.translation, 50); //replace with iteration, ikchain should be a struct with a target?
																													//if no target do nothing?
			if(objectList[i]->GetSkeleton()->hasKeyframes)
				objectList[i]->GetSkeleton()->Animate(deltaTime); //this overwrites control above
		}
	}

	//FOR ITERATING SUBCHAINS
	//std::map<char,int>::iterator it;
	//for (std::map<char,int>::iterator it=mymap.begin(); it!=mymap.end(); ++it)
	//std::cout << it->first << " => " << it->second << '\n';

	

	//targetPath.Update(deltaTime);
	//target->worldProperties.translation = targetPath.GetPosition();

	processInput();
	draw();
}

//Draw loops through each 3d object, and switches to the correct shader for that object, and full the uniform matrices with the up-to-date values,
//before finally binding the VAO and drawing with verts or indices
void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glPolygonMode(GL_FRONT, GL_LINE); //Wireframe mode

	glm::mat4 viewMatrix = glm::lookAt(camera.viewProperties.position, camera.viewProperties.position + camera.viewProperties.forward, camera.viewProperties.up); 

	for(int i = 0; i < objectList.size(); i++)
	{
		shaderManager.SetShaderProgram(objectList[i]->GetShaderProgramID());

		glm::mat4 MVP = projectionMatrix * viewMatrix * objectList.at(i)->GetModelMatrix();

		int mvpMatrixLocation = glGetUniformLocation(objectList[i]->GetShaderProgramID(), "mvpMatrix"); // Get the location of our projection matrix in the shader
		glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVP/*&MVP[0][0]*/)); // Send our model/view/projection matrix to the shader
		
		if(objectList[i]->HasSkeleton())
		{
			int numBones = objectList[i]->GetSkeleton()->GetBones().size();

			glm::mat4 boneMatrices[MAX_BONES];
			int boneMatricesAttribLocations[MAX_BONES]; //INVESTIGATE - does this really need to be done every frame?

			for(int j = 0; j < MAX_BONES; j++)
				boneMatrices[j] = glm::mat4(1);

			objectList[i]->GetSkeleton()->UpdateGlobalTransforms(objectList[i]->GetSkeleton()->GetRootBone(), glm::mat4());

			for(int boneidx = 0; boneidx < numBones; boneidx++)
			{
				Bone* bone = objectList[i]->GetSkeleton()->GetBone(boneidx);
				boneMatrices[int(bone->id)] = bone->finalTransform;
			}

			//Send up-to-date bone matrix data to the shader 
			for(int j = 0; j < MAX_BONES/*numBones?*/; j++)
			{
				stringstream ss;
				ss << "boneMatrices[" << j << "]";
				boneMatricesAttribLocations[j] = glGetUniformLocation (objectList[i]->GetShaderProgramID(), ss.str().c_str());
				glUniformMatrix4fv (boneMatricesAttribLocations[j], numBones, GL_FALSE, glm::value_ptr(boneMatrices[j])/*&boneMatrices[j][0][0]*/);
			}
		}

		glBindVertexArray(objectList.at(i)->GetVAO());
		
		if(objectList[i]->MeshEntries.size() > 1)
		{
			for(int meshEntryIdx = 0; meshEntryIdx < objectList[i]->MeshEntries.size(); meshEntryIdx++)
			{
				glDrawElementsBaseVertex(GL_TRIANGLES, 
                                 objectList[i]->MeshEntries[meshEntryIdx].NumIndices, 
                                 GL_UNSIGNED_INT, 
                                 (void*)(sizeof(unsigned int) * objectList[i]->MeshEntries[meshEntryIdx].BaseIndex), 
                                 objectList[i]->MeshEntries[meshEntryIdx].BaseVertex);
			}
		}
		else if(objectList[i]->GetIndices().size() > 0)
		{
			glDrawElements( GL_TRIANGLES, objectList[i]->GetIndices().size(), GL_UNSIGNED_INT, (void*)0);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, objectList.at(i)->GetVertexCount());	
		}
	}	

	glUseProgram(shaderManager.GetShaderProgramID("diffuse"));
	glBindVertexArray( line_vao );
	glDrawArrays(GL_LINES, 0, lines.size()*2 );

	printouts();

	glutSwapBuffers();
}

void keyPressed (unsigned char key, int x, int y) 
{  
	keyStates[key] = true; // Set the state of the current key to pressed  

	if(key == '+')
		testAnimBone++;
	else if(key == '-')
		if(testAnimBone > 0)
			testAnimBone--;
}  
  
void keyUp (unsigned char key, int x, int y) 
{  
	keyStates[key] = false; // Set the state of the current key to not pressed  
}  

void passiveMouseMotion(int x, int y)  
{
	//As glutWarpPoint triggers an event callback to Mouse() we need to return to ensure it doesn't recursively call
    static bool just_warped = false;
    if(just_warped) {
        just_warped = false;
        return;
    }

	camera.ProcessMouse(x, y, deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT);

	glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
	just_warped = true;
}

// OTHER FUNCTIONS

//Process keystates
void processInput()
{
	if(keyStates[27])
		exit(0);

	//TODO
	//camera.processKeyboard(keyStates);
	
	if(keyStates['w'])
		camera.viewProperties.position += camera.viewProperties.forward * float(deltaTime) * camera.moveSpeed; //glm::vec3(0.f, 0.f, -1.f);
	if(keyStates['s'])
		camera.viewProperties.position -= camera.viewProperties.forward  * float(deltaTime) * camera.moveSpeed; //moveVector = glm::vec3(0.f, 0.f, -1.f);
	if(keyStates['a'])
		camera.viewProperties.position -= glm::cross(camera.viewProperties.forward, camera.viewProperties.up) * float(deltaTime) * camera.moveSpeed;
	if(keyStates['d'])
		camera.viewProperties.position += glm::cross(camera.viewProperties.forward, camera.viewProperties.up) * float(deltaTime) * camera.moveSpeed;

	if(keyStates['u'])
		target->worldProperties.translation += glm::vec3(1,0,0) * targetSpeed * float(deltaTime);
	if(keyStates['i'])
		target->worldProperties.translation += glm::vec3(-1,0,0) * targetSpeed * float(deltaTime);
	if(keyStates['j'])
		target->worldProperties.translation += glm::vec3(0,1,0) * targetSpeed * float(deltaTime);
	if(keyStates['k'])
		target->worldProperties.translation += glm::vec3(0,-1,0) * targetSpeed * float(deltaTime);
	if(keyStates['n'])
		target->worldProperties.translation += glm::vec3(0,0,1) * targetSpeed * float(deltaTime);
	if(keyStates['m'])
		target->worldProperties.translation += glm::vec3(0,0,-1) * targetSpeed * float(deltaTime);
}

void printouts()
{
	std::stringstream ss;

	//Bottom left is 0,0

	ss.str(std::string()); // clear
	ss << fps << " fps";
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-20, ss.str().c_str());

	//PRINT TARGET POSITION
	ss.str(std::string()); // clear
	ss << std::fixed << std::setprecision(PRECISION) << "target (x: " << target->worldProperties.translation.x << ", y: " 
		<< target->worldProperties.translation.y << ", z: " << target->worldProperties.translation.z << ") - (u/i , j/k, n/m)";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-20, ss.str().c_str());

	//PRINT BONE SELECTION
	Bone* bone = objectList[0]->GetSkeleton()->GetBone(testAnimBone % objectList[0]->GetSkeleton()->GetBones().size());

	ss.str(std::string()); // clear
	ss << "Selected Bone: [" << int(bone->id) << "] " << bone->name << " (-/+)";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),80, ss.str().c_str());
	
	ss.str(std::string()); // clear
	ss << "x Angle: " << bone->GetEulerAngles().x << " (t/y)";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),60, ss.str().c_str());
	ss.str(std::string()); // clear
	ss << "y Angle: " << bone->GetEulerAngles().y << " (g/h)";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),40, ss.str().c_str());
	ss.str(std::string()); // clear
	ss << "z Angle: " << bone->GetEulerAngles().z << " (v/b)";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),20, ss.str().c_str());

	//ss.str(std::string()); // clear
	//ss << "Selected Bone: [" << testAnimBone % objectList[0]->GetSkeleton()->GetBones().size() << "] " << objectList[0]->GetSkeleton()->GetBone(testAnimBone % objectList[0]->GetSkeleton()->GetBones().size())->name;
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),80, ss.str().c_str());
	//ss.str(std::string()); // clear
	//ss << "Selected Bone: [" << testAnimBone % objectList[0]->GetSkeleton()->GetBones().size() << "] " << objectList[0]->GetSkeleton()->GetBone(testAnimBone % objectList[0]->GetSkeleton()->GetBones().size())->name;
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),80, ss.str().c_str());

	//PRINT BONES
 //   for(int boneidx = 0; boneidx < objectList[0]->GetSkeleton()->GetBones().size(); boneidx++)
	//{
	//	Bone* tmp = objectList[0]->GetSkeleton()->GetBones()[boneidx];

	//	ss.str(std::string()); // clear

	//	ss << std::fixed << std::setprecision(PRECISION) << "bone[" << boneidx << "] = (" << tmp->GetMeshSpacePosition().x << ", " << tmp->GetMeshSpacePosition().y << ", " << tmp->GetMeshSpacePosition().z << ")";
	//	ss << " -- (" << decomposeT(tmp->finalTransform).x << ", " << decomposeT(tmp->finalTransform).y << ", " << decomposeT(tmp->finalTransform).z << ")";
	//	drawText(20,20*(boneidx+1), ss.str().c_str());
	//}

	//PRINT CAMERA
	ss.str(std::string()); // clear
	ss << "camera.forward: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.forward.x << ", " << camera.viewProperties.forward.y << ", " << camera.viewProperties.forward.z << ")";
	drawText(20,WINDOW_HEIGHT-20, ss.str().c_str());

	ss.str(std::string()); // clear
	ss << "camera.pos: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.position.x << ", " << camera.viewProperties.position.y << ", " << camera.viewProperties.position.z << ")";
	drawText(20,WINDOW_HEIGHT-40, ss.str().c_str());

	ss.str(std::string()); // clear
	ss << "camera.up: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.up.x << ", " << camera.viewProperties.up.y << ", " << camera.viewProperties.up.z << ")";
	drawText(20,WINDOW_HEIGHT-60, ss.str().c_str());

	//PRINT ANIMATION TIMER
	/*if(objectList[i]->GetSkeleton()->hasKeyframes)
	{
		ss << "AnimationTimer: " << objectList[i]->GetSkeleton()->GetAnimationTimer();
		drawText(20,50, ss.str().c_str());
	}*/
}


















