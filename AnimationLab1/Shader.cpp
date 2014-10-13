#include "Shader.h"

Shader::Shader()
{

}

Shader::~Shader()
{

}

// 1.2
void Shader::CompileShader(GLuint ShaderProgram, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	#pragma region ERROR CHECKING
	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	#pragma endregion

	const char* st = shaderText.c_str();

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&st, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	
	#pragma region ERROR CHECKING
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	#pragma endregion

	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Shader::LoadFile(const std::string& fileName)
{
	std::ifstream file(fileName);
	
	#pragma region ERROR CHECKING
	if(!file.is_open())
	{
		std::cout << "Error Loading file: " << fileName << " - impossible to open file" << std::endl;
		return false;
	}

	if(file.fail())
	{
		std::cout << "Error Loading file: " << fileName << std::endl;
		return false;
	}
	#pragma endregion

	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	shaderText = stream.str();

	return true;
}