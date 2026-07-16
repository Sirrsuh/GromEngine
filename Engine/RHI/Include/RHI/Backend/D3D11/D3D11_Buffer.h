#pragma once

#include "../../RHI_Buffer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>

namespace grom
{

class D3D11Buffer : public Buffer
{
public:
	D3D11Buffer() = default;
	~D3D11Buffer() override;

	void* GetHandle() override;
	BufferDesc& GetDesc() override;
	void Update(void* data, u32 size, u32 offset) override;
	void* Map() override;
	void Unmap() override;

	static D3D11Buffer* Create(BufferDesc& desc, ID3D11Device* device, ID3D11DeviceContext* context);

private:
	ID3D11Buffer* m_Buffer = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	BufferDesc m_Desc;
};

} // namespace grom
