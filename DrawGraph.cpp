#include "stdafx.h"
#include "CSnow.h"
#include "SnowModel.h"
#include "DrawGraph.h"

void DrawGrid(HDC hdc, int x, int y, int width, int height, int stepX, int stepY)
{
	HPEN pen;
	pen = CreatePen(PS_DOT, 1, RGB(0,0,0));
	pen = (HPEN)SelectObject(hdc, pen);

	for(int i = 0; i <= width; i += stepX) {
		MoveToEx(hdc, x + i, y, NULL);
		LineTo(hdc, x + i, y + height);
	}

	for(int i = height - 1; i >= 0 ; i -= stepY) {
		MoveToEx(hdc, x, y + i, NULL);
		LineTo(hdc, x + width, y + i);
	}

	pen = (HPEN)SelectObject(hdc, pen);
	DeleteObject(pen);
}
void DrawTemperature(HDC hdc, int x, int y, int width, int height, double timeS, double timeE)
{
	HPEN pen;
	double maxPositiveHeight, maxNegativeHeight;
	std::pair<double, double> min_max = CSnow::current()->getMinMaxTemperature(timeS, timeE);
	
	maxPositiveHeight = (min_max.second < 0) ? 0 : min_max.second;
	maxNegativeHeight = (min_max.first > 0) ? 0 : abs(min_max.first);
	
	double w_delta = (timeE - timeS)/width;
	double h_delta = height/(maxNegativeHeight + maxPositiveHeight);
	double y_delta = h_delta*maxPositiveHeight;

	pen = CreatePen(0, 1, 0x000000);
	pen = (HPEN)SelectObject(hdc, pen);
	
	MoveToEx(hdc, x, y + y_delta, NULL);
	LineTo(hdc, x + width, y_delta);

	MoveToEx(hdc, x, y + y_delta - (int)(h_delta*CSnow::current()->getTemperature(timeS)), NULL);
	for (int i = 0; i < width; i++)
		LineTo(hdc, x + i, y + y_delta - h_delta*CSnow::current()->getTemperature(i*w_delta + timeS));
		//LineTo(hdc, x + i, y + height - (int)(h_delta*CSnow::current()->getTGradient(i*w_delta + timeS)));
	
	//MoveToEx(hdc, x, y + y_delta - (int)(h_delta*CSnow::current()->getBaseTemperature(timeS)), NULL);
	//for (int i = 0; i < width; i++)
	//	LineTo(hdc, x + i, y + y_delta - h_delta*CSnow::current()->getBaseTemperature(i*w_delta + timeS));

	LOGFONT lFont;
	ZeroMemory(&lFont, sizeof(lFont));
	lFont.lfHeight = 16;
	lFont.lfWeight = FW_BOLD;
	wcscpy_s(lFont.lfFaceName, WINDOW_FONT);
	HFONT font = CreateFontIndirect(&lFont);
	font = SelectFont(hdc, font);
	
	std::wstringstream sstream;
	sstream.precision(4);
	sstream << _T("T = ") << -maxNegativeHeight;
	TextOut(hdc, x + MARGIN, y + height - 16, sstream.str().c_str(), sstream.str().size());

	font = SelectFont(hdc, &font);
	DeleteFont(font);
	
	/*sstream << _T("t = ") << maxPositiveHeight;
	TextOut(hdc, x + MARGIN, y + 12, sstream.str().c_str(), sstream.str().size());*/
	
	pen = (HPEN)SelectObject(hdc, pen);
	DeleteObject(pen);
}
void DrawCursor(HDC hdc, int x, int y, int height)
{
	HPEN pen = CreatePen(0, 2, 0x000000);
	pen = (HPEN)SelectObject(hdc, pen);
	MoveToEx(hdc, x, y, NULL);
	LineTo(hdc, x, y + height);
	pen = (HPEN)SelectObject(hdc, pen);
	DeleteObject(pen);

}
void DrawCurrentInfo(HDC hdc, int x, int y, int width, int height)
{
	LOGFONT lFont;
	ZeroMemory(&lFont, sizeof(lFont));
	lFont.lfHeight = 16;
	lFont.lfWeight = FW_BOLD;
	wcscpy_s(lFont.lfFaceName, WINDOW_FONT);
	HFONT font = CreateFontIndirect(&lFont);
	font = SelectFont(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	
	std::wstring str;
	if (x > width/2) 
		SetTextAlign(hdc, TA_RIGHT);
	else
		SetTextAlign(hdc, TA_LEFT);
	str = _T("  time = ") + CSnow::current()->ToString(CSnow::TIME, 4) + _T("h  ");
	TextOut(hdc, x, (height + y)/2 - lFont.lfHeight, str.c_str(), str.size());
	str = _T("  Height = ") + CSnow::current()->ToString(CSnow::HEIGHT, 4) + _T("mm  ");
	TextOut(hdc, x, (height + y)/2, str.c_str(), str.size());
	font = SelectFont(hdc, font);
	DeleteObject(font);
}
void DrawGraph(HDC hdc, int x, int y, int width, int height, int type, double timeS, double timeE)
{
	HPEN pen;
	double t, delta, maxHeight;
	int h;
		
	delta = (timeE - timeS)/width;
	t = timeS;		
	//maxHeight = (type)? (CSnow::current()->getSnowHeight(timeS, timeE)) : (CSnow::current()->getSnowDensity(timeE));
	maxHeight = CSnow::current()->getSnowHeight(timeS, timeE);
	
	SetBkMode(hdc, TRANSPARENT);
	//ShowWindow(GetDlgItem(GetParent(hWnd), IDC_PROGRESS), 1);
	//SendDlgItemMessage(GetParent(hWnd), IDC_PROGRESS, PBM_SETRANGE, NULL, MAKELPARAM(0,width));

	for(int i = 0; i < width; i++, t += delta)
	{
		int dy = 0;
		//SendDlgItemMessage(GetParent(hWnd), IDC_PROGRESS, PBM_SETPOS, i, NULL);
		for (int l = 0; l < CSnow::current()->getLayersNum(); l++) {
			//if (type)h = height*CSnow::current()->getLayer(l)->GetHeight(t)/maxHeight;
			//else h = height*CSnow::current()->getLayer(l)->GetDensity(t)/maxHeight;
			h = height*CSnow::current()->getLayer(l)->GetHeight(t)/maxHeight;

			if (h == 0)
				continue;

			pen = CreatePen(0, 1, (type) ? (CSnow::current()->getLayer(l)->GetParticleColor(t)) : (CSnow::current()->getLayer(l)->GetDensityColor(t)));
			pen = (HPEN)SelectObject(hdc, pen);
							
			MoveToEx(hdc, x + i, y + height - dy - (int)h, NULL);
			LineTo(hdc, x + i, y + height - dy);
			dy += h;
			
			pen = (HPEN)SelectObject(hdc, pen);
			DeleteObject(pen);

			//SetPixel(hdc, x + i, y + height - dy, 0);
		}
	}
	//ShowWindow(GetDlgItem(GetParent(hWnd), IDC_PROGRESS), 0);
	DrawGrid(hdc, x, y, width, height);
	
	LOGFONT lFont;
	ZeroMemory(&lFont, sizeof(lFont));
	lFont.lfHeight = 16;
	lFont.lfWeight = FW_BOLD;
	wcscpy_s(lFont.lfFaceName, WINDOW_FONT);
	HFONT font = CreateFontIndirect(&lFont);
	font = SelectFont(hdc, font);
	
	std::wstringstream sstream;
	sstream.precision(4);
	sstream << _T("H = ") << maxHeight << _T("mm");
	TextOut(hdc, x + MARGIN, y, sstream.str().c_str(), sstream.str().size());

	font = SelectFont(hdc, &font);
	DeleteFont(font);
}