#include "stdafx.h"
#include <CommCtrl.h>
#include "SnowModel.h"
#include "CSnow.h"
#include "DrawGraph.h"

extern HINSTANCE hInst;								// текущий экземпляр
extern BOOL bInvalidateGraph;
extern BOOL bInvalidateTemperature;

HDC hdcMem;
HBITMAP hMemBmp;

#define LAYERS_COUNT	2 //0 - SNOW, 1 - GRAPH, 2 - ALL
HDC hdcLayers[LAYERS_COUNT];
HBITMAP hLayersBmp[LAYERS_COUNT];

BOOL GRAPHWND_OnCreate(HWND hWnd, LPCREATESTRUCT);
void GRAPHWND_OnSize(HWND hWnd, UINT state, int cx, int cy);
void GRAPHWND_OnPaint(HWND hWnd);
void GRAPHWND_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void GRAPHWND_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);
void GRAPHWND_OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void GRAPHWND_OnDestroy(HWND hWnd);

LRESULT CALLBACK WndGraphProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	HANDLE_MSG(hWnd, WM_CREATE, GRAPHWND_OnCreate);
	HANDLE_MSG(hWnd, WM_SIZE, GRAPHWND_OnSize);
	HANDLE_MSG(hWnd, WM_PAINT, GRAPHWND_OnPaint);
	HANDLE_MSG(hWnd, WM_LBUTTONDOWN, GRAPHWND_OnLButtonDown);
	HANDLE_MSG(hWnd, WM_COMMAND, GRAPHWND_OnCommand);
	HANDLE_MSG(hWnd, WM_KEYDOWN, GRAPHWND_OnKeyDown);
	HANDLE_MSG(hWnd, WM_DESTROY, GRAPHWND_OnDestroy);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

	//int wmId, wmEvent;
	//HDC hdc;
	
	//static HWND hEditS, hEditE;
	//static HWND hRadio1, hRadio2;
	//static double delta;
	//static int timeS, timeE;

	//RECT rect;
		
BOOL GRAPHWND_OnCreate(HWND hWnd, LPCREATESTRUCT)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	CreateWindow(_T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_NUMBER, MARGIN, rect.bottom - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_EDIT_S, hInst, NULL);
	CreateWindow(_T("edit"), _T("500"), WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_RIGHT, rect.right - BUTTON_WIDTH - MARGIN, rect.bottom - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_EDIT_E, hInst, NULL);
	CreateWindow(_T("button"), _T("Particles"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON  | BS_PUSHLIKE, rect.right - BUTTON_WIDTH - MARGIN, MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_RADIO_1, hInst, NULL);
	CreateWindow(_T("button"), _T("Density"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE, rect.right - BUTTON_WIDTH - MARGIN, MARGIN + 2 + BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_RADIO_2, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_RADIO_1, BM_SETCHECK, TRUE, NULL);
	SendDlgItemMessage(hWnd, IDC_EDIT_S, EM_LIMITTEXT, (WPARAM)10, NULL);
	SendDlgItemMessage(hWnd, IDC_EDIT_E, EM_LIMITTEXT, (WPARAM)10, NULL);
				
	HDC hdc = GetDC(hWnd);
	for (int i = 0; i < LAYERS_COUNT; i++) {
		hdcLayers[i] = CreateCompatibleDC(hdc);
		hLayersBmp[i] = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		hLayersBmp[i] = (HBITMAP)SelectObject(hdcLayers[i], hLayersBmp[i]);
	}
	ReleaseDC(hWnd, hdc);

	bInvalidateGraph = TRUE;
	SetFocus(hWnd);
	return TRUE;
}
void GRAPHWND_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_S), MARGIN, cy - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_E), cx - BUTTON_WIDTH - MARGIN, cy - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
	MoveWindow(GetDlgItem(hWnd, IDC_RADIO_1), cx - BUTTON_WIDTH - MARGIN, MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
	MoveWindow(GetDlgItem(hWnd, IDC_RADIO_2), cx - BUTTON_WIDTH - MARGIN, MARGIN + 2 + BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, TRUE);
		
	HDC hdc = GetDC(hWnd);
	bInvalidateGraph = TRUE;
	for (int i = 0; i < LAYERS_COUNT; i++) {
		hLayersBmp[i] = CreateCompatibleBitmap(hdc, cx, cy);
		hLayersBmp[i] = (HBITMAP)SelectObject(hdcLayers[i], hLayersBmp[i]);
		DeleteObject(hLayersBmp[i]); // Remove old bitmap. Don't need it anymore
	}
	ReleaseDC(hWnd, hdc);
}
void GRAPHWND_OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	int dx;
	static double timeS, timeE;
	static double delta;
	RECT rect;
	GetClientRect(hWnd, &rect);

	if (bInvalidateGraph) {
		int state;
		
		state = SendDlgItemMessage(hWnd, IDC_RADIO_1, BM_GETCHECK, NULL, NULL);
		timeS = GetDlgItemInt(hWnd, IDC_EDIT_S, NULL, TRUE);
		timeE = GetDlgItemInt(hWnd, IDC_EDIT_E, NULL, TRUE);
		
		delta = (timeE - timeS)/rect.right;

		for (int i = 0; i < LAYERS_COUNT; i++)
			PatBlt(hdcLayers[i], 0, 0, rect.right, rect.bottom, WHITENESS);

		DrawGraph(hdcLayers[0], 0, 0, rect.right, rect.bottom - BUTTON_HEIGHT - 1, state, timeS, timeE);
		DrawGrid(hdcLayers[0], 0, 0, rect.right, rect.bottom - BUTTON_HEIGHT - 1, timeS, timeE);
		//DrawTimeLabels(hdcLayers[0], 0, 0, rect.right, timeS, timeE);
		
		if (GetMenuState(GetMenu(GetParent(hWnd)), IDM_TEMPERATURE, MF_CHECKED))
			DrawTemperature(hdcLayers[1], 0, 0, rect.right, rect.bottom - BUTTON_HEIGHT - 1, timeS, timeE);
		
		bInvalidateGraph = FALSE;
	} else if (bInvalidateTemperature)
	{
		PatBlt(hdcLayers[1], 0, 0, rect.right, rect.bottom, WHITENESS);
		if (GetMenuState(GetMenu(GetParent(hWnd)), IDM_TEMPERATURE, MF_CHECKED))
			DrawTemperature(hdcLayers[1], 0, 0, rect.right, rect.bottom - BUTTON_HEIGHT - 1, timeS, timeE);

		bInvalidateTemperature = FALSE;
	}
		
	HDC hdc = BeginPaint(hWnd, &ps);
	dx = delta > 0 ? (int)((CSnow::current()->getTime() - timeS)/delta) : 0;
	BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, hdcLayers[0], ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, hdcLayers[1], ps.rcPaint.left, ps.rcPaint.top, SRCAND);
	DrawCursor(hdc, rect.left + dx, rect.top, rect.bottom);
	DrawCurrentInfo(hdc, rect.left + dx, rect.top, rect.right, rect.bottom - BUTTON_HEIGHT - 1);
	EndPaint(hWnd, &ps);
}
void GRAPHWND_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	int timeS, timeE;
	double delta;
	RECT rect;

	GetClientRect(hWnd, &rect);
	timeS = GetDlgItemInt(hWnd, IDC_EDIT_S, NULL, TRUE);
	timeE = GetDlgItemInt(hWnd, IDC_EDIT_E, NULL, TRUE);
	delta = (double)(timeE - timeS)/rect.right;
	
	CSnow::current()->setTime((delta >= 0.5)?((int)(x*delta + timeS)):(x*delta + timeS));
	SetFocus(hWnd);
		
	UpdateCurrentTime();
}
void GRAPHWND_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_RADIO_1:
	case IDC_RADIO_2:
		if (codeNotify == BN_CLICKED) {
			bInvalidateGraph = TRUE;
			UpdateGraph();
			UpdateLayersView();
		}
		break;
	case IDC_EDIT_E:
	case IDC_EDIT_S:
		if (codeNotify == EN_CHANGE) {
			bInvalidateGraph = TRUE;
			//	UpdateGraph();
		}
		break;
	default:
		FORWARD_WM_COMMAND(hWnd, id, hwndCtl, codeNotify, DefWindowProc);
	}
}
void GRAPHWND_OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	int timeS, timeE;
	double time;
		
	switch(vk)
	{
	case VK_LEFT:
	case VK_RIGHT:
		time = CSnow::current()->getTime() + ((vk == VK_LEFT)?(-1):(1));
		timeS = GetDlgItemInt(hWnd, IDC_EDIT_S, NULL, TRUE);
		timeE = GetDlgItemInt(hWnd, IDC_EDIT_E, NULL, TRUE);
		if (time < 0) time = 0;
		else if (time < timeS) 
		{
			double d = timeS - time;
			SetDlgItemInt(hWnd, IDC_EDIT_S, timeS - d, TRUE);
			SetDlgItemInt(hWnd, IDC_EDIT_E, timeE - d, TRUE);
			bInvalidateGraph = TRUE;
		}
		else if (time > timeE) 
		{
			double d = time - timeE;
			SetDlgItemInt(hWnd, IDC_EDIT_E, timeE + d, TRUE);
			SetDlgItemInt(hWnd, IDC_EDIT_S, timeS + d, TRUE);
			bInvalidateGraph = TRUE;
		}

		CSnow::current()->setTime(time);
		UpdateCurrentTime();
		break;
	}
}
void GRAPHWND_OnDestroy(HWND hWnd)
{
	for (int i = 0; i < LAYERS_COUNT; i++) {
		DeleteObject(hLayersBmp[i]);
		DeleteDC(hdcLayers[i]);
	}

	PostQuitMessage(0);
}