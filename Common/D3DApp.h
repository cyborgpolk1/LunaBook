#ifndef D3DAPP_H
#define D3DAPP_H

#include <Windows.h>
#include <d3d11.h>
#include <string>

#include "GameTimer.h"
#include "d3dUtil.h"

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	int Run();

	// Framework methods. Derived client class overrides these methods to
	// implement specific application requirements.

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam);

	// Convenience overrides for handling mouse input
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CalculateFrameStats();

	void CreateShader(ID3D11VertexShader** shader, LPCWSTR filename, LPCSTR entry, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* vertexDesc, UINT vertexDescSize);
	void CreateShader(ID3D11PixelShader** shader, LPCWSTR filename, LPCSTR entry);
	void CreateShader(ID3D11GeometryShader** shader, LPCWSTR filename, LPCSTR entry);
	void CreateShader(ID3D11HullShader** shader, LPCWSTR filename, LPCSTR entry);
	void CreateShader(ID3D11DomainShader** shader, LPCWSTR filename, LPCSTR entry);
	void CreateShader(ID3D11ComputeShader** shader, LPCWSTR filename, LPCSTR entry);

protected:
	HINSTANCE	mhAppInst;	// application instance handle
	HWND		mhMainWnd;	// main window handle
	bool		mAppPaused;	// is the application paused?
	bool		mMinimized;	// is the application minimized?
	bool		mMaximized;	// is the application maximized?
	bool		mResizing;	// are the resize bars being dragged?
	UINT		m4xMsaaQuality; // quality level of 4X MSAA

	// Used to keep track of the "delta-time" and game time
	GameTimer mTimer;

	// The D3D11 device, the swap chain for page flipping,
	// the 2D texture for the depth/stencil buffer,
	// the render target and depth/stencil views, 
	// and the viewport

	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	// The following variables are initialized in the D3DApp constructor
	// to default values. However, you can override the values in the
	// derived class constructor to pick different defaults.

	// Window title/caption. D3DApp defaults to "D3D11 Application".
	std::wstring mMainWndCaption;

	// Initial size of the window's client area. D3DApp defaults to
	// 800x600. Note, however, that these values change at runtime
	// to reflect the current client area size as the window is resized.
	int mClientWidth;
	int mClientHeight;

	// Hardware device or reference device? D3DApp defaults to
	// D3D_DRIVER_TYPE_HARDWARE.
	D3D_DRIVER_TYPE md3dDriverType;

	// True to use 4X MSAA. The default is false.
	bool mEnable4xMsaa;
};

#endif