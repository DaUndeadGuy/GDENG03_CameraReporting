#include "CameraManager.h"
#include "InputSystem.h"
#include "SceneCamera.h"
#include "Camera.h"
#include "Debug.h"

std::unique_ptr<CameraManager> CameraManager::sharedInstance = nullptr;

CameraManager* CameraManager::GetInstance()
{
	return sharedInstance.get();
}

void CameraManager::Initialize(UINT width, UINT height)
{
	try
	{
		sharedInstance.reset();
		sharedInstance = std::make_unique<CameraManager>(width, height);
	}
	catch (...)
	{
		Debug::LogError("Camera Manager creation failed!");
	}
}

void CameraManager::Destroy()
{
	sharedInstance.reset();
}

CameraManager::CameraManager(UINT width, UINT height)
{
	this->m_sceneCamera = std::make_shared<SceneCamera>(width, height);
	this->m_sceneCamera->SetPosition(0.0f, 0.0f, -10.0f);
	this->AddCamera(this->m_sceneCamera);
	this->PossessCamera(this->m_sceneCamera);

	std::shared_ptr<InputListener> listenerRef = std::dynamic_pointer_cast<InputListener>(this->m_sceneCamera);
	InputSystem::GetInstance()->AddListener(listenerRef.get());
	InputSystem::GetInstance()->AddListener(this);
}

void CameraManager::CameraSwitcher()
{

}

void CameraManager::CycleCameras()
{
	if (m_cameraList.size() == 1)
	{
		Debug::LogWarning("CameraManager::CycleCameras: No cameras available to cycle through.");
		return;
	}

	if (m_cameraCycleTracker >= m_cameraList.size())
	{
		m_cameraCycleTracker = 1; // Skip the scene camera
	}

	PossessCamera(m_cameraCycleTracker);
	m_cameraCycleTracker++;
}

void CameraManager::ResumePossess()
{
	if (m_previousCamera != nullptr)
	{
		m_activeCamera = m_previousCamera;
		m_possessionState = POSSESSED;
		Debug::Log("CameraManager::ResumePossess: Resumed possession of " + m_activeCamera->GetName());
	}
	else
	{
		Debug::LogWarning("CameraManager::ResumePossess: No previous camera to resume possession.");
	}
}

void CameraManager::CopyPositionToSceneCamera(const CameraPtr& reference)
{
	this->m_sceneCamera->SetPosition(reference->GetLocalPosition());
	this->m_sceneCamera->SetRotation(reference->GetLocalRotation());
}

int CameraManager::AddCamera(const CameraPtr& reference, bool setMain)
{
	this->m_cameraList.push_back(reference);
	this->m_cameraMap[this->m_cameraIndex] = reference;
	int index = static_cast<int>(this->m_cameraIndex);
	this->m_cameraIndex++;

	if (setMain)
	{
		this->PossessCamera(reference);
	}

	return index;
}

void CameraManager::Update(float deltaTime)
{
	Debug::Assert(m_activeCamera != nullptr, "No Active Camera Set!");
	CameraSwitcher();
	this->m_activeCamera->Update(deltaTime);
}

Matrix CameraManager::GetActiveCameraViewMatrix()
{
	Debug::Assert(m_activeCamera != nullptr, "No Active Camera Set!");
	return this->m_activeCamera->GetViewMatrix();
}

Matrix CameraManager::GetActiveCameraProjMatrix()
{
	Debug::Assert(m_activeCamera != nullptr, "No Active Camera Set!");
	return this->m_activeCamera->GetProjectionMatrix();
}

void CameraManager::UpdateViewportSize(UINT width, UINT height)
{
	for (const auto& camera : this->m_cameraList)
	{
		camera->SetViewportSize(width, height);
	}
}

void CameraManager::PossessCamera(int index)
{
	if (index >= m_cameraList.size())
	{
		Debug::LogWarning("Camera " + std::to_string(index) + " has not yet been set!");
		return;
	}

	m_previousCamera = m_activeCamera;
	m_activeCamera = this->m_cameraList[index];

	if (index != 0)
		m_possessionState = POSSESSED;

	Debug::Log("CameraManager::PossessCamera: Switched to camera at index " + std::to_string(index));
}

void CameraManager::PossessCamera(const CameraPtr& reference)
{
	for (const auto& [id, ref] : this->m_cameraMap)
	{
		if (ref == reference)
		{
			Debug::Log("CameraManager::PossessCamera: Possessing camera with ID " + std::to_string(id));
			PossessCamera(id);
			return;
		}
	}

	Debug::Log("CameraManager::PossessCamera: " + m_activeCamera->GetName() + " is not registered! ");
}

void CameraManager::UnpossessCamera()
{
	if (m_sceneCamera)
	{
		// By commenting this line out, the scene camera will no longer
		// copy the position of the camera you are unpossessing.
		// CopyPositionToSceneCamera(m_activeCamera); 

		m_previousCamera = m_activeCamera;
		m_activeCamera = m_sceneCamera;
		m_possessionState = UNPOSSESSED;
		Debug::Log("CameraManager::UnpossessCamera: Unpossessed " + m_previousCamera->GetName());
	}
	else
	{
		Debug::LogWarning("CameraManager::Unpossess: Scene camera not initialized.");
	}
}

const std::shared_ptr<Camera>& CameraManager::GetActiveCamera() const
{
	return this->m_activeCamera;
}

const std::shared_ptr<Camera>& CameraManager::GetSceneCamera() const
{
	return this->m_sceneCamera;
}

void CameraManager::OnKeyPressed(int key)
{
	switch (key)
	{
	case VK_TAB: CycleCameras(); break;
	case VK_F1: PossessCamera(1); break;
	case VK_F2: PossessCamera(2); break;
	case VK_F3: PossessCamera(3); break;
	case VK_F4: PossessCamera(4); break;
	case VK_F5: PossessCamera(5); break;
	case VK_ESCAPE:

		if (m_possessionState == POSSESSED)
		{
			UnpossessCamera();
		}
		else
		{
			ResumePossess();
		}

		break;

	default:
		break;
	}
}