#pragma once

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXTex/DirectXTex/DirectXTex.h>

#pragma warning(push)
#pragma warning(disable : 4324)
#include "d3dx12.h"
#pragma warning(pop)

// GLFW
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// commonly used
#include <string>
#include <memory>
#include <queue>
#include <stdlib.h>
#include <stdio.h>

// program specific
#define FRAME_COUNT 2
#define MAX_CBV_SRV_UAV_COUNT 256