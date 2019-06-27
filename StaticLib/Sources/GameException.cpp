#include "stdafx.h"

#include "GameException.h"

#include <comdef.h>

namespace Library
{
	GameException::GameException(const char* const& message, HRESULT hr)
		: exception(message), mHR(hr)
	{
	}

	HRESULT GameException::HR() const
	{
		return mHR;
	}

	std::wstring GameException::whatw() const
	{
		_com_error err(mHR);
		std::string whatString(what());
		std::wstring whatw;
		whatw.assign(whatString.begin(), whatString.end());
		whatw += L"\n";
		whatw += err.ErrorMessage();

		return whatw;
	}
}