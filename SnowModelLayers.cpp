#include "stdafx.h"
#include <CommCtrl.h>
#include "SnowModel.h"
#include "CSnow.h"

extern HINSTANCE hInst;								// текущий экземпляр

void DrawLayers(HDC hdc, int x, int y, int width, int height, int type)
{
	double maxHeight = CSnow::current()->getSnowHeight();
	double temperatureRange = 40;//max(abs(bottomTemperature - topTemperature), abs(topTemperature));
	if(temperatureRange == 0)temperatureRange = 1;

	if (maxHeight == 0)
		return;

	HBRUSH brush;
	int h;
	int topT, bottomT;
	int rightMargin = 40;
	RECT rect;
	CLayer *layer;

	
	HPEN pen = CreatePen(0, 1, 0);
	//HPEN boldPen = CreatePen(0, 3, 0);
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	HPEN dotPen = CreatePen(PS_DASH, 1, RGB(0,0,0));

	LOGFONT lFont;
	ZeroMemory(&lFont, sizeof(lFont));
	lFont.lfHeight = 16;
	lFont.lfWeight = FW_BOLD;
	wcscpy_s(lFont.lfFaceName, WINDOW_FONT);
	HFONT font = CreateFontIndirect(&lFont);
	HFONT oldFont = SelectFont(hdc, font);
		
	SetBkMode(hdc, TRANSPARENT);

	int dy = 0;
	for (int l = 0; l < CSnow::current()->getLayersNum(); l++) {
		layer = CSnow::current()->getLayer(l);
		//if (type)h = height*CSnow::current()->getLayer(l)->GetHeight(t)/maxHeight;
		//else h = height*CSnow::current()->getLayer(l)->GetDensity(t)/maxHeight;
		h = (int)(height*layer->GetHeight()/maxHeight);

		if (h == 0)
			continue;

		//draw layer
		brush = CreateSolidBrush((type) ? layer->GetParticleColor() : layer->GetDensityColor());
		rect.left = x; rect.right = x + width - rightMargin;
		rect.top = y + height - dy - (int)h; rect.bottom = y + height - dy;
		FillRect(hdc, &rect, brush);
		DeleteObject(brush);

		//draw layer temperature
		topT = (int)((width - rightMargin)*layer->GetTopTemperature()/temperatureRange);
		bottomT = (int)((width - rightMargin)*layer->GetBottomTemperature()/temperatureRange);
		SelectObject(hdc, dotPen);
		SetBkMode(hdc, OPAQUE);
		MoveToEx(hdc, x + width - rightMargin + topT - 1, rect.top, NULL);
		LineTo(hdc, x + width - rightMargin + bottomT - 1, rect.bottom);
		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, pen);

		if(layer->GetSelection()) {
			HPEN boldPen = CreatePen(0, 3, ((type) ? layer->GetParticleSelectionColor() : 0));
			SelectObject(hdc, boldPen);
			MoveToEx(hdc, rect.left + 1, rect.top + 1, NULL);
			LineTo(hdc, rect.left + 1, rect.bottom - 1);
			LineTo(hdc, rect.right - 2, rect.bottom - 1);
			LineTo(hdc, rect.right - 2, rect.top + 1);
			LineTo(hdc, rect.left + 1, rect.top + 1);
			SelectObject(hdc, pen);
			DeleteObject(boldPen);
		}
		
		MoveToEx(hdc, x + width - rightMargin + 10, rect.top, NULL);
		LineTo(hdc, x + width - rightMargin + 10, rect.bottom);
		
		TEXTMETRIC tMetric;
		GetTextMetrics(hdc, &tMetric);
		if (h > tMetric.tmHeight/2) {
			SetTextAlign(hdc, TA_LEFT);
			SetTextColor(hdc, 0x0);
			TextOut(hdc, x + width - rightMargin + 14, (rect.bottom + rect.top)/2 - tMetric.tmHeight/2, layer->ToString(CLayer::HEIGHT, 4).c_str(), layer->ToString(CLayer::HEIGHT, 4).size());
			SetTextAlign(hdc, TA_CENTER);
			SetTextColor(hdc, 0xffffff);
			TextOut(hdc, (x + width - rightMargin)/2, (rect.bottom + rect.top)/2 - tMetric.tmHeight/2, layer->ToString((type) ? CLayer::PARTICLE : CLayer::DENSITY, 4).c_str(), layer->ToString((type) ? CLayer::PARTICLE : CLayer::DENSITY, 4).size());
		}
		
		dy += h;
		MoveToEx(hdc, x, y + height - dy, NULL);
		LineTo(hdc, x + width - rightMargin + 16, y + height - dy);

		//draw grid
		MoveToEx(hdc, x, y + height - dy + 3, NULL);
		LineTo(hdc, x, y + height - dy - 3);
		MoveToEx(hdc, x + (width - rightMargin)/4, y + height - dy + 3, NULL);
		LineTo(hdc, x + (width - rightMargin)/4, y + height - dy - 3);
		MoveToEx(hdc, x + (width - rightMargin)/2, y + height - dy + 3, NULL);
		LineTo(hdc, x + (width - rightMargin)/2, y + height - dy - 3);
		MoveToEx(hdc, x + (width - rightMargin)/4*3, y + height - dy +3, NULL);
		LineTo(hdc, x + (width - rightMargin)/4*3, y + height - dy - 3);
		MoveToEx(hdc, x + (width - rightMargin) - 1, y + height - dy + 3, NULL);
		LineTo(hdc, x + (width - rightMargin) - 1, y + height - dy - 3);
	}
	
	SelectObject(hdc, oldPen);
	SelectFont(hdc, &oldFont);
	DeleteObject(pen);
	DeleteObject(dotPen);
	DeleteFont(font);
}

LRESULT CALLBACK WndLayersProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);
		DrawLayers(hdc, MARGIN, MARGIN, rect.right - BUTTON_WIDTH, rect.bottom - MARGIN, SendDlgItemMessage(GetDlgItem(GetParent(hWnd), IDC_GRAPH_WINDOW), IDC_RADIO_1, BM_GETCHECK, NULL, NULL));
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		{
			short y;
			y = HIWORD(lParam);

			double snowHeight = CSnow::current()->getSnowHeight();
			GetClientRect(hWnd, &rect);
			double winHeight = rect.bottom;
			double dy = (winHeight - y)*snowHeight/(winHeight - MARGIN);

			double h = 0;
			for (int l = 0; l < CSnow::current()->getLayersNum(); l++) {
				if (dy < (h = h + CSnow::current()->getLayer(l)->GetHeight())) {
					CSnow::current()->setSelection(l);
					//ListView_SetItemState(GetDlgItem(GetParent(hWnd), IDC_LAYERS_LIST), l, );
					//ListView_SetSelectionMark(GetDlgItem(GetParent(hWnd), IDC_LAYERS_LIST), l);
					HWND hListview = GetDlgItem(GetParent(hWnd), IDC_LAYERS_LIST);
					ListView_SetItemState(hListview, ListView_GetItemCount(hListview) - l - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					break;
				}
			}
			
			UpdateCurrentLayer();
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
