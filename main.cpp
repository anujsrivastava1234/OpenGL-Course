#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//imgui headers
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

const GLuint WIDTH = 800, HEIGHT = 600;
GLuint VAO, VBO, IBO, shaders, uniformModel, uniformProjection;

bool direction = false;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.00005f;
float curAngle = 0.0f;

float xAxis = 0.0f;
float yAxis = 0.0f;
float zAxis = 0.0f;

static const char* vShader = R"(
#version 330
layout (location = 0 ) in vec3 pos;
uniform mat4 model;
uniform mat4 projection;
out vec4 vCol;
void main()
{
	gl_Position = projection  * model * vec4(pos, 1.0f);
	vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);
}
)";

static const char* fShader = R"(
#version 330
out vec4 color;
in vec4 vCol;
void main()
{
	color = vCol;
}
)";

void CreateTriangles()
{
	//GLuint indices[] = {
	//	0, 2, 3,
	//	0, 1, 3,
	//	2, 1, 3,
	//	0, 1, 2,
	//};

	GLuint indices[] = {
		0, 1, 2,
		1, 3, 2,
		0, 6, 4,
		0, 2, 6,
		1, 5, 7,
		1, 3, 7,
		4, 5, 6,
		5, 7, 6,
		0, 1, 5,
		0, 4, 5,
		2, 6, 3,
		3, 6, 7
	};

	
	//GLfloat vertices[] = {
	//	 -1.0f, -1.0f, 0.0f,  //0
	//	  0.0f, -1.0f, 1.0f,  //1
	//	  1.0f, -1.0f, 0.0f, //2
	//	  0.0f,  1.0f, 0.0f, //3	 
	//};

	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f, //0
		 1.0f, -1.0f, 0.0f, //1
		-1.0f, 1.0f, 0.0f, //2
		1.0f, 1.0f, 0.0f, //3

		-1.0f, -1.0f, 1.0f, //4
		1.0f, -1.0f, 1.0f, //5
		-1.0f, 1.0f, 1.0f, //6
		1.0f, 1.0f, 1.0f, //7
	};

	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glCreateBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

	uniformModel = glGetUniformLocation(shaders, "model");
	uniformProjection = glGetUniformLocation(shaders, "projection");
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

	glViewport(0, 0, bufferWidth, bufferHeight);
	glEnable(GL_DEPTH_TEST);


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	CreateTriangles();
	CompileShaders();

	glm::mat4 projection(1.0f);
	projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 0.1f, 100.0f);

	float transform[3]= {0.0f, 0.0f, 0.0f};
	float scale[3] = { 0.0f, 0.0f, 0.0f };
	bool x = true;
	bool y = false;
	bool z = false;

	while (!glfwWindowShouldClose(mainWindow))
	{
		glfwPollEvents();


		if (!direction)
		{
			triOffset += triIncrement;
		}
		else {
			triOffset -= triIncrement;
			//curAngle -= 0.01f;
		}

		if (abs(triOffset) >= triMaxOffset)
		{
			direction = !direction;
		}

		//curAngle += 0.05f;

		/*if (abs(curAngle) >= 360)
		{
			curAngle = 0;
		}*/

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("My Window");
		ImGui::Text("Hello");
		ImGui::SliderFloat3("Transform", transform, -0.7f, 0.7f, "%.1f");
		ImGui::SliderFloat3("Scale", scale, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("Rotate", &curAngle, 0, 360, "%.1f");
		if (ImGui::Checkbox("X", &x))
		{
			xAxis = 1.0f;
		}

		if (ImGui::Checkbox("Y", &y))
		{
			yAxis = 1.0f;
		}

		if (ImGui::Checkbox("Z", &z))
		{
			zAxis = 1.0f;
		}


		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		
		glUseProgram(shaders);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glm::mat4 model(1.0f);


		model = glm::translate(model, glm::vec3(transform[0], transform[1], -2.5f));

		model = glm::rotate(model, glm::radians(curAngle), glm::vec3(xAxis, yAxis, zAxis));

		model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));
		
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(mainWindow);
	}

	return 0;
}