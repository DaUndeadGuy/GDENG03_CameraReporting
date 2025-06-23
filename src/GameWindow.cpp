#include "GameWindow.h"

#include "GraphicsEngine.h"
#include "GameObjectManager.h"
#include "EngineTime.h"
#include "InputSystem.h"
#include "CameraManager.h"

#include "RenderSystem.h"
#include "BatchUploader.h"

#include "Cube.h"
#include "Plane.h"
#include "Sphere.h"
#include "Camera.h"

#include "Debug.h"
#include "Random.h"
#include "Colors.h"

GameWindow::GameWindow(UINT width, UINT height) : ABaseWindow(width, height) {}

void GameWindow::OnCreate(HWND hwnd)
{
	GraphicsEngine::Initialize(this->m_width, this->m_height, hwnd);
	GameObjectManager::Initialize(GraphicsEngine::GetInstance()->GetRenderSystem()->GetD3DDevicePtr().Get());
	EngineTime::Initialize(60);
	InputSystem::Initialize();
	CameraManager::Initialize(this->m_width, this->m_height);

	auto renderSystem = GraphicsEngine::GetInstance()->GetRenderSystem();

	GraphicsEngine::GetInstance()->GetBatchUploader()->StartUpload();

	ColorPalette palette;

	auto sphere = std::make_shared<Sphere>("Sphere", palette.Orange);
	sphere->SetPosition(0.0f, 5.0f, 0.0f);
	sphere->SetScale(5.0f, 5.0f, 5.0f);
	GameObjectManager::GetInstance()->AddGameObject(sphere);

	auto plane = std::make_shared<Plane>("Plane");
	plane->SetRotation(0.0f, 0.0f, 0.0f);
	plane->SetScale(3.0f, 3.0f, 3.0f);
	GameObjectManager::GetInstance()->AddGameObject(plane);

	auto colors = palette.GetPalette();
	const float radius = 10.0f;
	const float angleStep = 45.0f;

	for (int i = 0; i < 8; ++i)
	{
		float angleDeg = i * angleStep;
		float angleRad = DirectX::XMConvertToRadians(angleDeg);

		float x = radius * cos(angleRad);
		float z = radius * sin(angleRad);

		auto cube = std::make_shared<Cube>("Cube_" + std::to_string(i), colors[i]);
		cube->SetPosition(x, 0.5f, z);

		GameObjectManager::GetInstance()->AddGameObject(cube);
	}

	std::vector<Vector3> positions = {
	{ 20.0f, 10.0f,  0.0f },  // +X
	{-20.0f, 10.0f,  0.0f },  // -X
	{ 0.0f, 10.0f, 20.0f },   // +Z
	{ 0.0f, 10.0f, -20.0f },   // -Z
	};

	for (int i = 0; i < positions.size(); ++i)
	{
		auto cam = std::make_shared<Camera>("Camera_" + std::to_string(i), 1024, 768);
		cam->SetPosition(positions[i]);
		cam->SetLookAt(Vector3(0.0f, 0.0f, 0.0f));
		CameraManager::GetInstance()->AddCamera(cam);
		m_sceneCameras.push_back(cam);

		auto cameraCube = std::make_shared<Cube>("CameraCube_" + std::to_string(i), ColorPalette::Blue);
		cameraCube->SetScale(0.5f, 0.5f, 0.5f);
		GameObjectManager::GetInstance()->AddGameObject(cameraCube);
	}

	auto topDownCam = std::make_shared<Camera>("TopDownCamera", 1024, 768);
	topDownCam->SetPosition(0.0f, 30.0f, 0.0f);
	topDownCam->SetLookAt(Vector3(0.0f, 0.0f, 0.0f));
	CameraManager::GetInstance()->AddCamera(topDownCam);
	m_sceneCameras.push_back(topDownCam);

	auto topDownCameraCube = std::make_shared<Cube>("CameraCube_4", ColorPalette::Green);
	topDownCameraCube->SetScale(0.5f, 0.5f, 0.5f);
	GameObjectManager::GetInstance()->AddGameObject(topDownCameraCube);


	auto sceneCamCube = std::make_shared<Cube>("SceneCameraCube", Vector3(0.0f, 0.0f, 0.0f));
	sceneCamCube->SetScale(0.5f, 0.5f, 0.5f);
	GameObjectManager::GetInstance()->AddGameObject(sceneCamCube);

	GraphicsEngine::GetInstance()->GetBatchUploader()->StopAndWaitUpload();
}

void GameWindow::OnUpdate()
{
	auto deltaTime = EngineTime::GetDeltaTime();
	m_ticks += deltaTime;

	InputSystem::GetInstance()->ProcessInput();

	CameraManager::GetInstance()->Update(deltaTime);
	GameObjectManager::GetInstance()->UpdateAll(deltaTime);


	// --- FIXED CAMERA CUBE LOGIC ---
	// First, update all cube positions and set them to be visible by default.
	auto sceneCamera = CameraManager::GetInstance()->GetSceneCamera();
	auto sceneCameraCube = GameObjectManager::GetInstance()->FindObjectByName("SceneCameraCube");
	if (sceneCamera && sceneCameraCube)
	{
		sceneCameraCube->SetPosition(sceneCamera->GetLocalPosition());
		sceneCameraCube->SetRotation(sceneCamera->GetLocalRotation());
		sceneCameraCube->SetActive(true);
	}

	for (size_t i = 0; i < m_sceneCameras.size(); ++i)
	{
		auto camera = m_sceneCameras[i];
		auto cube = GameObjectManager::GetInstance()->FindObjectByName("CameraCube_" + std::to_string(i));
		if (camera && cube)
		{
			cube->SetPosition(camera->GetLocalPosition());
			cube->SetRotation(camera->GetLocalRotation());
			cube->SetActive(true);
		}
	}

	// Second, find the ONE cube that belongs to the ACTIVE camera and hide it.
	auto activeCamera = CameraManager::GetInstance()->GetActiveCamera();
	if (activeCamera == sceneCamera)
	{
		if (sceneCameraCube)
		{
			sceneCameraCube->SetActive(false);
		}
	}
	else
	{
		for (size_t i = 0; i < m_sceneCameras.size(); ++i)
		{
			if (m_sceneCameras[i] == activeCamera)
			{
				auto cubeToHide = GameObjectManager::GetInstance()->FindObjectByName("CameraCube_" + std::to_string(i));
				if (cubeToHide)
				{
					cubeToHide->SetActive(false);
				}
				break;
			}
		}
	}
	// --- END OF FIX ---


	FrameConstantsData frameData = {};
	frameData.viewMatrix = CameraManager::GetInstance()->GetActiveCameraViewMatrix();
	frameData.projMatrix = CameraManager::GetInstance()->GetActiveCameraProjMatrix();;

	GraphicsEngine::GetInstance()->GetRenderSystem()->UpdateFrameConstants(frameData);
}

void GameWindow::OnRender()
{
	auto context = GraphicsEngine::GetInstance()->GetRenderSystem()->GetDeviceContext();

	GraphicsEngine::GetInstance()->GetRenderSystem()->BeginFrame();

	GameObjectManager::GetInstance()->RenderAll(context);

	GraphicsEngine::GetInstance()->GetRenderSystem()->EndFrame();
}

void GameWindow::OnDestroy()
{
	CameraManager::Destroy();
	InputSystem::Destroy();
	GameObjectManager::Destroy();
	GraphicsEngine::Destroy();
}