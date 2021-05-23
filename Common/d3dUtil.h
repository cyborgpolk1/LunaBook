#ifndef D3DUTIL_H
#define D3DUTIL_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDGB_MAP_ALLOC
#include <crtdbg.h>
#endif

#define _XM_NO_INTRINSICS_

#include <d3d11.h>
#include <sstream>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <d3dcompiler.h>

using namespace DirectX;

#include "dxerr.h"

#if defined(DEBUG) || defined(_DEBUG)

	#ifndef HR
	#define HR(x)				\
		{						\
			HRESULT hr = (x);	\
			if (FAILED(hr))		\
			{					\
				DXTrace((WCHAR*)__FILE__, (DWORD)__LINE__, hr, L#x, true);	\
			}					\
		}
	#endif 

#else

	#ifndef HR
	#define HR(x) x
	#endif

#endif	// defined(DEBUG)

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

#define SafeDelete(x) { delete x; x = 0; }

// Order: left, right, bottom, top, near, far
void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M);

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
};

namespace ShaderHelper
{
    ID3DBlob* CompileShader(LPCWSTR filename, LPCSTR entry, LPCSTR target, const D3D_SHADER_MACRO* defines);
    void CreateShader(ID3D11Device* device, ID3D11VertexShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines, ID3D11InputLayout** inputLayout, const D3D11_INPUT_ELEMENT_DESC* vertexDesc, UINT vertexDescSize);
    void CreateShader(ID3D11Device* device, ID3D11VertexShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines);
    void CreateShader(ID3D11Device* device, ID3D11PixelShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines);
    void CreateShader(ID3D11Device* device, ID3D11GeometryShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines);
    void CreateShader(ID3D11Device* device, ID3D11GeometryShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines, const D3D11_SO_DECLARATION_ENTRY* soEntries, UINT soEntriesSize);
    void CreateShader(ID3D11Device* device, ID3D11HullShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines);
    void CreateShader(ID3D11Device* device, ID3D11DomainShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines);
    void CreateShader(ID3D11Device* device, ID3D11ComputeShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines);
};

enum BasicEffectOptions
{
    STANDARD_LIGHTING = 0,
    USE_TEXTURES = 1,
    USE_ALPHA_CLIPPING = 2,
    USE_ENV_MAPPING = 4,
    USE_NORMAL_MAP = 8
};

std::wstring ExePath();

namespace D3DHelper
{
    ID3D11ShaderResourceView* CreateTexture2DArraySRV(ID3D11Device* device, ID3D11DeviceContext* context, std::vector<std::wstring>& filenames);
    ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);
};

#endif // D3DUTIL_H