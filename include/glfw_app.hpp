#pragma once
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

class Application
{
public:
	Application();
	~Application();

	void Update();
	void* GetWindow();
	bool ShouldClose();

private:
	GLFWmonitor* _monitor = NULL;
	GLFWwindow* _window = NULL;
};