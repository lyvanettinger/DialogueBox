#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdlib.h>
#include <stdio.h>

#include "renderer.hpp"

void ErrorCallback(int error, const char* description)
{
	fputs(description, stderr);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		default:
			break;
		}
	}
}

static void OnResizeFrame(GLFWwindow* window, int width, int height)
{
	// TODO: implement
}

int main(int argc, char** argv)
{
	const int width = 800, height = 600;

	// initialize glfw
	bool success = glfwInit();
	if (!success)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	GLFWmonitor* monitor = NULL;

	// set the error callback
	glfwSetErrorCallback(ErrorCallback);

	// set window hints
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE); // we prefer double buffering
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // since we're only working with DX12, no context is needed

	// create window according to preference
#ifdef FULL_SCREEN
			// set monitor
	monitor = glfwGetPrimaryMonitor();
	m_window = glfwCreateWindow(width, height, "DiaBolic", monitor, NULL);
#elif WINDOWED_FULL_SCREEN
	monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "DiaBolic", monitor, NULL);
#else
	auto window = glfwCreateWindow(width, height, "DiaBolic", monitor, NULL);
#endif

	if (!window)
	{
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	if (monitor)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(window, KeyCallback);

	glfwSetTime(0.0);

	// for directx
	auto mainWindow = glfwGetWin32Window(window);

	Renderer* renderer = new Renderer(mainWindow);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		renderer->Render();
	}

	delete renderer;

	glfwDestroyWindow(window);
	glfwTerminate();
}