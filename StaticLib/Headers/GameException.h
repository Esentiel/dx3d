#pragma once

#include <exception>
#include <Windows.h>
#include <string>

namespace Library
{
	class GameException : public std::exception
	{
	public:
		GameException(std::string message, HRESULT hr = S_OK);

		HRESULT HR() const;
		std::wstring whatw() const;

	private:
		HRESULT mHR;
	};
}