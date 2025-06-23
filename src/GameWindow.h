#pragma once
#include "ABaseWindow.h"
#include <vector>
#include <memory>

// Forward-declare the Camera class to avoid including the full header
class Camera;

class GameWindow : public ABaseWindow
{
public:
	GameWindow(UINT width, UINT height);
	~GameWindow() = default;

	// Inherited via ABaseWindow
	void OnCreate(HWND hwnd) override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

private:
	float m_ticks = 0.0f;
	// Add this line to store the cameras
	std::vector<std::shared_ptr<Camera>> m_sceneCameras;
};