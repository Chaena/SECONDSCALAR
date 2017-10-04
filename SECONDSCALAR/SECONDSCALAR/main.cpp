#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <time.h>
#include <stdlib.h>
#include "main.h"

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

IDXGISwapChain *swapchain;
ID3D11Device *device;
ID3D11DeviceContext *dcontext;
ID3D11RenderTargetView *bbuf;
ID3D11InputLayout *layout;

ID3D11Buffer *vbufptr;

ID3D11VertexShader *vsptr;
ID3D11PixelShader *psptr;

const bool isAdjust = false;

struct VERTEX {
	FLOAT x, y, z;
	D3DXCOLOR Color;
};

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	WNDCLASSEX wc;
	RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	if (isAdjust) AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "WindowClass1";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(
		NULL,
		"WindowClass1",			// classname
		"Window",				// title
		WS_OVERLAPPEDWINDOW,	// style
		300,					// x
		300,					// y
		wr.right-wr.left,		// w
		wr.bottom-wr.top,		// h
		NULL,					// parent
		NULL,					// menus
		hInst,					// handle
		NULL					// multiple
		);

	ShowWindow(hWnd, nCmdShow);

	InitD3D(hWnd);

	MSG msg;

	float start = (float)clock();

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
		} RenderFrame((double)(clock()-start)/CLOCKS_PER_SEC);
	}
	CleanD3D();
	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		//MessageBox(NULL, "Hello world", "Hello world", MB_OK | MB_ICONEXCLAMATION);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

VERTEX ConstructVertex(float x, float y, float z, D3DXCOLOR color) {
	VERTEX vret;
	vret.x = x;
	vret.y = y;
	vret.z = z;
	vret.Color = color;
	return vret;
}

void InitPipeline() {
	ID3D10Blob *vs, *ps;
	D3DX11CompileFromFile("shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &vs, 0, 0);
	D3DX11CompileFromFile("shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &ps, 0, 0);

	device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &vsptr);
	device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &psptr);

	dcontext->VSSetShader(vsptr, 0, 0);
	dcontext->PSSetShader(psptr, 0, 0);

	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device->CreateInputLayout(ied, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &layout);
	dcontext->IASetInputLayout(layout);
}

void InitGraphics() {
	VERTEX vertices[] = {
	ConstructVertex(0.0f, 0.5f, 0.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)),
	ConstructVertex(0.5f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)),
	ConstructVertex(-0.5f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f))
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&bd, NULL, &vbufptr);

	D3D11_MAPPED_SUBRESOURCE map;
	dcontext->Map(vbufptr, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &map);
	memcpy(map.pData, vertices, sizeof(vertices));
	dcontext->Unmap(vbufptr, NULL);
}

void InitD3D(HWND hWnd) {
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;									// back buffers
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// color type
	scd.BufferDesc.Width = SCREEN_WIDTH;					// width
	scd.BufferDesc.Height = SCREEN_HEIGHT;					// height
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// swap chain usage
	scd.OutputWindow = hWnd;								// window
	scd.SampleDesc.Count = 4;								// multisampling
	scd.Windowed = TRUE;									// is windowed
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;		// allow switch

	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&device,
		NULL,
		&dcontext
	);

	ID3D11Texture2D *bbufptr;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&bbufptr);
	device->CreateRenderTargetView(bbufptr, NULL, &bbuf);
	bbufptr->Release();
	dcontext->OMSetRenderTargets(1, &bbuf, NULL);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	dcontext->RSSetViewports(1, &viewport);

	InitPipeline();
	InitGraphics();
}

void CleanD3D() {
	swapchain->SetFullscreenState(FALSE, NULL);
	layout->Release();
	vsptr->Release();
	psptr->Release();
	swapchain->Release();
	bbuf->Release();
	device->Release();
	dcontext->Release();
}

void RenderFrame(double t) {
	double period = 10.0f;
	dcontext->ClearRenderTargetView(bbuf, D3DXCOLOR(NormalizeT(t / period), NormalizeT(t / period), NormalizeT(t / period), 1.0f));
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	dcontext->IASetVertexBuffers(0, 1, &vbufptr, &stride, &offset);
	dcontext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dcontext->Draw(3, 0);
	swapchain->Present(0, 0);
}

double NormalizeT(double t) { return t - (double)((int)t); }