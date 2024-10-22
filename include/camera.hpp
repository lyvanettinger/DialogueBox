#pragma once

using namespace DirectX;

struct Camera
{
	XMVECTOR position = XMVectorSet(0, 0, -10, 1);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR front = XMVectorSet(0, 0, 10, 0);

	XMMATRIX model;
	XMMATRIX view;
	XMMATRIX projection;

	const FLOAT fov = 45.0f;
};