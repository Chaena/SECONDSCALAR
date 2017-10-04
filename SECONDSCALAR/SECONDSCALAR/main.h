#pragma once
#include <windows.h>
#include <windowsx.h>

struct VERTEX;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShowCmd);
void InitD3D(HWND hWnd);
void InitGraphics(void);
void CleanD3D(void);
void InitPipeline(void);
void RenderFrame(double t);

VERTEX ConstructVertex(float x, float y, float z, D3DXCOLOR color);

double NormalizeT(double t);