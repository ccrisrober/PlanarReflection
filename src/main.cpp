#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL/SOIL.h"
#include "objects/Mesh.h"

#include <iostream>

const GLchar* vertexSource =
"#version 420 core\n"
"in vec3 position;"
"in vec2 texCoord;"
"out vec3 outColor;"
"out vec2 outTexcoord;"
"uniform mat4 model;"
"uniform mat4 viewProj;"
"uniform vec3 color;"
"void main() {"
"    outColor = color;"
"    outTexcoord = texCoord;"
"    gl_Position = viewProj * model * vec4(position, 1.0);"
"}";
const GLchar* fragmentSource =
"#version 420 core\n"
"in vec3 outColor;"
"in vec2 outTexcoord;"
"out vec4 fragColor;"
"uniform sampler2D tex;"
"void main() {"
"    fragColor = vec4(outColor, 1.0) * texture(tex, outTexcoord);"
"}";

bool keys[1024];
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}
bool stencil = true;
GLFWwindow* window;
#include <sstream>
#define WINDOW_TITLE "Planar reflections"

void updateTitle() {
	std::stringstream ss;
	ss << WINDOW_TITLE;
	ss << " - STENCIL ";
	ss << (stencil? "ON" : "OFF");
	glfwSetWindowTitle(window, ss.str().c_str());
}

void do_movement()
{
	if (keys[GLFW_KEY_A]) {
		stencil = false;
		updateTitle();
	}
	else if (keys[GLFW_KEY_S]) {
		stencil = true;
		updateTitle();
	}
}

// Delta
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

GLuint createProgram() {
	GLuint vsCode = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsCode, 1, &vertexSource, NULL);
	glCompileShader(vsCode);

	GLuint fsCode = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsCode, 1, &fragmentSource, NULL);
	glCompileShader(fsCode);

	GLuint program = glCreateProgram();
	glAttachShader(program, vsCode);
	glAttachShader(program, fsCode);
	glLinkProgram(program);
	glUseProgram(program);

	return program;
}

GLuint loadTexture(char* file) {
	GLuint texture;
	glGenTextures(1, &texture);

	int width, height;
	unsigned char* image;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	image = SOIL_load_image(file, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texture;
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	updateTitle();

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat vertices[] = {
		// Positions			// TexCoords
		-1.0f, -1.0f, -0.5f,	0.0f, 0.0f,
		 1.0f, -1.0f, -0.5f,	1.0f, 0.0f,
		 1.0f,  1.0f, -0.5f,	1.0f, 1.0f,
		 1.0f,  1.0f, -0.5f,	1.0f, 1.0f,
		-1.0f,  1.0f, -0.5f,	0.0f, 1.0f,
		-1.0f, -1.0f, -0.5f,	0.0f, 0.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint program = createProgram();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	GLuint texture = loadTexture("../textures/container.jpg");
	GLuint texture2 = loadTexture("../textures/model.png");

	GLint modelUniform = glGetUniformLocation(program, "model");
	GLint colorUniform = glGetUniformLocation(program, "color");

	glm::mat4 view = glm::lookAt(
		glm::vec3(9.0f, -0.5f, 2.5f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);

	glUniformMatrix4fv(glGetUniformLocation(program, "viewProj"), 1, GL_FALSE, glm::value_ptr(projection * view));

	Mesh m("../models/monkeyhead.obj");

	glfwSetKeyCallback(window, key_callback);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);

	std::cout << "Stencil code ON: S " << std::endl;
	std::cout << "Stencil code OFF: A " << std::endl;

	while (!glfwWindowShouldClose(window)) {

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float time = glfwGetTime();
		
		glm::mat4 model;
		glm::mat4 modelScale;
		modelScale = glm::rotate(
			model,
			time * glm::radians(180.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		modelScale = glm::scale(modelScale, glm::vec3(1.3f));
		modelScale = glm::translate(modelScale, glm::vec3(0.0, 0.0, 2.0));
		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(modelScale));

		// Draw model
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform3f(colorUniform, 1.0f, 1.0f, 1.0f);
		m.render();

		// Draw floor
		if (stencil) {
			glEnable(GL_STENCIL_TEST);

			glStencilFunc(GL_ALWAYS, 1, 0xffffff);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xffffff);
			glDepthMask(GL_FALSE);

			glClear(GL_STENCIL_BUFFER_BIT);
		}
		glBindVertexArray(vao);
		glm::mat4 model2 = glm::translate(model, glm::vec3(2.0f * cos(glfwGetTime()), 2.0f * sin(glfwGetTime()), 0.0));
		model2 = glm::scale(
			model2,
			glm::vec3(4.5f)
		);
		glUniform3f(colorUniform, 0.75f, 0.0f, 0.0f);
		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model2));
		glBindTexture(GL_TEXTURE_2D, texture2);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Draw model reflection
		if (stencil) {
			glStencilFunc(GL_EQUAL, 1, 0xffffff);	// Only draw model if stencil value has equal
			glStencilMask(0x000000);
			glDepthMask(GL_TRUE);
		}

		modelScale = glm::translate(modelScale, glm::vec3(0.0, 0.0, -3.0));
		modelScale = glm::translate(modelScale, glm::vec3(0.0, 0.0, -1.0));
		modelScale = glm::scale(
			modelScale,
			glm::vec3(1, 1, -1)
		);
		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(modelScale));
		
		glUniform3f(colorUniform, 0.25f, 0.0f, 0.0f);
		glBindTexture(GL_TEXTURE_2D, texture);
		m.render();

		if (stencil) {
			glDisable(GL_STENCIL_TEST);
		}

		// Swap buffers
		glfwSwapBuffers(window);
	}

	glDeleteTextures(1, &texture);

	glDeleteProgram(program);
	// TODO: Clear shaders p.e: glDeleteShader(vsCode);

	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);

	return 0;
};