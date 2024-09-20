#pragma once
#include "pch.hpp"

#include <memory>

class Renderer;

class DialogueSample
{
public:
	DialogueSample(std::shared_ptr<Renderer> renderer);
	~DialogueSample();

	void Update();

private:
	std::shared_ptr<Renderer> _renderer;
};