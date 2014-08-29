// SnowModel.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include <string>
#include <CommCtrl.h>
#include <commdlg.h>
//#include <Fl/Fl_File_Chooser.h>
#include "SnowModel.h"
#include "CSnow.h"
#include "DrawGraph.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;								// текущий экземпляр
HWND hMainWnd;									//Главное окно
HWND hTemperatureDlg;
HWND hEditLayerDlg;
BOOL bEditLayer;
BOOL bInvalidateGraph;
BOOL bInvalidateTemperature;
TCHAR szTitle[MAX_LOADSTRING];					// Текст строки заголовка
TCHAR szWindowClass[MAX_LOADSTRING];			// имя класса главного окна
TCHAR szGraphClass[MAX_LOADSTRING];
//TCHAR szInfoClass[MAX_LOADSTRING];
TCHAR szLayersClass[MAX_LOADSTRING];

// Отправить объявления функций, включенных в этот модуль кода:
ATOM				MyRegisterClass(HINSTANCE hInstance, WNDPROC proc, LPCTSTR szName, UINT brBackground);
ATOM				MyRegisterChild(HINSTANCE hInstance, WNDPROC proc, LPCTSTR szName, UINT brBackground);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	WndGraphProc(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK	WndInfoProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	WndLayersProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	TempDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	EditLayerDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: разместите код здесь.
	MSG msg;
	HACCEL hAccelTable;

	// Инициализация глобальных строк
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SNOWMODEL, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GRAPH_WINDOW, szGraphClass, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_INFO_WINDOW, szInfoClass, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LAYERS_WINDOW, szLayersClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance, WndProc, szWindowClass, COLOR_BTNFACE);
	MyRegisterChild(hInstance, WndGraphProc, szGraphClass, COLOR_WINDOW);
	//MyRegisterClass(hInstance, WndInfoProc, szInfoClass, COLOR_WINDOW);
	MyRegisterChild(hInstance, WndLayersProc, szLayersClass, COLOR_WINDOW);

	// Включить GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Выполнить инициализацию приложения:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNOWMODEL));

	// Цикл основного сообщения:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			HWND hGraphWnd = GetDlgItem(hMainWnd, IDC_GRAPH_WINDOW);
			if ((msg.hwnd == GetDlgItem(hGraphWnd, IDC_EDIT_E) || msg.hwnd == GetDlgItem(hGraphWnd, IDC_EDIT_S)) && msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
				SetFocus(hGraphWnd);
				UpdateGraph();
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
//  КОММЕНТАРИИ:
//
//    Эта функция и ее использование необходимы только в случае, если нужно, чтобы данный код
//    был совместим с системами Win32, не имеющими функции RegisterClassEx'
//    которая была добавлена в Windows 95. Вызов этой функции важен для того,
//    чтобы приложение получило "качественные" мелкие значки и установило связь
//    с ними.
//
ATOM MyRegisterClass(HINSTANCE hInstance, WNDPROC proc, LPCTSTR szName, UINT brBackground)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= proc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNOWMODEL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(brBackground+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SNOWMODEL);
	wcex.lpszClassName	= szName;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

ATOM MyRegisterChild(HINSTANCE hInstance, WNDPROC proc, LPCTSTR szName, UINT brBackground)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= proc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(brBackground+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szName;
	wcex.hIconSm		= 0;

	return RegisterClassEx(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, CW_USEDEFAULT, MAINWND_WIDTH, MAINWND_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hMainWnd)
   {
      return FALSE;
   }

   ShowWindow(hMainWnd, nCmdShow);
   UpdateWindow(hMainWnd);
   UpdateAll();

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND	- обработка меню приложения
//  WM_PAINT	-Закрасить главное окно
//  WM_DESTROY	 - ввести сообщение о выходе и вернуться.
//
//

BOOL SNOWMODEL_OnCreate(HWND hWnd, LPCREATESTRUCT);
void SNOWMODEL_OnSize(HWND hWnd, UINT state, int cx, int cy);
void SNOWMODEL_OnPaint(HWND hWnd);
void SNOWMODEL_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);
void SNOWMODEL_OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void SNOWMODEL_OnNotify(HWND hWnd, int id, LPNMHDR lpnmhdr);
void SNOWMODEL_OnDestroy(HWND hWnd);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	HANDLE_MSG(hWnd, WM_CREATE, SNOWMODEL_OnCreate);
	HANDLE_MSG(hWnd, WM_SIZE, SNOWMODEL_OnSize);
	HANDLE_MSG(hWnd, WM_PAINT, SNOWMODEL_OnPaint);
	HANDLE_MSG(hWnd, WM_COMMAND, SNOWMODEL_OnCommand);
	HANDLE_MSG(hWnd, WM_KEYDOWN, SNOWMODEL_OnKeyDown);
	case WM_NOTIFY: HANDLE_WM_NOTIFY(hWnd, wParam, lParam, SNOWMODEL_OnNotify); break;
	HANDLE_MSG(hWnd, WM_DESTROY, SNOWMODEL_OnDestroy);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
///////////////////////
	//int wmId, wmEvent;
	//
	//HDC hdc;
	//

	//
	//	
	//
	//short height, width;
	//double diffH, diffW;

BOOL SNOWMODEL_OnCreate(HWND hWnd, LPCREATESTRUCT)
{
	HWND hChild;
	RECT rect, clRect;
	LV_COLUMN lvc;//Структура для заголовка списка

	hEditLayerDlg = NULL;
	hTemperatureDlg = NULL;

	new CSnow();
	//CSnow::current()->add(0.1, 300, 0);
	//CSnow::current()->add(0.05, 50, 300);
	
	hChild = CreateWindow(WC_LISTVIEW, _T("Test"), WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, LISTBOX_X, LISTBOX_Y, LISTBOX_WIDTH, LISTBOX_HEIGHT, hWnd, (HMENU)IDC_LAYERS_LIST, hInst, NULL);
	ListView_SetExtendedListViewStyle(hChild,  LVS_EX_AUTOSIZECOLUMNS | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	ZeroMemory(&lvc,sizeof(lvc));
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;//Маска
	lvc.fmt=LVCFMT_LEFT;//Выравнивание влево
	lvc.cx=20;//Размер первого поля заголовка
	lvc.pszText=_T("#");//Строка с текстом первого поля заголовка
	ListView_InsertColumn(hChild,0,&lvc);//Включаем поле в заголовок
	lvc.cx=LISTBOX_WIDTH/3 - 10;//Размер первого поля заголовка
	lvc.pszText=_T("Density");//Строка с текстом первого поля заголовка
	ListView_InsertColumn(hChild,1,&lvc);//Включаем поле в заголовок
	lvc.pszText=_T("Height");//Строка с текстом второго поля заголовка
	ListView_InsertColumn(hChild,2,&lvc);//Включаем поле в заголовок
	lvc.pszText=_T("P Size");//Строка с текстом второго поля заголовка
	ListView_InsertColumn(hChild,3,&lvc);//Включаем поле в заголовок
	lvc.pszText=_T("T");//Строка с текстом второго поля заголовка
	ListView_InsertColumn(hChild,4,&lvc);//Включаем поле в заголовок
				
		
	CreateWindow(WC_BUTTON, _T("Add"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, LISTBOX_X, LISTBOX_Y + LISTBOX_HEIGHT + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_ADD_BTN, hInst, NULL);
	CreateWindow(WC_BUTTON, _T("Edit"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, LISTBOX_X + BUTTON_WIDTH + MARGIN, LISTBOX_Y + LISTBOX_HEIGHT + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_EDIT_BTN, hInst, NULL);
	CreateWindow(WC_BUTTON, _T("Delete"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, LISTBOX_X + 2*BUTTON_WIDTH + 2*MARGIN, LISTBOX_Y + LISTBOX_HEIGHT + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_DELETE_BTN, hInst, NULL);
		
	CreateWindow(szGraphClass, _T("Graph"), WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, GRAPHWND_X, GRAPHWND_Y, GRAPHWND_WIDTH, GRAPHWND_HEIGHT, hWnd, (HMENU)IDC_GRAPH_WINDOW, hInst, NULL);
	CreateWindow(WC_EDIT, CSnow::current()->ToString().c_str(), WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, INFOWND_X, INFOWND_Y, INFOWND_WIDTH, INFOWND_HEIGHT, hWnd, (HMENU)IDC_INFO_WINDOW, hInst, NULL);
	CreateWindow(szLayersClass, _T("Layers"), WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, LAYERSWND_X, LAYERSWND_Y, LAYERSWND_WIDTH, LAYERSWND_HEIGHT, hWnd, (HMENU)IDC_LAYERS_WINDOW, hInst, NULL);

	GetWindowRect(hWnd, &rect);
	GetClientRect(hWnd, &clRect);
	MoveWindow(hWnd, rect.left, rect.top, 2*(rect.right - rect.left) - clRect.right, 2*(rect.bottom - rect.top) - clRect.bottom, TRUE);
	CreateWindow(PROGRESS_CLASS, _T(""), WS_CHILD, (rect.right - BUTTON_WIDTH)/2, (rect.bottom - BUTTON_HEIGHT)/2, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_PROGRESS, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_PROGRESS, PBM_SETRANGE, NULL, MAKELPARAM(0,100));
	SendDlgItemMessage(hWnd, IDC_PROGRESS, PBM_SETSTEP, 1, NULL);

	return TRUE;
}
void SNOWMODEL_OnSize(HWND hWnd, UINT state, int cx, int cy)
{		
		double diffW = (double)cx/MAINWND_WIDTH;
		double diffH = (double)cy/MAINWND_HEIGHT;

		MoveWindow(GetDlgItem(hWnd,IDC_LAYERS_LIST), (int)(LISTBOX_X*diffW), (int)(LISTBOX_Y*diffH), (int)(LISTBOX_WIDTH*diffW), (int)(LISTBOX_HEIGHT*diffH), TRUE);
		MoveWindow(GetDlgItem(hWnd,IDC_ADD_BTN), (int)(LISTBOX_X*diffW), (int)((LISTBOX_Y + LISTBOX_HEIGHT + MARGIN)*diffH), (int)(BUTTON_WIDTH*diffW), (int)(BUTTON_HEIGHT*diffH), TRUE);
		MoveWindow(GetDlgItem(hWnd,IDC_EDIT_BTN), (int)((LISTBOX_X + MARGIN + BUTTON_WIDTH)*diffW) , (int)((LISTBOX_Y + LISTBOX_HEIGHT + MARGIN)*diffH), (int)(BUTTON_WIDTH*diffW), (int)(BUTTON_HEIGHT*diffH), TRUE);
		MoveWindow(GetDlgItem(hWnd,IDC_DELETE_BTN), (int)((LISTBOX_X + 2*MARGIN + 2*BUTTON_WIDTH)*diffW), (int)((LISTBOX_Y + LISTBOX_HEIGHT + MARGIN)*diffH), (int)(BUTTON_WIDTH*diffW), (int)(BUTTON_HEIGHT*diffH), TRUE);
		MoveWindow(GetDlgItem(hWnd,IDC_LAYERS_WINDOW), (int)(LAYERSWND_X*diffW), (int)(LAYERSWND_Y*diffH), (int)(LAYERSWND_WIDTH*diffW), (int)(LAYERSWND_HEIGHT*diffH), TRUE);
		MoveWindow(GetDlgItem(hWnd,IDC_INFO_WINDOW), (int)(INFOWND_X*diffW), (int)(INFOWND_Y*diffH), (int)(INFOWND_WIDTH*diffW), (int)(INFOWND_HEIGHT*diffH), TRUE);
		MoveWindow(GetDlgItem(hWnd,IDC_GRAPH_WINDOW), (int)(GRAPHWND_X*diffW), (int)(GRAPHWND_Y*diffH), (int)(GRAPHWND_WIDTH*diffW), (int)(GRAPHWND_HEIGHT*diffH), TRUE);
}

int GetEncoderClsid(const TCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;

	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void SNOWMODEL_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	OPENFILENAME ofn;
	TCHAR szFilter[] = _T("Snow Data files(*.SLM)\0*.slm\0All files(*.*)\0*.*\0");
	TCHAR szImgFilter[] = _T("PNG(*.png)\0*.png\0All files(*.*)\0*.*\0");
	TCHAR szFileName[500] = _T("");
	TCHAR szFileTitle[500] = _T("");
	
	switch (id)
	{
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_FILEOPEN:
		//static Fl_File_Chooser* fc = NULL;
		/*if(!fc) {
			Fl_File_Chooser::show_label = "Format:";
			Fl_File_Chooser::all_files_label = "All files (*)";
			fc = new Fl_File_Chooser("", "", Fl_File_Chooser::SINGLE, "Open File");
			fc->show();
			while(fc->shown()) Fl::wait();
			delete fc;
			fc = NULL;
		}*/
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter=szFilter;
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=sizeof(szFileName);
		ofn.lpstrFileTitle=szFileTitle;
		ofn.nMaxFileTitle=sizeof(szFileTitle);
		ofn.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		if(GetOpenFileName(&ofn)){
			if (CSnow::current()->readFile(szFileName)) {
				SetWindowText(hWnd, CSnow::current()->getName().c_str());
				UpdateAll();
			}
		}			
		break;
	case IDM_FILESAVE:
		if(CSnow::current()->writeFile())
			break;
	case IDM_FILESAVEAS:
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter=szFilter;
		ofn.lpstrFile=szFileName;
		ofn.nMaxFile=sizeof(szFileName);
		ofn.lpstrFileTitle=szFileTitle;
		ofn.nMaxFileTitle=sizeof(szFileTitle);
		ofn.Flags=OFN_PATHMUSTEXIST;
		if(GetSaveFileName(&ofn)){
			std::wstring wstr(szFileName);
			if (ofn.nFilterIndex == 1 && wstr.substr(wstr.size() - 4, 4) != _T(".slm"))
				wstr += _T(".slm");
			CSnow::current()->setFileName(wstr);
			CSnow::current()->writeFile();
			SetWindowText(hWnd, CSnow::current()->getName().c_str());
		}
		break;
	case IDM_FILESAVEGRAPH:
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = szImgFilter;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = sizeof(szFileName);
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFileTitle = sizeof(szFileTitle);
		ofn.Flags = OFN_PATHMUSTEXIST;
		if (GetSaveFileName(&ofn)){
			std::wstring wstr(szFileName);
			if (ofn.nFilterIndex == 1 && wstr.substr(wstr.size() - 4, 4) != _T(".png"))
				wstr += _T(".png");
			//CSnow::current()->setFileName(wstr);
			//CSnow::current()->writeFile();
			/////////////////////////////////////////////////
			HDC hdc = GetDC(hWnd);
			HDC hdcLayer = CreateCompatibleDC(hdc);
			HBITMAP	hBmp = CreateCompatibleBitmap(hdc, 1000, 1400);
			SelectObject(hdcLayer, hBmp);
			
			double timeS = 0;
			double timeE = 1500;
			PatBlt(hdcLayer, 0, 0, 1000, 1400, WHITENESS);
			DrawGraph(hdcLayer, 50, 50, 800, 400, 1, timeS, timeE);
			DrawTimeLabels(hdcLayer, 50, 450, 800, timeS, timeE);
			DrawGraph(hdcLayer, 50, 500, 800, 400, 0, timeS, timeE);
			DrawTemperature(hdcLayer, 50, 1000, 800, 300, timeS, timeE);
			DrawGrid(hdcLayer, 50, 50, 800, 1300, timeS, timeE, 100, 0);
									
			Gdiplus::Bitmap bmp(hBmp, NULL);
			CLSID encoderClsid;
			GetEncoderClsid(_T("image/png"), &encoderClsid);
			bmp.Save(wstr.c_str(), &encoderClsid, NULL);
			
			ReleaseDC(hWnd, hdc);

			/////////////////////////////////////////////////
			SetWindowText(hWnd, CSnow::current()->getName().c_str());
		}
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	case IDM_TEMPERATURE:
		{
		HMENU hMenu = GetMenu(hWnd);
		UINT state = GetMenuState(hMenu, IDM_TEMPERATURE, MF_CHECKED);
		CheckMenuItem(hMenu, IDM_TEMPERATURE, state == MF_CHECKED ? 0 : MF_CHECKED);
		bInvalidateTemperature = TRUE;
		UpdateGraph();
		break;
		}
	case IDM_TEMPERATURE_DLG:
		{
		if (GetMenuState(GetMenu(hWnd), IDM_TEMPERATURE_DLG, MF_CHECKED) == 0)
			hTemperatureDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TEMP_DLG), hWnd, TempDlgProc);
		else
			DestroyWindow(hTemperatureDlg);
		break;
		}
	case IDC_EDIT_BTN:
		bEditLayer = TRUE;
		if (!hEditLayerDlg)
			hEditLayerDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_EDIT_LAYER), hWnd, EditLayerDlgProc);
		
		SetWindowText(hEditLayerDlg, _T("Edit"));
		SetDlgItemText(hEditLayerDlg, IDOK, _T("Edit"));

		UpdateEditLayerData();
		//SetDlgItemText(hEditLayerDlg, IDC_EDIT1, _T(""));
		//SetDlgItemText(hEditLayerDlg, IDC_EDIT2, _T(""));
		break;
	case IDC_ADD_BTN:
		bEditLayer = FALSE;
		if (!hEditLayerDlg)
			hEditLayerDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_EDIT_LAYER), hWnd, EditLayerDlgProc);
		
		SetWindowText(hEditLayerDlg, _T("Add"));
		SetDlgItemText(hEditLayerDlg, IDOK, _T("Add"));
		//EnableWindow(GetDlgItem(hEditDlg, IDOK), TRUE);
		break;
	case IDC_DELETE_BTN:
		if (CSnow::current()->remove(CSnow::current()->getSelection()))
			UpdateAll();
		break;
	default:
		FORWARD_WM_COMMAND(hWnd, id, hwndCtl, codeNotify, DefWindowProc);
	}
}
void SNOWMODEL_OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	FORWARD_WM_KEYDOWN(hWnd, vk, cRepeat, flags, SendMessage);
}
void SNOWMODEL_OnNotify(HWND hWnd, int id, LPNMHDR lpnmhdr)
{
	if(lpnmhdr->idFrom == IDC_LAYERS_LIST) {
		if(lpnmhdr->code == LVN_ITEMCHANGED) {
			if(((NM_LISTVIEW *)lpnmhdr)->uNewState & LVIS_FOCUSED) {
				CSnow* snow = CSnow::current();
				snow->setSelection(ListView_GetItemCount(lpnmhdr->hwndFrom) - ((NM_LISTVIEW *)lpnmhdr)->iItem - 1);
				UpdateCurrentLayer();
			}
		}
	}
}
void SNOWMODEL_OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	// TODO: добавьте любой код отрисовки...
	EndPaint(hWnd, &ps);
}
void SNOWMODEL_OnDestroy(HWND hWnd)
{
	for(int i = CSnow::list.size() - 1; i >= 0; i--)
		delete CSnow::list[i];

	PostQuitMessage(0);
}

void UpdateGraph() { InvalidateRect(GetDlgItem(hMainWnd, IDC_GRAPH_WINDOW), NULL, FALSE); }
void UpdateLayersView() { InvalidateRect(GetDlgItem(hMainWnd, IDC_LAYERS_WINDOW), NULL, TRUE); }
void UpdateInfo() { SetDlgItemText(hMainWnd, IDC_INFO_WINDOW, CSnow::current()->ToString().c_str()); }
void UpdateLayersList()
{
	//SendDlgItemMessage(hMainWnd, IDC_LAYERS_LIST, LB_RESETCONTENT, NULL, NULL);
	HWND hListbox = GetDlgItem(hMainWnd, IDC_LAYERS_LIST);

	int iCount = ListView_GetItemCount(hListbox);
	int j = -1;
	for (int i = CSnow::current()->getLayersNum() - 1; i >= 0; i--) {		
		if (CSnow::current()->getLayer(i)->GetHeight() == 0) continue;
		j++;
		TCHAR str[80];
		if(j >= iCount) {
			LV_ITEM lvi;
			ZeroMemory(&lvi, sizeof (lvi)) ; //Обнуляем структуру
			lvi.mask = LVIF_TEXT;//Maска
			lvi.pszText = str;
			lvi.iItem = j;
			wsprintf(str, _T("%d"), i + 1);
			ListView_InsertItem(hListbox,&lvi);
		}

		wsprintf(str, _T("%d"), i + 1);
		ListView_SetItemText(hListbox, j, 0, str);
		wcscpy_s(str, CSnow::current()->getLayer(i)->ToString(CLayer::DENSITY).c_str());
		ListView_SetItemText(hListbox, j, 1, str);
		wcscpy_s(str, CSnow::current()->getLayer(i)->ToString(CLayer::HEIGHT).c_str());
		ListView_SetItemText(hListbox, j, 2, str);
		wcscpy_s(str, CSnow::current()->getLayer(i)->ToString(CLayer::PARTICLE).c_str());
		ListView_SetItemText(hListbox, j, 3, str);
		wcscpy_s(str, CSnow::current()->getLayer(i)->ToString(CLayer::TEMPERATURE).c_str());
		ListView_SetItemText(hListbox, j, 4, str);
	}

	if (iCount > ++j)
		for (int i = j; i < iCount; i++) ListView_DeleteItem(hListbox, j);
}

void UpdateTemperatureLog()
{
	if (!hTemperatureDlg)
		return;

	HWND hListbox = GetDlgItem(hTemperatureDlg, IDC_TEMPERATURE_LIST);

	ListView_DeleteAllItems(hListbox);
	
	for (int i = 0; i < CSnow::current()->getTLogSize(); i++) {		
		TCHAR str[80];
		LV_ITEM lvi;
		ZeroMemory(&lvi, sizeof (lvi)) ; //Обнуляем структуру
		lvi.mask = LVIF_TEXT;//Maска
		lvi.pszText = str;
		lvi.iItem = i;

		CSnow* snow = CSnow::current();
		wcscpy_s(str, ToString(snow->getTLog(i).first).c_str());
		//wcscpy_s(str, snow->ToString(snow->getTLog(i).first, CSnow::TIME).c_str());
		ListView_InsertItem(hListbox,&lvi);
		wcscpy_s(str, ToString(snow->getTLog(i).second).c_str());
		//wcscpy_s(str, snow->ToString(snow->getTLog(i).first, CSnow::TEMPERATURE).c_str());
		ListView_SetItemText(hListbox, i, 1, str);
		
	}
}

void UpdateCurrentTime()
{
	UpdateInfo();
	UpdateLayersList();	
	UpdateGraph();
	UpdateLayersView();
}

void UpdateEditLayerData()
{
	if (hEditLayerDlg && bEditLayer)
	{
		CLayer* layer = CSnow::current()->getLayer();
		if (layer)
		{
			SetDlgItemText(hEditLayerDlg, IDC_EDIT1, ToString(layer->GetTime0()).c_str());
			SetDlgItemText(hEditLayerDlg, IDC_EDIT2, ToString(layer->GetDuration()).c_str());
			SetDlgItemText(hEditLayerDlg, IDC_EDIT3, ToString(layer->GetLifeTime()).c_str());
			SetDlgItemText(hEditLayerDlg, IDC_EDIT4, ToString(layer->GetDensity0()).c_str());
			SetDlgItemText(hEditLayerDlg, IDC_EDIT5, ToString(layer->GetParticleSize0()).c_str());
			SetDlgItemText(hEditLayerDlg, IDC_EDIT6, ToString(layer->GetHeight0()).c_str());
		}
	}
}
void UpdateCurrentLayer()
{
	UpdateInfo();
	UpdateLayersView();
	UpdateEditLayerData();
}

void UpdateAll()
{
	bInvalidateGraph = TRUE;
	UpdateGraph();
	UpdateInfo();
	UpdateLayersList();	
	UpdateLayersView();
	UpdateTemperatureLog();
}

//LRESULT CALLBACK WndInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	int wmId, wmEvent;
//	PAINTSTRUCT ps;
//	HDC hdc;
//	
//	switch (message)
//	{
//	case WM_PAINT:
//		hdc = BeginPaint(hWnd, &ps);
//		// TODO: добавьте любой код отрисовки...
//		EndPaint(hWnd, &ps);
//		break;
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//	default:
//		return DefWindowProc(hWnd, message, wParam, lParam);
//	}
//	return 0;
//}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}