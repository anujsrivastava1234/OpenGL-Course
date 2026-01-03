#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const GLuint WIDTH = 800, HEIGHT = 600;
GLuint VAO, VBO, shaders, uniformXMove;

bool direction = false;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.00005f;

static const char* vShader = R"(
#version 330
layout (location = 0 ) in vec3 pos;
uniform float xMove;
void main()
{
	gl_Position = vec4(0.4 * pos.x + xMove, 0.4 * pos.y, pos.z, 1.0f);
}
)";

static const char* fShader = R"(
#version 330
out vec4 color;
void main()
{
	color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

void CreateTriangles()
{
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void AddShader(GLuint theProgram, const char* shaderSource, GLenum shaderType) {

	GLuint theShader = glCreateShader(shaderType);
	if (!theShader)
	{
		printf("Could not create the shader program\n");
		return;
	}

	const char* theCode[1];
	theCode[0] = shaderSource;

	GLint theCodeLength[1];
	theCodeLength[0] = strlen(shaderSource);

	glShaderSource(theShader, 1, theCode, theCodeLength);
	glCompileShader(theShader);
	
	GLint result = { 0 };
	GLchar infoLog[1024] = { 0 };
	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(theShader, 1024, NULL, infoLog);
		printf("Could not compile the shader : %s\n", infoLog);
		return;
	}

	glAttachShader(theProgram, theShader);
}

void CompileShaders()
{
	shaders = glCreateProgram();
	if (!shaders)
	{
		printf("Could not create program\n");
		return ;
	}

	AddShader(shaders, vShader, GL_VERTEX_SHADER);
	AddShader(shaders, fShader, GL_FRAGMENT_SHADER);

	GLint result = { 0 };
	GLchar infoLog[1024] = {0};
	glLinkProgram(shaders);

	glGetProgramiv(shaders, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaders, 1024, NULL, infoLog);
		printf("Could not link program : %s\n", infoLog);
		return;
	}

	glValidateProgram(shaders);
	glGetProgramiv(shaders, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaders, 1024, NULL, infoLog);
		printf("Could not validate program : %s\n", infoLog);
		return;
	}

	uniformXMove = glGetUniformLocation(shaders, "xMove");
}

int main(int argc, char* argv[])
{
	if (!glfwInit())
	{
		printf("Could not intialize the GLFW context\n");
		glfwTerminate();
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", NULL, NULL);

	if (!mainWindow)
	{
		printf("Could not create mainWindow\n");
		glfwTerminate();
		return 1;
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(mainWindow);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Could not intialize the GLEW context");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	CreateTriangles();
	CompileShaders();

	while (!glfwWindowShouldClose(mainWindow))
	{
		glfwPollEvents();

		if (!direction)
		{
			triOffset += triIncrement;
		}
		else {
			triOffset -= triIncrement;
		}

		if (abs(triOffset) >= triMaxOffset)
		{
			direction = !direction;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(shaders);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUniform1f(uniformXMove, triOffset);
		
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(mainWindow);
	}


	return 0;
}