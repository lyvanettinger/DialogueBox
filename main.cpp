#include "glfw_app.hpp"
#include "renderer.hpp"

#include <memory>

std::shared_ptr<Application> g_app;
std::shared_ptr<Renderer> g_renderer;

int main(int argc, char** argv)
{
	// TODO: input parameters
	g_app = std::make_shared<Application>();
	g_renderer = std::make_shared<Renderer>(g_app->GetWindow());

	while (!g_app->ShouldClose())
	{
		g_app->Update();
	}
}