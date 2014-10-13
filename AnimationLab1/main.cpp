#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include <assimp/Importer.hpp>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "ShaderManager.h"

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>
#include <vector>
#include <list>

using namespace std;

void drawText(int x, int y, const char *st);
void processInput();

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
int deltaTime;

int fps = 0;
int frameCounterTime = 0;
int frames = 0;
char *text;

ShaderManager shaderManager;
vector<Mesh*> objectList;

int main(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition (100, 100); 
    glutCreateWindow("Animation!");

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

	camera.Init();
	camera.viewProperties.position = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.viewProperties.forward = glm::vec3(0.f, 0.f, 1.f);
	camera.viewProperties.up = glm::vec3(0.0f, 1.0f, 0.0f);

	glClearColor(.5,.5,.5,1);
	//glEnable (GL_CULL_FACE); // cull face 
	//glCullFace (GL_BACK); // cull back face 
	//glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_DEPTH_TEST);

	shaderManager.Init();
	shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");

	shaderManager.SetShaderProgram("diffuse"); //TODO - the shader program needs to be set before vbo is generated, investigate

	objectList.push_back(new Mesh(glm::vec3(0,0,10), glm::vec3(1,0,0), 0, glm::vec3(1), "Models/hand.dae"));
	
	objectList.at(0)->SetShaderProgramID(shaderManager["diffuse"]);

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

	processInput();
	draw();
}

float y = 0.0f;

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //glPolygonMode(GL_FRONT, GL_LINE); //Wireframe mode

	glm::mat4 viewMatrix = glm::lookAt(camera.viewProperties.position, camera.viewProperties.position + camera.viewProperties.forward, camera.viewProperties.up); 

	for(int i = 0; i < objectList.size(); i++)
	{
		shaderManager.SetShaderProgram(objectList[i]->GetShaderProgramID());
		
		glm::mat4 MVP = projectionMatrix * viewMatrix * objectList.at(i)->GetModelMatrix();

		int mvpMatrixLocation = glGetUniformLocation(objectList[i]->GetShaderProgramID(), "mvpMatrix"); // Get the location of our projection matrix in the shader
		glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, &MVP[0][0]); // Send our model/view/projection matrix to the shader

		//BONE MATRIX STUFF
		
		int boneMatricesAttribLocations[MAX_BONES];

		for (int j = 0; j < MAX_BONES; j++) 
		{	
			stringstream ss;
			ss << "boneMatrices[" << j << "]";
			boneMatricesAttribLocations[j] = glGetUniformLocation (objectList[i]->GetShaderProgramID(), ss.str().c_str());
			glUniformMatrix4fv (boneMatricesAttribLocations[j], 1, GL_FALSE, &glm::mat4x4(1)[0][0]);
		}

		//glm::mat4 ear_mat = glm::mat4(1);
		float theta = 0.0f;
		float rot_speed = 1.0f;

		/*if (keyStates['l']) 
		{
			theta += rot_speed * deltaTime;

			ear_mat = objectList[i]->GetSkeleton()->GetBones()[0]->offset * 
				glm::rotate(glm::mat4(1), theta, glm::vec3(0,0,1)) 
				* glm::inverse(objectList[i]->GetSkeleton()->GetBones()[0]->offset);
			glUniformMatrix4fv (boneMatricesAttribLocations[0], 1, GL_FALSE, &ear_mat[0][0]);
		
		}*/

		bool monkey_moved = false;

		//glm::mat4 ear_mat2 = glm::mat4(1);
		if (keyStates['z']) 
		{
			/*theta += rot_speed * deltaTime;

			ear_mat2 = objectList[i]->GetSkeleton()->GetBones()[1]->offset * 
				glm::rotate(glm::mat4(1), theta, glm::vec3(0,0,1)) 
				* glm::inverse(objectList[i]->GetSkeleton()->GetBones()[1]->offset);
			glUniformMatrix4fv (boneMatricesAttribLocations[1], 1, GL_FALSE, &ear_mat2[0][0]);*/

			/*theta += rot_speed * deltaTime;
			objectList[i]->GetSkeleton()->TEMP_local_anims[0] = glm::rotate(glm::mat4(1), theta, glm::vec3(0,0,1));
			objectList[i]->GetSkeleton()->TEMP_local_anims[1] = glm::rotate(glm::mat4(1), -theta, glm::vec3(0,0,1));
			monkey_moved = true;*/
		}

		if (keyStates['x']) 
		{
			//theta -= rot_speed * deltaTime;
			y -= 0.5f * deltaTime;
			objectList[i]->GetSkeleton()->TEMP_local_anims[0] = glm::translate(glm::mat4(1), glm::vec3(0,y,0));
			objectList[i]->GetSkeleton()->TEMP_local_anims[1] = glm::translate(glm::mat4(1), glm::vec3(0,y,0));
			//objectList[i]->GetSkeleton()->TEMP_local_anims[0] = glm::rotate(glm::mat4(1), theta, glm::vec3(0,0,1));
			//objectList[i]->GetSkeleton()->TEMP_local_anims[1] = glm::rotate(glm::mat4(1), -theta, glm::vec3(0,0,1));
			monkey_moved = true;
		}

		if (keyStates['c']) 
		{
			y -= 0.5f * deltaTime;
			objectList[i]->GetSkeleton()->TEMP_local_anims[0] = glm::translate(glm::mat4(1), glm::vec3(0,y,0));
			//monkey_moved = true;
		}

		if (keyStates['v']) 
		{
			y += 0.5f * deltaTime;
			objectList[i]->GetSkeleton()->TEMP_local_anims[0] = glm::translate(glm::mat4(1), glm::vec3(0,y,0));
			//monkey_moved = true;
		}

		glm::mat4 monkey_bone_animation_mats[MAX_BONES];

		for(int i = 0; i < MAX_BONES; i++)
		{
			monkey_bone_animation_mats[i] = glm::mat4(1);
		}

		//if (monkey_moved) 
		//{
			objectList[i]->GetSkeleton()->GetGlobalTransforms(objectList[i]->GetSkeleton()->GetRootBone(), glm::mat4(1), monkey_bone_animation_mats);

			for(int mbam = 0; mbam < MAX_BONES; mbam++)
				glUniformMatrix4fv (boneMatricesAttribLocations[mbam], objectList[i]->GetSkeleton()->GetBones().size(), GL_FALSE, &monkey_bone_animation_mats[mbam][0][0]);
		//}
	
		glBindVertexArray(objectList.at(i)->GetVAO());
		
		//glDrawArrays(GL_TRIANGLES, 0, objectList.at(i)->GetVertexCount());
		glDrawElements( GL_TRIANGLES, objectList[i]->Indices.size(), GL_UNSIGNED_INT, (void*)0);
	}

	std::stringstream ss;
	ss << fps << " fps";
	drawText(20,20, ss.str().c_str());
    
	glutSwapBuffers();
}

void keyPressed (unsigned char key, int x, int y) 
{  
	keyStates[key] = true; // Set the state of the current key to pressed  
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
}

//draw text in screenspace
void drawText(int x, int y, const char *st)
{
	int l,i;

	l=strlen(st); // see how many characters are in text string.
	glWindowPos2i(x, y); // location to start printing text
	
	for(i=0; i < l; i++) // loop until i is greater then l
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, st[i]); // Print a character on the screen
	}
}

















