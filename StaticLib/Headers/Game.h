#pragma once

#include <windows.h>
#include <string>
#include "GameClock.h"
#include "GameTime.h"
#include "D3DApp.h"

using namespace Library;

namespace Library
{
	class Game
	{
	public:
		Game(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
		~Game();

		static Game* GetGame() { return m_game; }

		HINSTANCE Instance() const;
		HWND WindowHandle() const;
		const WNDCLASSEX& Window() const;
		const std::wstring& WindowClass() const;
		const std::wstring& WindowTitle() const;
		int ScreenWidth() const;
		int ScreenHeight() const;

		virtual void Run();
		virtual void Exit();
		virtual void Initialize();
		virtual void Update(const GameTime& gameTime);
		virtual void Draw(const GameTime& gameTime);

		void OnMouseButtonDown(int x, int y);
		void OnMouseMoved(WPARAM btnState, int x, int y);
	protected:
		virtual void InitializeWindow();
		virtual void Shutdown();

		static Game* m_game;

		static const UINT DefaultScreenWidth;
		static const UINT DefaultScreenHeight;

		HINSTANCE mInstance;
		std::wstring mWindowClass;
		std::wstring mWindowTitle;
		int mShowCommand;

		HWND mWindowHandle;
		WNDCLASSEX mWindow;

		UINT mScreenWidth;
		UINT mScreenHeight;

		GameClock mGameClock;
		GameTime mGameTime;

		std::unique_ptr<D3DApp> m_d3dApp;

		int m_mouseLastX;
		int m_mouseLastY;

	private:
		Game(const Game& rhs);
		Game& operator=(const Game& rhs);

		POINT CenterWindow(int windowWidth, int windowHeight);
		static LRESULT WINAPI WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	};
}