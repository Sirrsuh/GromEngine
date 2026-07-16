#include "RHI/Backend/D3D11/D3D11_Buffer.h"
#include "D3D11_Conversions.h"

namespace grom
{

D3D11Buffer::~D3D11Buffer()
{
	if (m_Buffer)
		m_Buffer->Release();
}

void* D3D11Buffer::GetHandle()
{
	return m_Buffer;
}

BufferDesc& D3D11Buffer::GetDesc()
{
	return m_Desc;
}

void D3D11Buffer::Update(void* data, u32 size, u32 offset)
{
	if (!data || size == 0)
		return;

	D3D11_BOX box{};
	box.left = offset;
	box.right = offset + size;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	m_Context->UpdateSubresource(m_Buffer, 0, &box, data, size, 0);
}

void* D3D11Buffer::Map()
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	HRESULT hr = m_Context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
		return nullptr;
	return mapped.pData;
}

void D3D11Buffer::Unmap()
{
	m_Context->Unmap(m_Buffer, 0);
}

D3D11Buffer* D3D11Buffer::Create(BufferDesc& desc, ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Buffer* buf = new D3D11Buffer();
	buf->m_Desc = desc;
	buf->m_Context = context;

	D3D11_BUFFER_DESC d3dDesc{};
	d3dDesc.ByteWidth = desc.Size;
	d3dDesc.Usage = ToD3DUsage(desc.Usage);
	d3dDesc.BindFlags = ToD3DBindFlags(desc.Type);
	d3dDesc.CPUAccessFlags = ToD3DCPUAccess(desc.Usage);
	d3dDesc.MiscFlags = 0;
	d3dDesc.StructureByteStride = 0;

	if (desc.Type == EBufferType::Structured)
	{
		d3dDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		d3dDesc.StructureByteStride = desc.Stride;
	}

	D3D11_SUBRESOURCE_DATA initdata{};
	D3D11_SUBRESOURCE_DATA* pInitData = nullptr;

	if (desc.InitialData && desc.Usage != EBufferUsage::Dynamic && desc.Usage != EBufferUsage::Staging)
	{
		initdata.pSysMem = desc.InitialData;
		initdata.SysMemPitch = 0;
		initdata.SysMemSlicePitch = 0;
		pInitData = &initdata;
	}

	HRESULT hr = device->CreateBuffer(&d3dDesc, pInitData, &buf->m_Buffer);
	if (FAILED(hr))
	{
		delete buf;
		return nullptr;
	}

	return buf;
}

}
