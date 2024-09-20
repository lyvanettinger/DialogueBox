#include "glfw_app.hpp"

#include <stdlib.h>
#include <stdio.h>

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

Application::Application(UINT width, UINT height, std::string name) :
	_width(width),
	_height(height),
	_name(name)
{
	// initialize glfw
	bool success = glfwInit();
	if (!success)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// set the error callback
	glfwSetErrorCallback(ErrorCallback);

	// set window hints
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE); // we prefer double buffering
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // since we're only working with DX12, no context is needed

	// create window according to preference
#ifdef FULL_SCREEN
			// set monitor
	_monitor = glfwGetPrimaryMonitor();
	_window = glfwCreateWindow(_width, _height, _name.c_str(), _monitor, NULL);
#elif WINDOWED_FULL_SCREEN
	_monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(_monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	_window = glfwCreateWindow(_width, _height, _name.c_str(), _monitor, NULL);
#else
	_window = glfwCreateWindow(_width, _height, _name.c_str(), _monitor, NULL);
#endif

	if (!_window)
	{
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	if (_monitor)
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(_window, KeyCallback);

	glfwSetTime(0.0);

	_hwnd = glfwGetWin32Window(_window);
}

Application::~Application()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Application::Update()
{
	glfwPollEvents();
}

bool Application::ShouldClose()
{
	return glfwWindowShouldClose(_window);
}