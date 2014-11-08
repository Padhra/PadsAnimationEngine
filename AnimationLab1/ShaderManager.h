#ifndef _SHADERMANAGER_H                // Prevent multiple definitions if this 
#define _SHADERMANAGER_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>

class ShaderManager
{
	private:
		std::map <std::string, GLuint> shaderProgramList;
		GLuint currentShaderProgramID;

	public:
		GLuint CreateShaderProgram(std::string name, const std::string& vsFilename, const std::string& psFilename);
		
		void SetShaderProgram(std::string shaderProgramName) { SetShaderProgram(shaderProgramList[shaderProgramName]); };
		void SetShaderProgram(GLuint shaderProgramID);

		GLuint GetShaderProgramID(std::string shaderProgramName) { return shaderProgramList[shaderProgramName]; }
		GLuint GetCurrentShaderProgramID() { return currentShaderProgramID; }
};

#endif