// Application.cpp : Defines the entry point for the application.
//

#include "stdafx.h"


#include "Game.h"
#include "GameException.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace Library;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE,
                     _In_ LPWSTR,
                     _In_ int       nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::unique_ptr<Game> game(new Game(hInstance, L"RenderingClass",
		L"Real-Time 3D Rendering", nCmdShow));

	try
	{
		game->Run();
	}
	catch (GameException ex)
	{
		MessageBox(game->WindowHandle(), ex.whatw().c_str(), game->WindowTitle().c_str(), MB_ABORTRETRYIGNORE);
	}

	return 0;
}
