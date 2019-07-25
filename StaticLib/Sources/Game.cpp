<<<<<<< HEAD
#include "stdafx.h"

#include <Windowsx.h>

#include "Game.h"
#include "Mesh.h"
#include "FileManager.h"
#include "RenderScene.h"
#include "Camera.h"

namespace Library
{
	const UINT Game::DefaultScreenWidth = 1280;
	const UINT Game::DefaultScreenHeight = 960;

	Game::Game(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
		: mInstance(instance), 
		mWindowClass(windowClass), 
		mWindowTitle(windowTitle), 
		mShowCommand(showCommand),
		mScreenWidth(DefaultScreenWidth), 
		mScreenHeight(DefaultScreenHeight)
	{
		assert(m_game == nullptr);
		m_game = this;
	}

	Game::~Game()
	{
	}

	HINSTANCE Game::Instance() const
	{
		return mInstance;
	}

	HWND Game::WindowHandle() const
	{
		return mWindowHandle;
	}

	const WNDCLASSEX& Game::Window() const
	{
		return mWindow;
	}

	const std::wstring& Game::WindowClass() const
	{
		return mWindowClass;
	}

	const std::wstring& Game::WindowTitle() const
	{
		return mWindowTitle;
	}

	int Game::ScreenWidth() const
	{
		return mScreenWidth;
	}

	int Game::ScreenHeight() const
	{
		return mScreenHeight;
	}

	void Game::Run()
	{
		InitializeWindow();
		Initialize();

		MSG message;
		ZeroMemory(&message, sizeof(message));

		mGameClock.Reset();

		while (message.message != WM_QUIT)
		{
			if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{
				mGameClock.UpdateGameTime(mGameTime);
				Update(mGameTime);
				Draw(mGameTime);
			}
		}

		Shutdown();
	}

	void Game::Exit()
	{
		PostQuitMessage(0);
	}

	void Game::Initialize()
	{
		m_d3dApp.reset(new D3DApp(mWindowHandle, mScreenWidth, mScreenHeight));
		m_d3dApp->Initialize();

		// create test meshes
		std::string filePath = "C:\\Users\\aburs.DESKTOP-DI240N9\\Documents\\box.fbx";
=======
#include "stdafx.h"

#include <Windowsx.h>

#include "Game.h"
#include "Mesh.h"
#include "FileManager.h"
#include "RenderScene.h"
#include "Camera.h"

namespace Library
{
	const UINT Game::DefaultScreenWidth = 1280;
	const UINT Game::DefaultScreenHeight = 960;

	Game::Game(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
		: mInstance(instance), 
		mWindowClass(windowClass), 
		mWindowTitle(windowTitle), 
		mShowCommand(showCommand),
		mScreenWidth(DefaultScreenWidth), 
		mScreenHeight(DefaultScreenHeight)
	{
		assert(m_game == nullptr);
		m_game = this;
	}

	Game::~Game()
	{
	}

	HINSTANCE Game::Instance() const
	{
		return mInstance;
	}

	HWND Game::WindowHandle() const
	{
		return mWindowHandle;
	}

	const WNDCLASSEX& Game::Window() const
	{
		return mWindow;
	}

	const std::wstring& Game::WindowClass() const
	{
		return mWindowClass;
	}

	const std::wstring& Game::WindowTitle() const
	{
		return mWindowTitle;
	}

	int Game::ScreenWidth() const
	{
		return mScreenWidth;
	}

	int Game::ScreenHeight() const
	{
		return mScreenHeight;
	}

	void Game::Run()
	{
		InitializeWindow();
		Initialize();

		MSG message;
		ZeroMemory(&message, sizeof(message));

		mGameClock.Reset();

		while (message.message != WM_QUIT)
		{
			if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{
				mGameClock.UpdateGameTime(mGameTime);
				Update(mGameTime);
				Draw(mGameTime);
			}
		}

		Shutdown();
	}

	void Game::Exit()
	{
		PostQuitMessage(0);
	}

	void Game::Initialize()
	{
		m_d3dApp.reset(new D3DApp(mWindowHandle, mScreenWidth, mScreenHeight));
		m_d3dApp->Initialize();

		// create test meshes
		std::string filePath = "../Content/models/Arissa.fbx";
>>>>>>> 0b25efb1998418c9b6b1b8cc4f64685fb169e376
		uint32_t numMesh = 0;
		uint32_t meshID = 0;
		do
		{
			std::unique_ptr<Mesh> mesh(new Mesh);
			bool res = g_D3D->fileMgr->ReadModelFromFBX(filePath.c_str(), meshID, mesh.get(), &numMesh);

			if (res)
			{
				mesh->Initialize();
				mesh->LoadVertexDataBuffer();
				mesh->Scale(DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f));
				g_D3D->renderScene->AddMesh(std::move(mesh));
			}
			meshID++;
		} while (meshID < numMesh);
		
		// box
		filePath = "../Content/models/box_ddc_true.fbx";
		numMesh = 0;
		meshID = 0;
		do
		{
			std::unique_ptr<Mesh> mesh(new Mesh);
			bool res = g_D3D->fileMgr->ReamModelFromFBX(filePath.c_str(), meshID, mesh.get(), &numMesh);

			if (res)
			{
				mesh->Initialize();
				mesh->LoadVertexDataBuffer();
				mesh->Scale(DirectX::XMFLOAT3(2.f, 2.f, 2.f));
				g_D3D->renderScene->AddMesh(std::move(mesh));
			}
			meshID++;
		} while (meshID < numMesh);

	}

	void Game::Update(const GameTime& gameTime)
	{
		auto delta = gameTime.ElapsedGameTime();
		std::string deltaText = std::to_string(1.0f/delta);
		std::string title("FPS: ");
		title.insert(title.cend(), deltaText.cbegin(), deltaText.cend());
		SetWindowTextA(mWindowHandle, title.c_str());

		// camera
		if (g_D3D && g_D3D->camera)
			g_D3D->camera->UpdateViewMatrix();
	}

	void Game::Draw(const GameTime& gameTime)
	{
		m_d3dApp->Draw(gameTime);
	}


	void Game::OnMouseButtonDown(int x, int y)
	{
		m_mouseLastX = x;
		m_mouseLastY = y;
	}


	void Game::OnMouseMoved(WPARAM btnState, int x, int y)
	{
		if ((btnState & MK_RBUTTON) != 0)
		{
			float dx = DirectX::XMConvertToRadians(0.25f*static_cast<float>(x - m_mouseLastX));
			float dy = DirectX::XMConvertToRadians(0.25f*static_cast<float>(y - m_mouseLastY));

			g_D3D->camera->Pitch(dy);
			g_D3D->camera->RotateY(-dx);
		}

		m_mouseLastX = x;
		m_mouseLastY = y;
	}

	void Game::InitializeWindow()
	{
		ZeroMemory(&mWindow, sizeof(mWindow));
		mWindow.cbSize = sizeof(WNDCLASSEX);
		mWindow.style = CS_CLASSDC;
		mWindow.lpfnWndProc = WndProc;
		mWindow.hInstance = mInstance;
		mWindow.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		mWindow.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
		mWindow.hCursor = LoadCursor(nullptr, IDC_ARROW);
		mWindow.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
		mWindow.lpszClassName = mWindowClass.c_str();

		RECT windowRectangle = { 0L, 0L, (LONG)mScreenWidth, (LONG)mScreenHeight };
		AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

		RegisterClassEx(&mWindow);
		POINT center = CenterWindow(mScreenWidth, mScreenHeight);
		mWindowHandle = CreateWindow(mWindowClass.c_str(), mWindowTitle.c_str(), WS_OVERLAPPEDWINDOW, center.x, center.y, windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, nullptr, nullptr, mInstance, nullptr);

		ShowWindow(mWindowHandle, mShowCommand);
		UpdateWindow(mWindowHandle);
	}

	void Game::Shutdown()
	{
		UnregisterClass(mWindowClass.c_str(), mWindow.hInstance);
	}


	Library::Game* Game::m_game = nullptr;

	LRESULT WINAPI Game::WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
		{
			//bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

			switch (wParam)
			{
			case 'A':
			case VK_LEFT:
				g_D3D->camera->Move(Camera::MoveDir::Left);
				break;
			case 'D':
			case VK_RIGHT:
				g_D3D->camera->Move(Camera::MoveDir::Right);
				break;
			case 'W':
			case VK_UP:
				g_D3D->camera->Move(Camera::MoveDir::Forward);
				break;
			case 'S':
			case VK_DOWN:
				g_D3D->camera->Move(Camera::MoveDir::Backward);
				break;
			case 'Q':
				g_D3D->camera->Move(Camera::MoveDir::Down);
				break;
			case 'E':
				g_D3D->camera->Move(Camera::MoveDir::Up);
				break;
			}
			break;
		}
		case WM_RBUTTONDOWN:
			Game::GetGame()->OnMouseButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_MOUSEMOVE:
			Game::GetGame()->OnMouseMoved(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		}

		return DefWindowProc(windowHandle, message, wParam, lParam);
	}

	POINT Game::CenterWindow(int windowWidth, int windowHeight)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		POINT center;
		center.x = (screenWidth - windowWidth) / 2;
		center.y = (screenHeight - windowHeight) / 2;

		return center;
	}
}