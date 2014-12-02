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
#include "Node.h"

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <map>
#include <vector>
#include <list>

#define HAND "Models/hand_with_animation.dae"
#define CUBE "Models/cubeTri.obj"
#define BOB "Models/boblampclean.md5mesh"
#define CONES "Models/Cones3.dae"
#define IZANUGI "Models/izanugi.DAE"
#define SQUALL "Models/Squall.DAE"
#define RIKKU "Models/Rikku6.DAE"

#define PRECISION 3

using namespace std;

//Callbacks
void keyPressed (unsigned char key, int x, int y); 
void keyUp (unsigned char key, int x, int y); 
void passiveMouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);
void handleSpecialKeypress(int key, int x, int y);
void handleSpecialKeyReleased(int key, int x, int y);
void update();
void draw();

bool isRightKeyPressed = false;
bool isLeftKeyPressed = false;
bool isUpKeyPressed = false;
bool isDownKeyPressed = false;
bool keyStates[256] = {false}; // Create an array of boolean values of length 256 (0-255)

void processContinuousInput();
void printouts();

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
int selectedTarget = 0;

std::vector<glm::vec3> lines;

short uiMode = 0;
enum UIMode { boneSelect = 0, xAngle, yAngle, zAngle, nodeSelect, nodeMove, lerpMode, targetMove, splineSpeed, fileSelect };
bool altDirectional = false;

static bool constraints = false;
bool firstRun = true;

int selectedFile = 1;

bool freeMouse = false;

int main(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition (100, 100); 
    glutCreateWindow("Final Fantasy X - Freemium Edition");

	glutSetCursor(GLUT_CURSOR_NONE);
	
	// REGISTER GLUT CALLBACKS

	//glutDisplayFunc(draw);
	//glutReshapeFunc (reshape);
	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events  
	glutSpecialFunc(handleSpecialKeypress);
	glutSpecialUpFunc(handleSpecialKeyReleased);
	glutMouseFunc (mouseButton);
	//glutMotionFunc (MouseMotion);
	glutPassiveMotionFunc(passiveMouseMotion);
	glutIdleFunc (update);

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

	glClearColor(0.5,0.5,0.5,1);
	glEnable (GL_CULL_FACE); // cull face 
	glCullFace (GL_BACK); // cull back face 
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_DEPTH_TEST);

	projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

	//TODO - constructor for camera
	camera.Init(0.0002f, 0.005f); 
	camera.viewProperties.position = glm::vec3(0.0f, 0.0f, 0.0f);
	//camera.viewProperties.forward = glm::vec3(0.f, 0.f, -10.f); //horizontal and vertical control this
	camera.viewProperties.up = glm::vec3(0.0f, 1.0f, 0.0f);

	shaderManager.CreateShaderProgram("skinned", "Shaders/skinned.vs", "Shaders/skinned.ps");
	shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");

	//TODO - make this nicer
	Node::objectList = &objectList;
	Node::shaderManager = &shaderManager;

	glm::quat q = glm::quat();
	q *= glm::angleAxis(-90.0f, glm::vec3(1,0,0));
	q *= glm::angleAxis(180.0f, glm::vec3(0,0,1));
	q *= glm::angleAxis(0.0f, glm::vec3(0,0,1));

	objectList.push_back(new Model(glm::vec3(0,0,5), glm::toMat4(q), glm::vec3(1), "Models/sora.dae", shaderManager.GetShaderProgramID("skinned"))); //what
	objectList[objectList.size()-1]->GetSkeleton()->LoadAnimation("Models/sora.dae");
	
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

	//Animation
	for(int i = 0; i< objectList.size(); i++)
	{
		if(objectList[i]->HasSkeleton())
		{
		//	//TODO - If animationMode == IK .. and so on
			int numbones = objectList[i]->GetSkeleton()->GetBones().size();
			int testAnimMod = testAnimBone % numbones;
			Bone* bone = objectList[i]->GetSkeleton()->GetBones()[testAnimMod];

			if(uiMode == UIMode::xAngle && isLeftKeyPressed)
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 5.0f, glm::vec3(1,0,0));
			else if(uiMode == UIMode::xAngle && isRightKeyPressed)
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 5.0f, glm::vec3(-1,0,0));

			if(uiMode == UIMode::yAngle && isLeftKeyPressed)
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 5.0f, glm::vec3(0,1,0));
			else if(uiMode == UIMode::yAngle && isRightKeyPressed)
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 5.0f, glm::vec3(0,-1,0));

			if(uiMode == UIMode::zAngle && isLeftKeyPressed)
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 5.0f, glm::vec3(0,0,1));
			else if(uiMode == UIMode::zAngle && isRightKeyPressed)
				objectList[i]->GetSkeleton()->GetBones()[testAnimMod]->transform = glm::rotate(bone->transform, 5.0f, glm::vec3(0,0,-1));

		//	if(objectList[i]->GetSkeleton()->ikChains.size() > 0)
		//		objectList[i]->GetSkeleton()->ComputeIK("chain1", /*glm::vec3(0,5,0)*/target->worldProperties.translation, 50); //replace with iteration, ikchain should be a struct with a target?
		//																											//if no target do nothing?
			if(objectList[i]->GetSkeleton()->hasKeyframes)
				objectList[i]->GetSkeleton()->Animate(deltaTime); //this overwrites control above
		}
	}

	//FOR ITERATING SUBCHAINS
	//std::map<char,int>::iterator it;
	//for (std::map<char,int>::iterator it=mymap.begin(); it!=mymap.end(); ++it)
	//std::cout << it->first << " => " << it->second << '\n';

	if(targetPath.nodes.size() > 0 && uiMode != UIMode::targetMove)
	{
		targetPath.Update(deltaTime);
		target->worldProperties.translation = targetPath.GetPosition();
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

	glm::mat4 viewMatrix = glm::lookAt(camera.viewProperties.position, camera.viewProperties.position + camera.viewProperties.forward, camera.viewProperties.up); 

	for(int i = 0; i < objectList.size(); i++)
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

			objectList[i]->GetSkeleton()->UpdateGlobalTransforms(objectList[i]->GetSkeleton()->GetRootBone(), glm::mat4());

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

	printouts();

	glutSwapBuffers();
}

void keyPressed (unsigned char key, int x, int y) 
{  
	keyStates[key] = true; // Set the state of the current key to pressed  

	if(key == 32)
		altDirectional = !altDirectional;

	if(key == 27)
		if(!freeMouse)
			freeMouse;

	//SPLINE EDITOR
	if(key == 'o')
		targetPath.Save(selectedFile);
	if(key == 'l')
	{
		int size = targetPath.nodes.size();
		for(int i = 0; i < size; i++)
		{
			Node* node = *(targetPath.nodes.begin());
			objectList.erase(std::remove(objectList.begin(), objectList.end(), node->box), objectList.end());
			targetPath.DeleteNode(node);
		}

		targetPath.Load(selectedFile);
	}
	if(key == 'p')
		targetPath.AddNode(new Node(glm::vec3(0,0,0)));
	if(key == 't')
		if (uiMode != UIMode::targetMove)
			uiMode = UIMode::targetMove;
		else
			uiMode = -1;
	if(key == 'n')
		uiMode = UIMode::nodeSelect;
	if(key == 'm')
		uiMode = UIMode::nodeMove;
	if(key == 'i')
		if(targetPath.mode == InterpolationMode::Cubic)
			targetPath.SetMode(InterpolationMode::Linear);
		else
			targetPath.SetMode(InterpolationMode::Cubic);
	if(key == 'j')
	{
		if(targetPath.nodes.size() > 0)
		{
			Node* node = targetPath.nodes[selectedTarget % targetPath.nodes.size()];
			objectList.erase(std::remove(objectList.begin(), objectList.end(), node->box), objectList.end());
			targetPath.DeleteNode(node);
		}
	}
	if(key == 'c')
		Skeleton::ConstraintsEnabled = !Skeleton::ConstraintsEnabled;
	if(key == 'u')
		uiMode = UIMode::splineSpeed;
	if(key == 'h')
		uiMode = UIMode::fileSelect;

	//SKELETAL CONTROLS
	if(key == 'b')
		uiMode = UIMode::boneSelect;
	if(key == 'x')
		uiMode = UIMode::xAngle;
	if(key == 'y')
		uiMode = UIMode::yAngle;
	if(key == 'z')
		uiMode = UIMode::zAngle;

	if(key == 'z')
		freeMouse = !freeMouse;
}  
  
void keyUp (unsigned char key, int x, int y) 
{  
	keyStates[key] = false; // Set the state of the current key to not pressed  
}  

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

		//x *= mouseSensitivity;
		//y *= mouseSensitivity;

		camera.ProcessMouse(x, y, deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT);

	
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

void handleSpecialKeypress(int key, int x, int y)
{
	switch (key) 
	{
		case GLUT_KEY_LEFT:

			if(uiMode == UIMode::boneSelect)
				testAnimBone--;
			else if(uiMode == UIMode::nodeSelect)
				selectedTarget--;
			else if(uiMode == UIMode::splineSpeed)
			{
				Spline::speedScalar -= 0.1f;

				if(Spline::speedScalar < 0)
					Spline::speedScalar = 0;
			}
			else if(uiMode == UIMode::fileSelect)
			{
				selectedFile--;

				if(selectedFile < 1)
					selectedFile = 1;
			}
			
			isLeftKeyPressed = true;

			break;

		case GLUT_KEY_RIGHT:

			if(uiMode == UIMode::boneSelect)
				testAnimBone++;
			else if(uiMode == UIMode::nodeSelect)
				selectedTarget++;
			else if(uiMode == UIMode::splineSpeed)
				Spline::speedScalar += 0.1f;
			else if(uiMode == UIMode::fileSelect)
				selectedFile++;

			isRightKeyPressed = true;
			
			break;

		case GLUT_KEY_UP:
			isUpKeyPressed = true;
			break;

		case GLUT_KEY_DOWN:
			isDownKeyPressed = true;
			break;
	}
}

void handleSpecialKeyReleased(int key, int x, int y) 
{
	switch (key) 
	{
		case GLUT_KEY_LEFT:
			isLeftKeyPressed = false;
			break;

		case GLUT_KEY_RIGHT:
			isRightKeyPressed = false;
			break;

		case GLUT_KEY_UP:
			isUpKeyPressed = false;
			break;

		case GLUT_KEY_DOWN:
			isDownKeyPressed = false;
			break;
	}
}

// OTHER FUNCTIONS

//Process keystates
void processContinuousInput()
{
	if(keyStates[27])
	{
		if(!freeMouse)
		{
			freeMouse = true;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
	}
	//exit(0);

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
	if(keyStates['q'])
		camera.viewProperties.position -= camera.viewProperties.up * float(deltaTime) * camera.moveSpeed; //glm::vec3(0.f, 0.f, -1.f);
	if(keyStates['e'])
		camera.viewProperties.position += camera.viewProperties.up * float(deltaTime) * camera.moveSpeed; //moveVector = glm::vec3(0.f, 0.f, -1.f);

	if(uiMode == UIMode::targetMove)
	{
		if(isLeftKeyPressed)
			target->worldProperties.translation += glm::vec3(1,0,0) * targetSpeed * float(deltaTime);
		if(isRightKeyPressed)
			target->worldProperties.translation += glm::vec3(-1,0,0) * targetSpeed * float(deltaTime);
		if(isUpKeyPressed && !altDirectional)
			target->worldProperties.translation += glm::vec3(0,1,0) * targetSpeed * float(deltaTime);
		if(isDownKeyPressed && !altDirectional)
			target->worldProperties.translation += glm::vec3(0,-1,0) * targetSpeed * float(deltaTime);
		if(isUpKeyPressed && altDirectional)
			target->worldProperties.translation += glm::vec3(0,0,1) * targetSpeed * float(deltaTime);
		if(isDownKeyPressed && altDirectional)
			target->worldProperties.translation += glm::vec3(0,0,-1) * targetSpeed * float(deltaTime);
	}

	if(targetPath.nodes.size() > 0 && uiMode == UIMode::nodeMove)
	{
		if(isLeftKeyPressed)
			targetPath.nodes[selectedTarget % targetPath.nodes.size()]->box->worldProperties.translation += glm::vec3(1,0,0) * targetSpeed * float(deltaTime);
		if(isRightKeyPressed)
			targetPath.nodes[selectedTarget % targetPath.nodes.size()]->box->worldProperties.translation += glm::vec3(-1,0,0) * targetSpeed * float(deltaTime);
		if(isUpKeyPressed && !altDirectional)
			targetPath.nodes[selectedTarget % targetPath.nodes.size()]->box->worldProperties.translation += glm::vec3(0,1,0) * targetSpeed * float(deltaTime);
		if(isDownKeyPressed && !altDirectional)
			targetPath.nodes[selectedTarget % targetPath.nodes.size()]->box->worldProperties.translation += glm::vec3(0,-1,0) * targetSpeed * float(deltaTime);
		if(isUpKeyPressed && altDirectional)
			targetPath.nodes[selectedTarget % targetPath.nodes.size()]->box->worldProperties.translation += glm::vec3(0,0,1) * targetSpeed * float(deltaTime);
		if(isDownKeyPressed && altDirectional)
			targetPath.nodes[selectedTarget % targetPath.nodes.size()]->box->worldProperties.translation += glm::vec3(0,0,-1) * targetSpeed * float(deltaTime);
	}
}

void printouts()
{
	std::stringstream ss;

	//Bottom left is 0,0

	ss.str(std::string()); // clear
	ss << "Current Mode: " ;
	switch(uiMode)
	{
		case UIMode::boneSelect:
			ss << "boneSelect";
			break;
		case UIMode::lerpMode:
			ss << "lerpMode";
			break;
		case UIMode::nodeMove:
			ss << "nodeMove";
			break;
		case UIMode::nodeSelect:
			ss << "nodeSelect";
			break;
		case UIMode::splineSpeed:
			ss << "splineSpeed";
			break;
		case UIMode::targetMove:
			ss << "targetMove";
			break;
		case UIMode::xAngle:
			ss << "xAngle";
			break;
		case UIMode::yAngle:
			ss << "yAngle";
			break;
		case UIMode::zAngle:
			ss << "zAngle";
			break;
	}
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-20, ss.str().c_str());

	ss.str(std::string()); // clear
	ss << fps << " fps ";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-60, ss.str().c_str());


	ss.str(std::string()); // clear
	if(Skeleton::ConstraintsEnabled)
		ss << "Constraints enabled |c|";
	else
		ss << "Constraints disabled |c|";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-40, ss.str().c_str());

	//PRINT TARGET POSITION
	
	//drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),WINDOW_HEIGHT-20, ss.str().c_str());

	//PRINT SPLINE INFO
	/*ss.str(std::string());
	ss << "SPLINE EDITOR ";
	drawText(20,220, ss.str().c_str());

	ss.str(std::string());
	ss << "|h| File Select: " << selectedFile;
	drawText(20,200, ss.str().c_str());

	ss.str(std::string());
	ss << "|u| Speed scalar: " << Spline::speedScalar;
	drawText(20,180, ss.str().c_str());

	ss.str(std::string());
	ss << "|p| Add a node ";
	drawText(20,160, ss.str().c_str());

	ss.str(std::string());
	ss << "|o| Save Spline ";
	drawText(20,140, ss.str().c_str());

	ss.str(std::string());
	ss << "|l| Load Spline ";
	drawText(20,120, ss.str().c_str());

	ss.str(std::string());
	ss << "|i| Interpolation Mode: ";
	if(targetPath.mode == InterpolationMode::Linear)
		ss << "Linear";
	else
		ss << "Cubic";
	drawText(20,100, ss.str().c_str());

	ss.str(std::string());
	if(targetPath.nodes.size() > 0)
		ss << "|n| Selected Node: " << selectedTarget % targetPath.nodes.size();
	else
		ss << "|n| Selected Node: N/A";
	drawText(20,80, ss.str().c_str());

	ss.str(std::string());
	ss << "|j| Delete selected node ";
	drawText(20,60, ss.str().c_str());

	ss.str(std::string());
	ss << "|m| Move Selected Node";
	drawText(20,40, ss.str().c_str());*/

	//ss.str(std::string()); // clear
	//ss << std::fixed << std::setprecision(PRECISION) << "|t| target (x: " << target->worldProperties.translation.x << ", y: " 
	//	<< target->worldProperties.translation.y << ", z: " << target->worldProperties.translation.z << ")";
	//drawText(20,20, ss.str().c_str());

	//PRINT BONE SELECTION (of first object)
	if(objectList.size() > 0)
	{
		if(objectList[0]->HasSkeleton())
		{
			ss.str(std::string()); // clear
			ss << "SKELETAL CONTROLS";
			drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),100, ss.str().c_str());
	
			Bone* bone = objectList[0]->GetSkeleton()->GetBone(testAnimBone % objectList[0]->GetSkeleton()->GetBones().size());

			ss.str(std::string()); // clear
			ss << "Selected Bone: [" << int(bone->id) << "] " << bone->name << " |b|";
			drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),80, ss.str().c_str());
	
			ss.str(std::string()); // clear
			ss << "x Angle: " << bone->GetEulerAngles().x << " |x|";
			drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),60, ss.str().c_str());
			ss.str(std::string()); // clear
			ss << "y Angle: " << bone->GetEulerAngles().y << " |y|";
			drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),40, ss.str().c_str());
			ss.str(std::string()); // clear
			ss << "z Angle: " << bone->GetEulerAngles().z << " |z|";
			drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*10),20, ss.str().c_str());
		}
	}

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

	//PRINT BONES
 //   for(int boneidx = 0; boneidx < objectList[0]->GetSkeleton()->GetBones().size(); boneidx++)
	//{
	//	Bone* tmp = objectList[0]->GetSkeleton()->GetBones()[boneidx];

	//	ss.str(std::string()); // clear

	//	ss << std::fixed << std::setprecision(PRECISION) << "bone[" << boneidx << "] = (" << tmp->GetMeshSpacePosition().x << ", " << tmp->GetMeshSpacePosition().y << ", " << tmp->GetMeshSpacePosition().z << ")";
	//	ss << " -- (" << decomposeT(tmp->finalTransform).x << ", " << decomposeT(tmp->finalTransform).y << ", " << decomposeT(tmp->finalTransform).z << ")";
	//	drawText(20,20*(boneidx+1), ss.str().c_str());
	//}

	//PRINT ANIMATION TIMER
	//TODO - selected model
	if(objectList[0]->HasSkeleton())
	{
		if(objectList[0]->GetSkeleton()->hasKeyframes)
		{
			ss.str(std::string()); // clear
			ss << "AnimationTimer: " << objectList[0]->GetSkeleton()->GetAnimationTimer();
			drawText(20,50, ss.str().c_str());
		}
	}
}


















