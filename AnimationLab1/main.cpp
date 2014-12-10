#include <GL/glew.h>
#include <GL/freeglut.h>

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
#include "Node.h"
#include "LevelEditor.h"
#include "Player.h"
#include "NPC.h"
#include "SplineEditor.h"

#include "Common.h"
#include "Keys.h"
#include "Gamepad.h"

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <map>
#include <vector>
#include <list>

#include <xinput.h>

using namespace std;

//Callbacks
void keyPressed (unsigned char key, int x, int y); 
void keyUp (unsigned char key, int x, int y); 
void passiveMouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);
void handleSpecialKeypress(int key, int x, int y);
void handleSpecialKeyReleased(int key, int x, int y);
void mouseWheel(int, int, int, int);
void gamepad(unsigned int buttonMask, int x, int y, int z);

void reshape(int w, int h);
void update();
void draw();

bool directionKeys[4] = {false};

bool keyStates[256] = {false}; // Create an array of boolean values of length 256 (0-255)

void processContinuousInput();
void printouts();

Camera camera;
glm::mat4 projectionMatrix; // Store the projection matrix
bool freeMouse = false;

//int WINDOW_WIDTH = 1920;
//int WINDOW_HEIGHT = 1080;

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

int oldTimeSinceStart;
double deltaTime;

int fps = 0;
int frameCounterTime = 0;
int frames = 0;
//char *text;

ShaderManager shaderManager;
vector<Model*> objectList;

LevelEditor* levelEditor;
SplineEditor* splineEditor;

short editMode = 0;
enum EditMode { off = 0, levelEdit, splineEdit };

Player* player;
bool moveFix = false;

NPC* donald;

bool printText = false;

Spline spline0;

Gamepad* xgamepad;

int main(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition (0, 0); 
    glutCreateWindow("KH 0.5 Remix");

	glutSetCursor(GLUT_CURSOR_NONE);
	
	// REGISTER GLUT CALLBACKS

	//glutDisplayFunc(draw);
	//glutReshapeFunc (reshape);
	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events  
	glutSpecialFunc(handleSpecialKeypress);
	glutSpecialUpFunc(handleSpecialKeyReleased);
	glutMouseFunc (mouseButton);
	glutMouseWheelFunc(mouseWheel);
	//glutMotionFunc (MouseMotion);
	glutPassiveMotionFunc(passiveMouseMotion);
	glutIdleFunc (update);
	glutJoystickFunc(gamepad, 300);
	glutForceJoystickFunc();

	// A call to glewInit() must be done after glut is initialized!
	glewExperimental = GL_TRUE;
    GLenum res = glewInit();
	
	#pragma region ERROR CHECKING
	// Check for any errors
    if (res != GLEW_OK) 
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
    }
	#pragma endregion 

	glClearColor(135.0/255.0, 206.0/255.0, 250.0/255.0, 1);
	glEnable (GL_CULL_FACE); // cull face 
	glCullFace (GL_BACK); // cull back face 
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_DEPTH_TEST);

	projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

	//TODO - constructor for camera
	camera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 0.0002f, 0.01f); 

	xgamepad = new Gamepad();

	levelEditor = new LevelEditor(&objectList);

	shaderManager.Init();

	shaderManager.CreateShaderProgram("skinned", "Shaders/skinned.vs", "Shaders/skinned.ps");
	shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");

	shaderManager.CreateShaderProgram("black", "Shaders/diffuse.vs", "Shaders/black.ps");
	shaderManager.CreateShaderProgram("white", "Shaders/diffuse.vs", "Shaders/white.ps");
	shaderManager.CreateShaderProgram("red", "Shaders/diffuse.vs", "Shaders/red.ps");

	shaderManager.CreateShaderProgram("text", "Shaders/diffuse.vs", "Shaders/black.ps");

	Node::objectList = &objectList;

	vector<Model*> loadedObjects = LevelEditor::Load(1);
	objectList.insert(objectList.end(), loadedObjects.begin(), loadedObjects.end());
	
	player = new Player(objectList, &camera, xgamepad, new Model(glm::vec3(15,0,0), glm::mat4(1), glm::vec3(.6), "Models/sora.dae", shaderManager.GetShaderProgramID("skinned"))); 
	donald = new NPC(objectList, new Model(glm::vec3(5,0,0), glm::mat4(1), glm::vec3(.1), "Models/don1.dae", shaderManager.GetShaderProgramID("skinned")), player);

	//objectList.push_back(new Model(glm::vec3(0,0,0), glm::mat4(1), glm::vec3(1), "Models/destinyisland.dae", shaderManager.GetShaderProgramID("diffuse")));

	#pragma region IK Stuff
	//std::vector<Bone*> chain; //just name end effector and number of links to go back!!!!
	//
	//Bone* fingersL = objectList[index]->GetSkeleton()->GetBone("fingers.L");
	//chain.push_back(fingersL); 
	//
	//Bone* wristL = fingersL->parent;
	//wristL->dofLimits.SetXLimits(-51, 68);
	//wristL->dofLimits.SetYLimits(0, 0);
	//wristL->dofLimits.SetZLimits(-20, 20);
	//chain.push_back(wristL); 

	//Bone* forearmL = wristL->parent;
	//forearmL->dofLimits.SetXLimits(0, 0);
	//forearmL->dofLimits.SetZLimits(0, 114);
	//chain.push_back(forearmL); 

	//Bone* upperArmL = forearmL->parent;
	////upperArmL->dofLimits.SetXLimits(-60, -40);
	//upperArmL->dofLimits.SetYLimits(0, 0);
	//upperArmL->dofLimits.SetZLimits(-80, 50);
	//chain.push_back(upperArmL); 
	//
	///*Bone* head = objectList[index]->GetSkeleton()->GetBone("head");
	//chain.push_back(head);

	//Bone* neck = head->parent;
	//neck->dofLimits.SetXLimits(-20, 20);
	//neck->dofLimits.SetYLimits(-35, 35);
	//neck->dofLimits.SetZLimits(-6, 50);
	//chain.push_back(neck);*/

	///*for(int i = 0; i < 3; i++)
	//	chain.push_back(chain[chain.size()-1]->parent);*/
	//std::reverse(chain.begin(),chain.end());
	//objectList.at(index)->GetSkeleton()->DefineIKChain("chain1", chain);

	//Bone* upperArmR = objectList[index]->GetSkeleton()->GetBone("upperarm.R");
	//upperArmR->transform = glm::rotate(upperArmR->transform, 70.0f, glm::vec3(1,0,0));

	//TARGET
	//target = new Model(glm::vec3(0,0,5), glm::mat4(1), glm::vec3(.1), MESH_FILE3, shaderManager.GetShaderProgramID("diffuse"));
	//objectList.push_back(target);

	//targetPath.SetMode(InterpolationMode::Cubic);
	#pragma endregion

	splineEditor = new SplineEditor(&camera);
	splineEditor->spline.SetMode(InterpolationMode::Cubic);

	spline0.SetSpeed(0.5f);
	spline0.Load(2, false);

	glutMainLoop();
    
	return 0;
}

// GLUT CALLBACK FUNCTIONS
void update()
{
	//Calculate deltaTime
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = timeSinceStart - oldTimeSinceStart;
    oldTimeSinceStart = timeSinceStart;

	//Calculate fps
	frames++;
	frameCounterTime += deltaTime;
	if(frameCounterTime > 1000)
	{
		fps = frames;
		frames = frameCounterTime = 0;
	}

	camera.Update(deltaTime);
	player->Update(deltaTime);
	donald->Update(deltaTime); //TODO - make a character class with functions for update / input etc.

	//Animation
	for(int i = 0; i< objectList.size(); i++)
	{
		if(objectList[i]->HasSkeleton())
		{
			//TODO - If animationMode == IK .. and so on
			//	if(objectList[i]->GetSkeleton()->ikChains.size() > 0)
			//		objectList[i]->GetSkeleton()->ComputeIK("chain1", /*glm::vec3(0,5,0)*/target->worldProperties.translation, 50); //replace with iteration, ikchain should be a struct with a target?
			//																											//if no target do nothing?

			if(objectList[i]->GetSkeleton()->hasKeyframes)
				objectList[i]->GetSkeleton()->Animate(deltaTime); //this overwrites control above
			
			objectList[i]->GetSkeleton()->UpdateGlobalTransforms(objectList[i]->GetSkeleton()->GetRootBone(), glm::mat4());
		}
	}

	/*if(objectList.size() == 2)
		objectList[0]->worldProperties.orientation *= glm::toMat4(glm::angleAxis(1.0f, glm::vec3(0,1,0)));*/

	//FOR ITERATING SUBCHAINS
	//std::map<char,int>::iterator it;
	//for (std::map<char,int>::iterator it=mymap.begin(); it!=mymap.end(); ++it)
	//std::cout << it->first << " => " << it->second << '\n';

	if(editMode == EditMode::splineEdit)
	{
		splineEditor->spline.Update(deltaTime);

		if(splineEditor->spline.nodes.size() > 0)
			splineEditor->tester->worldProperties.translation = splineEditor->spline.GetPosition();
	}

	if(camera.mode == CameraMode::path)
	{
		spline0.Update(deltaTime);
		camera.viewProperties.position = spline0.GetPosition();

		camera.SetTarget(glm::vec3(0,0,0));
	}

	processContinuousInput();
	draw();
}

//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
//before finally binding the VAO and drawing with verts or indices
void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glm::mat4 viewMatrix = camera.GetViewMatrix();

	for(int i = 0; i < objectList.size(); i++)
	{
		if(objectList[i]->drawMe)
		{
			//Set shader
			shaderManager.SetShaderProgram(objectList[i]->GetShaderProgramID());

			//Set MVP matrix
			glm::mat4 MVP = projectionMatrix * viewMatrix * objectList.at(i)->GetModelMatrix();
			int mvpMatrixLocation = glGetUniformLocation(objectList[i]->GetShaderProgramID(), "mvpMatrix"); // Get the location of mvp matrix in the shader
			glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVP)); // Send updated mvp matrix 
		
			//Set Bone matrices
			if(objectList[i]->HasSkeleton())
			{
				glm::mat4 boneMatrices[MAX_BONES];
				int boneMatricesAttribLocations[MAX_BONES]; //INVESTIGATE - does this really need to be done every frame?

				for(int j = 0; j < MAX_BONES; j++)
					boneMatrices[j] = glm::mat4(1);

				//objectList[i]->GetSkeleton()->UpdateGlobalTransforms(objectList[i]->GetSkeleton()->GetRootBone(), glm::mat4());

				int numBones = objectList[i]->GetSkeleton()->GetBones().size();
				for(int boneidx = 0; boneidx < numBones; boneidx++)
				{
					Bone* bone = objectList[i]->GetSkeleton()->GetBone(boneidx);
					boneMatrices[bone->id] = bone->finalTransform;
				}

				//Send up-to-date bone matrix data to the shader 
				for(int j = 0; j < MAX_BONES; j++)
				{
					stringstream ss;
					ss << "boneMatrices[" << j << "]";
					boneMatricesAttribLocations[j] = glGetUniformLocation (objectList[i]->GetShaderProgramID(), ss.str().c_str()); //Get location of bone matrix in shader
					glUniformMatrix4fv (boneMatricesAttribLocations[j], numBones, GL_FALSE, glm::value_ptr(boneMatrices[j])); //send updated matrix
				}
			}

			//Render
			objectList.at(i)->Render(shaderManager.GetCurrentShaderProgramID());
		}
	}	

	shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("text"));

	//std::stringstream ss;
	//ss.str(std::string()); // clear
	//ss << "Press 'h' to show / hide controls";
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-20, ss.str().c_str());

	//ss.str(std::string()); // clear
	//ss << fps << " fps ";
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-40, ss.str().c_str());

	if(printText)
		printouts();

	glutSwapBuffers();
}

//KEYBOARD FUCNTIONS
void keyPressed (unsigned char key, int x, int y) 
{  
	keyStates[key] = true; // Set the state of the current key to pressed  

	if(key == KEY::KEY_L || key == KEY::KEY_l)
	{
		if(editMode != EditMode::levelEdit)
		{
			if(editMode == EditMode::splineEdit)
			{
				splineEditor->tester->drawMe = false;
				splineEditor->spline.DeleteAllNodes();
			}

			editMode = EditMode::levelEdit;
		}
		else
		{
			editMode = EditMode::off;
		}
	}

	if(key == KEY::KEY_P || key == KEY::KEY_p)
	{
		if(editMode != EditMode::splineEdit)
		{
			editMode = EditMode::splineEdit;
			splineEditor->tester->drawMe = true;
		}
		else
		{
			editMode = EditMode::off;
			splineEditor->tester->drawMe = false;
			splineEditor->spline.DeleteAllNodes();
		}
	}

	if(editMode == levelEdit)
	{
		levelEditor->ProcessKeyboardOnce(key, x, y);

		if(key == KEY::KEY_8)
		{
			vector<Model*> loadedObjects = levelEditor->Load(levelEditor->fileSelect);
			objectList.insert(objectList.end(), loadedObjects.begin(), loadedObjects.end());
		}
	}
	else if(editMode == splineEdit)
	{
		splineEditor->ProcessKeyboardOnce(key, x, y);
	}
		
	camera.ProcessKeyboardOnce(key, x, y);
	player->ProcessKeyboardOnce(key, x, y);
	donald->ProcessKeyboardOnce(key, x, y);

	if(key == KEY::KEY_h || key == KEY::KEY_H)
		printText = !printText;
}  
  
void keyUp (unsigned char key, int x, int y) 
{  
	keyStates[key] = false; // Set the state of the current key to not pressed  
}  

//Process keystates
void processContinuousInput()
{
	if(keyStates[KEY::KEY_ESCAPE])
	{
		if(!freeMouse)
		{
			freeMouse = true;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
	}
	//exit(0);

	if(editMode == levelEdit)
		levelEditor->ProcessKeyboardContinuous(keyStates, directionKeys, deltaTime);
	else if(editMode == splineEdit)
		splineEditor->ProcessKeyboardContinuous(keyStates, directionKeys, deltaTime);

	camera.ProcessKeyboardContinuous(keyStates, deltaTime);
	player->ProcessKeyboardContinuous(keyStates, deltaTime);
}

//DIRECTIONAL KEYS DOWN
void handleSpecialKeypress(int key, int x, int y)
{
	switch (key) 
	{
		case GLUT_KEY_LEFT:
			directionKeys[DKEY::Left] = true;
			break;

		case GLUT_KEY_RIGHT:
			directionKeys[DKEY::Right] = true;	
			break;

		case GLUT_KEY_UP:
			directionKeys[DKEY::Up] = true;
			break;

		case GLUT_KEY_DOWN:
			directionKeys[DKEY::Down] = true;
			break;
	}

	if(editMode == levelEdit)
		levelEditor->ProcessKeyboardOnce(key, x, y);
	else if(editMode == splineEdit)
		splineEditor->ProcessKeyboardOnce(key, x, y);
}

//DIRECTIONAL KEYS UP
void handleSpecialKeyReleased(int key, int x, int y) 
{
	switch (key) 
	{
		case GLUT_KEY_LEFT:
			directionKeys[DKEY::Left] = false;
			break;

		case GLUT_KEY_RIGHT:
			directionKeys[DKEY::Right] = false;	
			break;

		case GLUT_KEY_UP:
			directionKeys[DKEY::Up] = false;
			break;

		case GLUT_KEY_DOWN:
			directionKeys[DKEY::Down] = false;
			break;
	}
}

//MOUSE FUCNTIONS
void passiveMouseMotion(int x, int y)  
{
	if(!freeMouse)
	{
		//As glutWarpPoint triggers an event callback to Mouse() we need to return to ensure it doesn't recursively call
		static bool just_warped = false;
		if(just_warped) {
			just_warped = false;
			return;
		}

		camera.MouseRotate(x, y, WINDOW_WIDTH, WINDOW_HEIGHT, deltaTime); 

		glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
		just_warped = true;
	}
}

void mouseButton(int button, int state, int x, int y)
{
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if(freeMouse)
			{
				freeMouse = false;
				glutSetCursor(GLUT_CURSOR_NONE);
			}
			break;
    }
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
		camera.Zoom(-deltaTime);
    else
		camera.Zoom(deltaTime);
}

//GAMEPAD FUNCTION

void  gamepad (unsigned int buttonMask, int x, int y, int z)
{
	if(buttonMask & GLUT_JOYSTICK_BUTTON_A) {
    printf("button A is pressed ");
	}
	if(buttonMask & GLUT_JOYSTICK_BUTTON_B) {
		printf("button B is pressed ");
	}
	if(buttonMask & GLUT_JOYSTICK_BUTTON_C) {
		printf("button C is pressed ");
	}
	if(buttonMask & GLUT_JOYSTICK_BUTTON_D) {
		printf("button D is pressed ");
	}

	//x, y left analog stick. deadzone 200
}

// OTHER FUNCTIONS

void printouts()
{
	std::stringstream ss;

	//Bottom left is 0,0

	ss.str(std::string()); // clear
	ss << "Current Mode: " ;
	switch(editMode)
	{
		case EditMode::levelEdit:
			ss << "LEVEL EDITOR";
			break;
		case EditMode::splineEdit:
			ss << "SPLINE EDITOR";
			break;
		
	}
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-60, ss.str().c_str());

	//ss.str(std::string()); // clear
	//if(Skeleton::ConstraintsEnabled)
	//	ss << "Constraints enabled |c|";
	//else
	//	ss << "Constraints disabled |c|";
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-40, ss.str().c_str());

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
	//TODO - selected model
	if(player != nullptr)
	{
		player->PrintOuts(WINDOW_WIDTH, WINDOW_HEIGHT);

		if(player->model->GetSkeleton()->hasKeyframes)
			player->model->GetSkeleton()->PrintOuts(WINDOW_WIDTH, WINDOW_HEIGHT);
	}

	if(editMode == EditMode::levelEdit)
		levelEditor->PrintOuts(WINDOW_WIDTH, WINDOW_HEIGHT);
	else if(editMode == EditMode::splineEdit)
		splineEditor->PrintOuts(WINDOW_WIDTH, WINDOW_HEIGHT);
}


















