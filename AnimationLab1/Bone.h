#ifndef _BONE_H                // Prevent multiple definitions if this 
#define _BONE_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <glm\glm.hpp>
#include <list>
#include <vector>

#include "Helper.h"

//The bone should also have an initial orientation, 
//local transformation, and the ability to update its orientation with respect to its parent (to create animation).
//You may want a function here that can calculate the global transform of the bone, 
//based on the global transform of the parent.
class Bone
{
	private:

	public:
		const char* name;
		int id; // if this node corresponds to one of our weight-painted bones then we give
		//the index of that here, otherwrise it is set to -1

		Bone* parent; //If parent == null, I am (g)root
		std::vector<Bone*> children;

		glm::mat4 offset;
		/*glm::vec3 m_rotationAxis;
		float m_rotationAngle;
		glm::vec3 m_translation;*/				//A better node		//glm::mat4 transform;
};

/*
	void RenderObject(Node *pNode)
	{
		nodeGlobal = pNode->getGlobalTransform();
		Send uniform matrix nodeGlobal to shader
		pNode->render();
	
		//DEPTH FIRST pattern from root to leaf node
		for each child of pNode, do
			RenderObject(child);
	}

	void displayFunction(void)
	{
		RenderObject(pSceneRootNode);
	}
*/

//GetGlobalTransform()
//{
	//translate to x,y of root
	//rotate to root rotation
	//translate offset, rotate to child rot
	//if child == me, return translation matrix?
//}

#endif