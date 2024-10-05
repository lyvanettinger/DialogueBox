#include "pch.hpp"

#include "glfw_app.hpp"
#include "renderer.hpp"
#include "dialogue_sample.hpp"

#include <memory>

std::shared_ptr<Application> g_app;
std::shared_ptr<Renderer> g_renderer;
std::unique_ptr<DialogueSample> g_sample;

int main()
{
	// TODO: input parameters for application window
	g_app = std::make_shared<Application>(800, 600, "DiaBolic");
	g_renderer = std::make_shared<Renderer>(g_app);
	g_sample = std::make_unique<DialogueSample>(g_renderer);

	while (!g_app->ShouldClose())
	{
		g_app->Update();
		g_sample->Update();
		g_renderer->Render();
	}
}