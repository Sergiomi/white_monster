#include "stdafx.h"
//#include <wchar.h>
#include <CommCtrl.h>
#include "SnowModel.h"
#include "CSnow.h"

extern HINSTANCE hInst;
extern HWND hTemperatureDlg;
extern HWND hEditLayerDlg;
extern BOOL bEditLayer;

int currentT = -1;
BOOL bEdit = FALSE;
HWND hEditDlg;

INT_PTR CALLBACK	EditDlgProc(HWND, UINT, WPARAM, LPARAM);

//BOOL TEMPDLG_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void TEMPDLG_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);
void TEMPDLG_OnNotify(HWND hWnd, int id, LPNMHDR lpnmhdr);
void TEMPDLG_OnDestroy(HWND hWnd);

void EDITLAYERDLG_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);

INT_PTR CALLBACK TempDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	HWND hChild;
	RECT rect;
	LV_COLUMN lvc;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		hTemperatureDlg = hDlg;
		hEditDlg = NULL;
		hMenu = GetMenu(GetParent(hDlg));
		CheckMenuItem(hMenu, IDM_TEMPERATURE_DLG, MF_CHECKED);

		GetClientRect(hDlg, &rect);
		hChild = CreateWindow(WC_LISTVIEW, _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 0, 0, rect.right, rect.bottom - 74, hDlg, (HMENU)IDC_TEMPERATURE_LIST, hInst, NULL);
		ListView_SetExtendedListViewStyle(hChild,  LVS_EX_AUTOSIZECOLUMNS | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		ZeroMemory(&lvc,sizeof(lvc));
		lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;//Маска
		lvc.fmt=LVCFMT_LEFT;//Выравнивание влево
		lvc.cx=rect.right/2;//Размер первого поля заголовка
		lvc.pszText=_T("time");//Строка с текстом первого поля заголовка
		ListView_InsertColumn(hChild,0,&lvc);//Включаем поле в заголовок
		lvc.pszText=_T("T");//Строка с текстом первого поля заголовка
		ListView_InsertColumn(hChild,1,&lvc);//Включаем поле в заголовок

		UpdateTemperatureLog();
		break;
	//case WM_INITDIALOG: HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, NULL);//TEMPDLG_OnInitDialog); break;
	HANDLE_MSG(hDlg, WM_COMMAND, TEMPDLG_OnCommand);
	HANDLE_MSG(hDlg, WM_DESTROY, TEMPDLG_OnDestroy);
	case WM_NOTIFY: HANDLE_WM_NOTIFY(hDlg, wParam, lParam, TEMPDLG_OnNotify); break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

void TEMPDLG_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case IDOK:
	case IDCANCEL:
		DestroyWindow(hWnd);
		break;
	case IDC_EDIT_BTN:
		bEdit = TRUE;
		if (!hEditDlg)
			hEditDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_EDIT_TEMPERATURE), hWnd, EditDlgProc);
		SetDlgItemText(hEditDlg, IDOK, _T("Edit"));
		if (currentT >= 0 && currentT < CSnow::current()->getTLogSize()) {
			SetDlgItemText(hEditDlg, IDC_EDIT1, ToString(CSnow::current()->getTLog(currentT).first).c_str());
			SetDlgItemText(hEditDlg, IDC_EDIT2, ToString(CSnow::current()->getTLog(currentT).second).c_str());
			EnableWindow(GetDlgItem(hEditDlg, IDOK), TRUE);
		} else {
			EnableWindow(GetDlgItem(hEditDlg, IDOK), FALSE);
		}
		break;
	case IDC_ADD_BTN:
		bEdit = FALSE;
		if (!hEditDlg)
			hEditDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_EDIT_TEMPERATURE), hWnd, EditDlgProc);
		
		SetDlgItemText(hEditDlg, IDOK, _T("Add"));
		SetDlgItemText(hEditDlg, IDC_EDIT1, _T(""));
		SetDlgItemText(hEditDlg, IDC_EDIT2, _T(""));
		EnableWindow(GetDlgItem(hEditDlg, IDOK), TRUE);
		break;
	case IDC_DELETE_BTN:
		CSnow::current()->removeTLog(currentT);
		//currentT--;
		UpdateAll();
		//if (currentT >= 0)
		//	ListView_SetItemState(GetDlgItem(hWnd, IDC_LAYERS_LIST), currentT, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		break;
	}
}
void TEMPDLG_OnDestroy(HWND hWnd)
{
	HMENU hMenu;
	hMenu = GetMenu(GetParent(hWnd));
	CheckMenuItem(hMenu, IDM_TEMPERATURE_DLG, 0);
	hTemperatureDlg = NULL;
}

void TEMPDLG_OnNotify(HWND hWnd, int id, LPNMHDR lpnmhdr)
{
	if(lpnmhdr->idFrom == IDC_TEMPERATURE_LIST) {
		if(lpnmhdr->code == LVN_ITEMCHANGED) {
			if(((NM_LISTVIEW *)lpnmhdr)->uNewState & LVIS_FOCUSED) {
				currentT = ((NM_LISTVIEW *)lpnmhdr)->iItem;
				if (hEditDlg && bEdit && (currentT >= 0 && currentT < CSnow::current()->getTLogSize()))
				{
					SetDlgItemText(hEditDlg, IDC_EDIT1, ToString(CSnow::current()->getTLog(currentT).first).c_str());
					SetDlgItemText(hEditDlg, IDC_EDIT2, ToString(CSnow::current()->getTLog(currentT).second).c_str());
					EnableWindow(GetDlgItem(hEditDlg, IDOK), TRUE);
				}
			}
		}
	}
}

INT_PTR CALLBACK EditDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	RECT listRect;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		GetWindowRect(hDlg, &rect);
		GetWindowRect(GetDlgItem(GetParent(hDlg), IDC_TEMPERATURE_LIST), &listRect);
		MoveWindow(GetDlgItem(GetParent(hDlg), IDC_TEMPERATURE_LIST), 0, rect.bottom - rect.top, rect.right - rect.left, listRect.bottom - listRect.top - rect.bottom + rect.top, TRUE);

		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			TCHAR str[80];
			GetDlgItemText(hDlg, IDC_EDIT1, str, sizeof(str));
			double time = _wtof(str);
			GetDlgItemText(hDlg, IDC_EDIT2, str, sizeof(str));
			double T = _wtof(str);
			if (bEdit) {
				CSnow::current()->editTLog(currentT, time, T);
			} else {
				CSnow::current()->addTLog(time, T);
			}
			UpdateAll();
		} else if (LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
		}
		break;
	case WM_DESTROY:
		GetClientRect(GetParent(hDlg), &rect);
		MoveWindow(GetDlgItem(GetParent(hDlg), IDC_TEMPERATURE_LIST),  0, 0, rect.right, rect.bottom - 74, TRUE);

		hEditDlg = NULL;
		break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

INT_PTR CALLBACK EditLayerDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//RECT rect;
	//RECT listRect;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("Свежий снег"));
		SendDlgItemMessage(hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("Метелевый снег"));
		SendDlgItemMessage(hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("Снежная крупа"));
		SendDlgItemMessage(hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("Ледяной дождь"));
		SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, 0, 0);
		SetDlgItemText(hDlg, IDC_EDIT1, _T("0"));
		SetDlgItemText(hDlg, IDC_EDIT2, _T("0"));
		SetDlgItemText(hDlg, IDC_EDIT4, _T("0.1"));
		SetDlgItemText(hDlg, IDC_EDIT5, _T("0.1"));
		break;
	HANDLE_MSG(hDlg, WM_COMMAND, EDITLAYERDLG_OnCommand);
	case WM_DESTROY:
		hEditLayerDlg = NULL;
		break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

void EDITLAYERDLG_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case IDOK:
		{
		TCHAR str[80];
		GetDlgItemText(hWnd, IDC_EDIT1, str, sizeof(str));
		double time = _wtof(str);
		GetDlgItemText(hWnd, IDC_EDIT2, str, sizeof(str));
		double duration = _wtof(str);
		GetDlgItemText(hWnd, IDC_EDIT3, str, sizeof(str));
		double life_time = _wtof(str);
		GetDlgItemText(hWnd, IDC_EDIT4, str, sizeof(str));
		double density = _wtof(str);
		GetDlgItemText(hWnd, IDC_EDIT5, str, sizeof(str));
		double size = _wtof(str);
		GetDlgItemText(hWnd, IDC_EDIT6, str, sizeof(str));
		double height = _wtof(str);
		if (bEditLayer) {
			CLayer* layer = CSnow::current()->getLayer();
			if (layer)
			{
				layer->SetTime0(time);
				layer->SetDuration(duration);
				layer->SetLifetime(life_time);
				layer->SetDensity0(density);
				layer->SetParticleSize0(size);
				layer->SetHeight0(height);
				layer->Reset();
			}
			UpdateAll();
			//CSnow::current()->editTLog(currentT, time, T);
		} else {
			CSnow::current()->add(density, height, time, size, life_time, duration);
			UpdateAll();
		}
		break;
		}
	case IDCANCEL:
		DestroyWindow(hWnd);
		break;
	case IDC_COMBO1:
		if (codeNotify == CBN_SELCHANGE)
		{
			switch (SendMessage(hwndCtl, CB_GETCURSEL, 0, 0))
			{
			case 0: 
				SetDlgItemText(hWnd, IDC_EDIT4, _T("0.1"));
				SetDlgItemText(hWnd, IDC_EDIT5, _T("0.1"));
				break;
			case 1: 
				SetDlgItemText(hWnd, IDC_EDIT4, _T("0.3"));
				SetDlgItemText(hWnd, IDC_EDIT5, _T("0.05"));
				break;
			case 2: 
				SetDlgItemText(hWnd, IDC_EDIT4, _T("0.2"));
				SetDlgItemText(hWnd, IDC_EDIT5, _T("1.0"));
				break;
			case 3: 
				SetDlgItemText(hWnd, IDC_EDIT4, _T("0.8"));
				SetDlgItemText(hWnd, IDC_EDIT5, _T("0.5"));
				break;
			}
		}
		break;
	default:
		FORWARD_WM_COMMAND(hWnd, id, hwndCtl, codeNotify, DefWindowProc);
	}
}