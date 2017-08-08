#ifndef COMMONLIBS_H
#define COMMONLIBS_H

#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined(DEBUG) || defined(_DEBUG)
#define D3DMAIN(X) \
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) \
	{ \
		\
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
		\
		X theApp(hInstance); \
		\
		if (!theApp.Init()) \
		return 0; \
		\
		return theApp.Run(); \
	} 
#else
#define D3DMAIN(X) \
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) \
	{ \
		X theApp(hInstance); \
		\
		if (!theApp.Init()) \
		return 0; \
		\
		return theApp.Run(); \
	} 
#endif

#endif