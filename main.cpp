#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <algorithm>

gps::Window myWindow;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

gps::Camera myCamera(
	glm::vec3(0.0f, 10.0f, 0.0f),
	glm::vec3(40.0f, 10.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLuint cameraPositionLoc;

GLfloat cameraSpeed = 1.0f;
double pitch = 0.0f, yaw = 0.0f;

GLboolean pressedKeys[1024];

gps::Model3D teapot;
gps::Model3D green;
gps::Model3D airship;
gps::Model3D propeller;
gps::Model3D leftDoor;
gps::Model3D rightDoor;
gps::Model3D jeep;
gps::Model3D frontLeftWheel;
gps::Model3D frontRightWheel;
gps::Model3D backLeftWheel;
gps::Model3D backRightWheel;
gps::Model3D rock;
gps::Model3D swan;
gps::Model3D rain;
gps::Model3D building;
gps::Model3D water;
gps::Model3D peacock;

gps::Shader myBasicShader;
gps::Shader reflectionShader;
gps::Shader transparentShader;


GLfloat angle = 0.0f;

GLfloat doorAngle = 0.0f;
GLfloat doorSpeed = 0.008f;

GLfloat carPosition = 0.0f;
GLfloat carPositionLoc;
GLfloat carSpeed = 0.1f;
GLuint headlights = 1;
GLuint headlightsLoc;

GLfloat fogDensity = 0.000f;
GLfloat fogDensityLoc;
GLfloat fogIncrease = 0.00005f;

GLfloat reflectFactor = 0.5f;
GLfloat reflectFactorLoc;

GLfloat r1 = 0.0f, r2 = 0.0f;
bool rAux = false;
bool itsRaining = false;

float delta = 0.0f;
float movementSpeed = 0.09f;
double lastTimeStamp = glfwGetTime();

bool presentation;
int scene = 0;

void raining() {
	if (r1 < -35.0f) {
		r1 = 40.0f;
		rAux = true;
	}
	r1 -= 0.9f;
	if (rAux) {
		r2 -= 0.9f;
	}
	if (r2 < -35.0f) {
		r2 = 40.0f;
	}
}

void updateDelta(double elapsedSeconds) {
	delta += movementSpeed * elapsedSeconds;
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 1.0f, far_plane = 5.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	double yawMouse, pitchMouse;
	glfwGetCursorPos(myWindow.getWindow(), &yawMouse, &pitchMouse);
	myCamera.rotate(pitchMouse / 4, yawMouse / 4);
}

void processMovement() {
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_1)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_LINE_SMOOTH);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_2)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_3)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_4)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LINE_SMOOTH);
	}
	if (pressedKeys[GLFW_KEY_UP]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 0.01f;
	}
	if (pressedKeys[GLFW_KEY_E]) {
		angle += 0.01f;
	}
	if (pressedKeys[GLFW_KEY_H]) {
		pitch -= 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_K]) {
		pitch += 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_J]) {
		yaw -= 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_U]) {
		yaw += 0.3f;
		myCamera.rotate(yaw, pitch);
	}
	if (pressedKeys[GLFW_KEY_5]) {
		doorAngle += doorSpeed;
		if (doorAngle > 3.14f / 2.0 || doorAngle < 0.0f) {
			doorSpeed = -doorSpeed;
		}
	}
	if (pressedKeys[GLFW_KEY_6]) {
		carPosition -= carSpeed;
	}
	if (pressedKeys[GLFW_KEY_7]) {
		carPosition += carSpeed;
	}
	if (pressedKeys[GLFW_KEY_8]) {
		fogDensity += fogIncrease;

		if (fogDensity > 0.01f || fogDensity < 0.0f) {
			fogIncrease = -fogIncrease;
		}
		myBasicShader.useShaderProgram();
		glUniform1f(fogDensityLoc, fogDensity);
		reflectionShader.useShaderProgram();
		glUniform1f(fogDensityLoc, fogDensity);
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_9)) {
		myCamera = gps::Camera(
			glm::vec3(0.0f, 10.0f, 0.0f),
			glm::vec3(40.0f, 10.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		myCamera.resetCameraFrontDirection();
		pitch = 0.0f;
		yaw = 0.0f;
		angle = 0.0f;
		presentation = true;
		scene = 0;
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_0)) {
		presentation = false;
		scene = 0;
		pressedKeys[GLFW_KEY_S] = false;
		pressedKeys[GLFW_KEY_H] = false;
		pressedKeys[GLFW_KEY_D] = false;
		pressedKeys[GLFW_KEY_A] = false;
		pressedKeys[GLFW_KEY_UP] = false;
		pressedKeys[GLFW_KEY_6] = false;
		pressedKeys[GLFW_KEY_5] = false;
		pressedKeys[GLFW_KEY_7] = false;
		pressedKeys[GLFW_KEY_8] = false;
		pressedKeys[GLFW_KEY_J] = false;
		pressedKeys[GLFW_KEY_U] = false;
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_KP_7)) {
		itsRaining = true;
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_KP_8)) {
		itsRaining = false;
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_KP_4)) {
		headlights = 1;
	}
	if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_KP_5)) {
		headlights = 0;
	}
}

void animation() {
	if (presentation) {
		if (scene <= 35) {
			scene++;
			pressedKeys[GLFW_KEY_S] = true;
			pressedKeys[GLFW_KEY_H] = true;
			pressedKeys[GLFW_KEY_D] = true;
			itsRaining = true;
		}
		if (scene > 35 && scene <= 60) {
			scene++;
			pressedKeys[GLFW_KEY_S] = false;
			pressedKeys[GLFW_KEY_H] = false;
			pressedKeys[GLFW_KEY_D] = false;
			pressedKeys[GLFW_KEY_A] = true;
		}
		if (scene > 60 && scene <= 300) {
			scene++;
			pressedKeys[GLFW_KEY_A] = false;
		}
		if (scene > 300 && scene <= 550) {
			scene++;
			itsRaining = false;
			pressedKeys[GLFW_KEY_H] = true;
		}
		if (scene > 550 && scene <= 560) {
			scene++;
			pressedKeys[GLFW_KEY_H] = false;
			pressedKeys[GLFW_KEY_UP] = true;
			pressedKeys[GLFW_KEY_D] = true;
		}
		if (scene > 560 && scene <= 960) {
			scene++;
			pressedKeys[GLFW_KEY_D] = false;
			pressedKeys[GLFW_KEY_UP] = false;
			pressedKeys[GLFW_KEY_8] = true;
			if (scene < 760)
				pressedKeys[GLFW_KEY_6] = true;
			if (scene >= 760) {
				pressedKeys[GLFW_KEY_6] = false;
				pressedKeys[GLFW_KEY_7] = true;
			}
			pressedKeys[GLFW_KEY_5] = true;
		}
		if (scene > 960 && scene <= 1220) {
			scene++;
			pressedKeys[GLFW_KEY_5] = false;
			pressedKeys[GLFW_KEY_7] = false;
			pressedKeys[GLFW_KEY_8] = false;
			pressedKeys[GLFW_KEY_H] = true;
		}
		if (scene > 1220 && scene <= 1222) {
			scene++;
			pressedKeys[GLFW_KEY_H] = false;
		}
		if (scene > 1222 && scene <= 1600) {
			scene++;
			pressedKeys[GLFW_KEY_W] = true;
		}
		if (scene > 1600 && scene <= 1650) {
			scene++;
			pressedKeys[GLFW_KEY_W] = false;
		}
		if (scene > 1650 && scene <= 1940) {
			scene++;
			pressedKeys[GLFW_KEY_H] = true;
		}
		if (scene > 1940 && scene <= 1941) {
			scene++;
			pressedKeys[GLFW_KEY_H] = false;
		}
		if (scene > 1941 && scene <= 2570) {
			scene++;
			if (scene > 1941 && scene <= 2100)
				pressedKeys[GLFW_KEY_UP] = true;
			else
				pressedKeys[GLFW_KEY_UP] = false;
			pressedKeys[GLFW_KEY_W] = true;
		}
		if (scene > 2570 && scene <= 2770) {
			scene++;
			pressedKeys[GLFW_KEY_W] = false;
			pressedKeys[GLFW_KEY_J] = true;
			pressedKeys[GLFW_KEY_H] = true;
		}
		if (scene > 2770 && scene <= 2800) {
			scene++;
			pressedKeys[GLFW_KEY_J] = false;
			pressedKeys[GLFW_KEY_H] = false;
		}
		if (scene > 2800 && scene <= 2975) {
			scene++;
			pressedKeys[GLFW_KEY_U] = true;
		}
		if (scene > 2975 && scene <= 3150) {
			scene++;
			pressedKeys[GLFW_KEY_U] = false;
			pressedKeys[GLFW_KEY_H] = true;
		}
		if (scene > 3150 && scene <= 4020) {
			scene++;
			pressedKeys[GLFW_KEY_H] = false;
			pressedKeys[GLFW_KEY_W] = true;
		}
		if (scene > 4020 && scene <= 4400) {
			scene++;
			pressedKeys[GLFW_KEY_W] = false;
			pressedKeys[GLFW_KEY_H] = true;
		}
		if (scene > 4400 && scene <= 4500) {
			scene++;
			pressedKeys[GLFW_KEY_H] = false;
			pressedKeys[GLFW_KEY_UP] = true;
		}
		if (scene > 4500) {
			scene++;
			pressedKeys[GLFW_KEY_UP] = false;
			if (scene == 4550) {
				presentation = false;
				scene = 0;
			}
		}
	}
}

void initOpenGLWindow() {
	myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);	
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(myWindow.getWindow(), 960, 700);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void initModels() {
	teapot.LoadModel("models/teapot/teapot20segUT.obj");
	green.LoadModel("models/green/untitled.obj");
	airship.LoadModel("models/airship/airship.obj");
	propeller.LoadModel("models/airship/propeller1.obj");
	rightDoor.LoadModel("models/usa/rightdoor.obj");
	leftDoor.LoadModel("models/usa/leftdoor.obj");
	jeep.LoadModel("models/car/jeep.obj");
	frontRightWheel.LoadModel("models/car/wheel.obj");
	frontLeftWheel.LoadModel("models/car/wheel.obj");
	backRightWheel.LoadModel("models/car/wheel.obj");
	backLeftWheel.LoadModel("models/car/wheel.obj");
	rock.LoadModel("models/diamond/diamond.obj");
	swan.LoadModel("models/swan/swan.obj");
	rain.LoadModel("models/rain/rain.obj");
	building.LoadModel("models/building/building.obj");
	water.LoadModel("models/water/water.obj");
	peacock.LoadModel("models/peacock/peacock.obj");
}

void initShaders() {
	myBasicShader.loadShader(
		"shaders/basic.vert",
		"shaders/basic.frag");
	reflectionShader.loadShader(
		"shaders/reflection.vert",
		"shaders/reflection.frag");
	transparentShader.loadShader(
		"shaders/transparent.vert",
		"shaders/transparent.frag");

	faces.push_back("hills/hills_ft.tga");
	faces.push_back("hills/hills_bk.tga");
	faces.push_back("hills/hills_up.tga");
	faces.push_back("hills/hills_dn.tga");
	faces.push_back("hills/hills_rt.tga");
	faces.push_back("hills/hills_lf.tga");

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	mySkyBox.Load(faces);
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 10000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	myBasicShader.useShaderProgram();
	fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
	glUniform1f(fogDensityLoc, fogDensity);

	cameraPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "viewPos");
	glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));

	carPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "carPosition");
	glUniform1f(carPositionLoc, carPosition);

	headlightsLoc = glGetUniformLocation(myBasicShader.shaderProgram, "headlights");
	glUniform1i(headlightsLoc, headlights);


	reflectionShader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(reflectionShader.shaderProgram, "model");

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(reflectionShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(reflectionShader.shaderProgram, "normalMatrix");

	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 10000.0f);
	projectionLoc = glGetUniformLocation(reflectionShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(reflectionShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(reflectionShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	fogDensityLoc = glGetUniformLocation(reflectionShader.shaderProgram, "fogDensity");
	glUniform1f(fogDensityLoc, fogDensity);

	reflectFactorLoc = glGetUniformLocation(reflectionShader.shaderProgram, "reflectFactor");
	glUniform1f(reflectFactorLoc, reflectFactor);



	transparentShader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(transparentShader.shaderProgram, "model");

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(transparentShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(transparentShader.shaderProgram, "normalMatrix");

	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 10000.0f);
	projectionLoc = glGetUniformLocation(transparentShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(transparentShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(transparentShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	fogDensityLoc = glGetUniformLocation(transparentShader.shaderProgram, "fogDensity");
	glUniform1f(fogDensityLoc, fogDensity);

	reflectFactorLoc = glGetUniformLocation(transparentShader.shaderProgram, "reflectFactor");
	glUniform1f(reflectFactorLoc, reflectFactor);

	cameraPositionLoc = glGetUniformLocation(transparentShader.shaderProgram, "cameraPosition");
	glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(myCamera.getCameraTarget()));
}

void renderTeapot(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(95.0f, -0.5f, -185.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	teapot.Draw(shader);
}

void renderGreen(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	green.Draw(shader);
}

void renderAirship(gps::Shader shader) {
	double currentTimeStamp = glfwGetTime();
	updateDelta(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;

	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	glm::mat4 rotate, translate;
	translate = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f, 0.0f, 0.0f));
	rotate = glm::rotate(glm::mat4(1.0f), -delta, glm::vec3(0.0f, 1.0f, 0.0f));

	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 200.0f, 200.0f)) * rotate * translate;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	airship.Draw(shader);

	// back propeller left
	model = glm::mat4(1.0f);
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 200.0f, 200.0f)) * rotate * translate *
		glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, -19.0f, -77.0f)) *
		glm::rotate(glm::mat4(1.0f), 100 * delta, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	propeller.Draw(shader);

	// back propeller right
	model = glm::mat4(1.0f);
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 200.0f, 200.0f)) * rotate * translate *
		glm::translate(glm::mat4(1.0f), glm::vec3(13.0f, -19.0f, -77.0f)) *
		glm::rotate(glm::mat4(1.0f), 100 * delta, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	propeller.Draw(shader);

	// front propeller left
	model = glm::mat4(1.0f);
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 200.0f, 200.0f)) * rotate * translate *
		glm::translate(glm::mat4(1.0f), glm::vec3(-19.0f, -25.0f, -42.5f)) *
		glm::rotate(glm::mat4(1.0f), 100 * delta, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	propeller.Draw(shader);

	// front propeller right
	model = glm::mat4(1.0f);
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 200.0f, 200.0f)) * rotate * translate *
		glm::translate(glm::mat4(1.0f), glm::vec3(17.0f, -25.0f, -42.5f)) *
		glm::rotate(glm::mat4(1.0f), 100 * delta, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	propeller.Draw(shader);
}

void renderDoors(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(85.47f, 0.0f, -171.8f)) *
		glm::rotate(glm::mat4(1.0f), -doorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rightDoor.Draw(shader);

	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(85.47f, 0.0f, -171.8f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(-7.87f, 0.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), doorAngle + 3.14f, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	leftDoor.Draw(shader);
}

void renderJeep(gps::Shader shader) {
	reflectionShader.useShaderProgram();

	reflectFactor = 0.75f;
	glUniform1f(reflectFactorLoc, reflectFactor);

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	// jeep
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(4 * carPosition, 1.0f, -146.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	jeep.Draw(reflectionShader);


	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

	// front right wheel
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(4 * carPosition, 1.0f, -146.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(-4.8f, -1.6f, 2.9f)) *
		glm::rotate(glm::mat4(1.0f), -2.0f * carPosition, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	frontRightWheel.Draw(shader);

	// front left wheel
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(4 * carPosition, 1.0f, -146.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(-4.8f, -1.6f, -2.9f)) *
		glm::rotate(glm::mat4(1.0f), 3.14f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), 2.0f * carPosition, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	frontLeftWheel.Draw(shader);

	// back right wheel
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(4 * carPosition, 1.0f, -146.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(5.3f, -1.6f, 2.9f)) *
		glm::rotate(glm::mat4(1.0f), -2.0f * carPosition, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	backRightWheel.Draw(shader);

	// back left wheel
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(4 * carPosition, 1.0f, -146.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(5.3f, -1.6f, -2.9f)) *
		glm::rotate(glm::mat4(1.0f), 3.14f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), 2.0f * carPosition, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	backLeftWheel.Draw(shader);
}

void renderRock(gps::Shader shader) {
	shader.useShaderProgram();

	reflectFactor = 0.5f;
	glUniform1f(reflectFactorLoc, reflectFactor);

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 30.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rock.Draw(shader);

	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 50.0f, -100.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rock.Draw(shader);
}

void renderSwan(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	model =
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(-530.0f, -7.4f, -120.0f)) *
		glm::rotate(glm::mat4(1.0f), -3.0f * delta, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	swan.Draw(shader);
}

void renderRain(gps::Shader shader) {
	raining();

	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	model =
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, r1, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rain.Draw(shader);

	if (rAux) {
		model = glm::mat4(1.0f);
		modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
		model =
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, r2, 0.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		rain.Draw(shader);
	}
}

void renderBuild(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glEnable(GL_CULL_FACE);
	building.Draw(shader);
	glDisable(GL_CULL_FACE);
}

void renderWater(gps::Shader shader) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shader.useShaderProgram();

	reflectFactor = 0.5f;
	glUniform1f(reflectFactorLoc, reflectFactor);

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	water.Draw(shader);
	glDisable(GL_BLEND);
}

void renderPeacock(gps::Shader shader) {
	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(95.0f, -1.5f, -190.0f)) *
		glm::rotate(glm::mat4(1.0f), -1.57f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 0.15f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	peacock.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.8, 0.8, 0.8, 1.0);

	view = myCamera.getViewMatrix();
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	
	myBasicShader.useShaderProgram();	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	reflectionShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	transparentShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	myBasicShader.useShaderProgram();
	glUniform1f(carPositionLoc, carPosition);
	glUniform1i(headlightsLoc, headlights);

	view = glm::lookAt(glm::vec3(0.0f, 10.0f, 2.5f),
		glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	renderTeapot(myBasicShader);
	renderGreen(myBasicShader);
	renderAirship(myBasicShader);
	renderDoors(myBasicShader);
	renderJeep(myBasicShader);
	renderRock(reflectionShader);
	renderSwan(myBasicShader);
	renderBuild(myBasicShader);
	if (itsRaining) {
		renderRain(myBasicShader);
	}
	renderWater(transparentShader);
	renderPeacock(myBasicShader);

	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	myWindow.Delete();
}

int main(int argc, const char* argv[]) {
	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
	setWindowCallbacks();

	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();
		animation();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}
	cleanup();

	return EXIT_SUCCESS;
}