#pragma once
#include "AGameObject.h"
#include "InputListener.h"
#include "Math.h"

class DeviceContext;

class Camera : public AGameObject 
{
public:
	Camera(String name, UINT viewportWidth, UINT viewportHeight);
	~Camera() = default;

	void Update(float deltaTime) override;

	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();

	void SetViewportSize(UINT width, UINT height);
	void SetLookAt(Vector3 lookat);

	bool IsPerspectiveView() const { return m_isPerspectiveView; }

private:
	bool m_isPerspectiveView = true;
	float m_deltaTime = 0.0f;

	/* Perspective */
	float m_fov = 45.0f;
	float m_aspectRatio = 0.0f;
	float m_pNearZ = 0.01f;
	float m_pFarZ = 100.0f;

	/* Orthographic */
	UINT m_viewportWidth = 0;
	UINT m_viewportHeight = 0;
	float m_oNearZ = -4.0f;
	float m_oFarZ = 4.0f;

private:
	void Draw(DeviceContext* dvcContext) override;
};