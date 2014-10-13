#include "ShaderManager.h"
#include "Shader.h"

//Creates a program and adds it to the shader program list
GLuint ShaderManager::CreateShaderProgram(std::string name, const std::string& vsFilename, const std::string& psFilename)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram(); //https://www.opengl.org/sdk/docs/man2/xhtml/glCreateProgram.xml
	
	#pragma region ERROR CHECKING
	if (shaderProgramID == 0) 
	{
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}
	#pragma endregion

	// Create two shader objects, one for the vertex, and one for the fragment shader

	Shader vs;
	vs.LoadFile(vsFilename);
	vs.CompileShader(shaderProgramID, GL_VERTEX_SHADER);
	//https://www.opengl.org/sdk/docs/man4/html/glCreateShader.xhtml
	//https://www.opengl.org/sdk/docs/man/html/glShaderSource.xhtml
	//https://www.opengl.org/sdk/docs/man/html/glCompileShader.xhtml
	//https://www.opengl.org/sdk/docs/man/html/glAttachShader.xhtml

	Shader ps;
	ps.LoadFile(psFilename);
	ps.CompileShader(shaderProgramID, GL_FRAGMENT_SHADER);

	shaderProgramList[name] = shaderProgramID;
	
	return shaderProgramID;
}

void ShaderManager::SetShaderProgram(GLuint shaderProgramID)
{
	activeShader = shaderProgramID;

	glLinkProgram(shaderProgramID); //https://www.opengl.org/sdk/docs/man/html/glLinkProgram.xhtml
	
	#pragma region ERROR CHECKING
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) 
	{
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		system("pause");
		exit(1);
	}
	#pragma endregion
	
	//check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID); 
	
	#pragma region ERROR CHECKING
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) 
	{
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		system("pause");
		exit(1);
	}
	#pragma endregion

	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID); //https://www.opengl.org/sdk/docs/man/html/glUseProgram.xhtml
}