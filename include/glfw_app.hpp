#pragma once

class Application
{
public:
	Application(UINT width, UINT height, std::string name);
	~Application();

	void Update();
	void* GetWindow() { return _window; }
	HWND GetHWND() { return _hwnd; }
	bool ShouldClose();

	UINT GetWidth() { return _width; }
	UINT GetHeight() { return _height; }
	std::string GetName() { return _name; }

private:
	UINT _width, _height;
	std::string _name;
	GLFWmonitor* _monitor = NULL;
	GLFWwindow* _window = NULL;
	HWND _hwnd = NULL;
};