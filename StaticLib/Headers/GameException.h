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

	#define THROW_GAME_EXCEPTION(str_, hr_) throw GameException(std::string(str_) + "\n at " + std::string(__FILE__) + ":" + std::to_string(__LINE__), hr_);
	#define THROW_GAME_EXCEPTION_SIMPLE(str_) throw GameException(std::string(str_) + "\n at " + std::string(__FILE__) + ":" + std::to_string(__LINE__));
}