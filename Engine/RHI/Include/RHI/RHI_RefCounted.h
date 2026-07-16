#pragma once

#include "RHI_Enums.h"

#ifndef GROM_RHI_API
#define GROM_RHI_API
#endif

namespace grom
{

class RefCounted
{
public:
	RefCounted() = default;
	RefCounted(const RefCounted&) = delete;
	RefCounted& operator=(const RefCounted&) = delete;

	virtual ~RefCounted() = default;

	void AddRef()
	{
		++m_RefCount;
	}

	void Release()
	{
		if (--m_RefCount <= 0)
		{
			delete this;
		}
	}

protected:
	i32 m_RefCount = 0;
};

} // namespace grom
