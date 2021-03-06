// P1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "P3.h"
#include <commdlg.h>
#include <CommCtrl.h>
#include <string>
#include "IncludeAll.h"


#define MAX_LOADSTRING 100
#define ID_TOOLBAR		1000	// ID of the toolbar
#define IMAGE_WIDTH     18
#define IMAGE_HEIGHT    17
#define BUTTON_WIDTH    0
#define BUTTON_HEIGHT   0
#define TOOL_TIP_MAX_LEN   32

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szDrawWndClass[MAX_LOADSTRING] = L"Draw child class";
WCHAR szDrawWndTitle[MAX_LOADSTRING] = L"Draw window title";
HWND hwndToolBarWnd;
int nCurrentSelectedMenuItem;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    MDICloseProc(HWND,LPARAM);
LRESULT CALLBACK    DrawWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



void OnNewDrawWnd(HWND);
void doCreate_ToolBar(HWND);
void doToolBar_NotifyHandle(LPARAM);
void doToolBar_AddSTDButton();
void doToolBar_AddUserButton();
void doToolBar_Style(int);
void doView_ToolBar(HWND);
void UpdateMenuState(HWND, int);


void OnLButtonDown(HWND, LPARAM , int &, int &, int &, int &, Text*, PDRAW); 
void OnMouseMove(HWND, WPARAM, LPARAM, int, int, int &, int &, COLORREF);
void OnEditLButtonDown(HWND, LPARAM, PDRAW);
void OnEditMouseMove(HWND, LPARAM, WPARAM, Object*, PDRAW);
void DrawBitmap(HDC);


WCHAR inputText[256];
INT_PTR CALLBACK GetText(HWND, UINT, WPARAM, LPARAM);
bool openFile = false;
bool saveFile = false;


HWND hwndMDIClient, hwndFrame;
int count = 1;
HMENU hMenu;

int X1, Y1, X2, Y2;
bool beginDraw = false;
bool endDraw = false;
bool isBitmap = false;
int right = -1;

// color
CHOOSECOLOR cc;
COLORREF accCustClr[16];
DWORD rgbCurrent = RGB(255, 0, 0);


// font
CHOOSEFONT cf;
LOGFONT lf;

// Open/save file
OPENFILENAME ofn;
TCHAR szFile[256];
TCHAR szFilter[] = L"Draw(*.drw)\0*.drw\0All file\0*.\0"	; 
HWND activeChild;
bool SaveFile(HWND, const wchar_t*);
bool OpenFile(HWND, const wchar_t*, PDRAW);

BOOL CALLBACK PaintAll(HWND, LPARAM);


char textObj[] = "TEXT";
char circleObj[] = "CIRCLE";
char recObj[] = "RECTANGLE";
char lineObj[] = "LINE";

UINT CF_TEXTCB = RegisterClipboardFormatA(textObj);
UINT CF_RECT = RegisterClipboardFormatA(recObj);
UINT CF_CIRCLE = RegisterClipboardFormatA(circleObj);
UINT CF_LINE = RegisterClipboardFormatA(lineObj);

void CopyObject();
void CutObject();
void DeleteObject();
void PasteObject(HWND);
HGLOBAL hGlobal;
int copyType = -1;
HGLOBAL hGlb;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_P1, szWindowClass, MAX_LOADSTRING);
	//InitApplication();
	MyRegisterClass(hInst);
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_P1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
		if (!TranslateAccelerator(hwndFrame, hAccelTable, &msg) && !TranslateMDISysAccel(hwndMDIClient, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_P1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_P1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (!RegisterClassExW(&wcex))
		return FALSE;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpszClassName = szDrawWndClass;
	wcex.lpfnWndProc = DrawWndProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDC_P1));
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 8;


	if (!RegisterClassExW(&wcex))
		return FALSE;

	return TRUE;
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, L"MyPaint", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//



//void OnNewDrawWnd(HWND hWnd)
//{
//	MDICREATESTRUCT mdiCreate;
//	WCHAR buff[100];
//	wsprintf(buff, L"Noname-%d.drw", count);
//	ZeroMemory(&mdiCreate, sizeof(MDICREATESTRUCT));
//
//	mdiCreate.szClass = L"Child class";
//	mdiCreate.szTitle = buff;
//	mdiCreate.cx = mdiCreate.x = CW_USEDEFAULT;
//	mdiCreate.cy = mdiCreate.y = CW_USEDEFAULT;
//	mdiCreate.hOwner = hInst;
//	mdiCreate.style = 0;
//	mdiCreate.lParam = NULL;
//	SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mdiCreate);
//}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	hMenu = GetMenu(hWnd);

    switch (message)
    {
	case WM_CREATE:
	{
		
		CLIENTCREATESTRUCT ccs;
		ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), 3);
		ccs.idFirstChild = 50002;
		hwndMDIClient = CreateWindow(L"MDICLIENT", (LPCTSTR)NULL, WS_CHILD | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 0, 0, 0, 0, hWnd, (HMENU)NULL, hInst, (LPVOID)&ccs);
		ShowWindow(hwndMDIClient, SW_SHOW);
		doCreate_ToolBar(hWnd);
		doToolBar_AddSTDButton();
		doToolBar_AddUserButton();
		return 0;
	}
	break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
			switch (wmId)
			{
			case ID_FILE_NEW:
			{
				MDICREATESTRUCT mdiCreate;
				WCHAR buff[100];
				wsprintf(buff, L"Noname-%d.drw", count);
				ZeroMemory(&mdiCreate, sizeof(MDICREATESTRUCT));
				mdiCreate.szClass = szDrawWndClass;
				mdiCreate.szTitle = buff;
				mdiCreate.cx = mdiCreate.x = CW_USEDEFAULT;
				mdiCreate.cy = mdiCreate.y = CW_USEDEFAULT;
				mdiCreate.hOwner = hInst;
				mdiCreate.style = 0;
				mdiCreate.lParam = NULL;
				SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mdiCreate);
				count++;
			}

			break;

			case ID_DRAW_FONT:
			{
				ZeroMemory(&cf, sizeof(CHOOSEFONT));
				cf.lStructSize = sizeof(CHOOSEFONT);
				cf.hwndOwner = hWnd;
				cf.lpLogFont = &lf;
				cf.rgbColors = rgbCurrent;
				cf.Flags = CF_SCREENFONTS | CF_EFFECTS;
				ChooseFont(&cf);
				UpdateMenuState(hWnd, wmId);
			}
			break;

			case ID_DRAW_COLOR:
			{
				ZeroMemory(&cc, sizeof(CHOOSECOLOR));
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = hWnd;
				cc.lpCustColors = (LPDWORD)accCustClr;
				cc.rgbResult = rgbCurrent;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT;
				ChooseColor(&cc);
				UpdateMenuState(hWnd, wmId);
			}
			break;

			case ID_WINDOW_VIEWHIDE:
				doView_ToolBar(hWnd);
				break;
			case ID_DRAW_LINE:
				UpdateMenuState(hWnd, wmId);
				break;
			case ID_DRAW_RECTANGLE:
				UpdateMenuState(hWnd, wmId);
				break;
			case ID_DRAW_ELLIPSE:
				UpdateMenuState(hWnd, wmId);
				break;
			case ID_DRAW_TEXT:
				UpdateMenuState(hWnd, wmId);
				break;

			case ID_FILE_OPEN:
			{
				MessageBox(hWnd, L"Bạn đã chọn Open", L"Notice", NULL);
				szFile[0] = '\0';
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = szFilter;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = szFile; // file path
				ofn.nMaxFile = sizeof(szFile);
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn))
				{
					openFile = true;
					saveFile = true;
					std::wstring path(ofn.lpstrFile);

					std::wstring tmp, name;
					for (int i = path.size() - 1; path.at(i) != 92; i--)
					{
						tmp.push_back(path[i]);
					}
					for (int i = tmp.size() - 1; i >= 0; i--)
					{
						name.push_back(tmp[i]);
					}
					MDICREATESTRUCT mdiCreate;
					mdiCreate.szClass = szDrawWndClass;
					mdiCreate.szTitle = name.c_str();
					mdiCreate.hOwner = hInst;
					mdiCreate.x = CW_USEDEFAULT;
					mdiCreate.y = CW_USEDEFAULT;
					mdiCreate.cx = CW_USEDEFAULT;
					mdiCreate.cy = CW_USEDEFAULT;
					mdiCreate.style = 0;
					mdiCreate.lParam = NULL;
					SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mdiCreate);
				}
				else
					openFile = false;
			}
			break;

			case ID_FILE_SAVE:
			{
				MessageBox(hWnd, L"Bạn đã chọn Save", L"Notice", NULL);

				if (!saveFile)
				{
					saveFile = true;
					szFile[0] = '\0';
					ZeroMemory(&ofn, sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = szFilter;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = szFile; // file path
					ofn.nMaxFile = sizeof(szFile);
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if (GetSaveFileName(&ofn))
					{
						std::wstring path(ofn.lpstrFile);
						path.append(L".drw");
						std::wstring tmp, name;
						for (int i = path.size() - 1; path.at(i) != 92; i--)
							tmp.push_back(path[i]);

						for (int i = tmp.size() - 1; i >= 0; i--)
							name.push_back(tmp[i]);

						SaveFile(activeChild, path.c_str());
						SetWindowText(activeChild, name.c_str());
						MessageBox(NULL, L"Save's done", L"Notification", NULL);
					}
					else
						MessageBox(NULL, L"Save's been cancelled", L"Notification", NULL);
				}
				else
				{
					std::wstring path(ofn.lpstrFile);
					path.append(L".drw");
					std::wstring tmp, name;

					for (int i = path.size() - 1; path.at(i) != 92; i--)
						tmp.push_back(path[i]);

					for (int i = tmp.size() - 1; i >= 0; i--)
						name.push_back(tmp[i]);

					SaveFile(activeChild, path.c_str());
					SetWindowText(activeChild, name.c_str());
					MessageBox(hWnd, L"Save's done", L"Notification", NULL);
				}
			}
			break;

			case ID_WINDOW_CLOSEALL:
				EnumChildWindows(hwndMDIClient, (WNDENUMPROC)MDICloseProc, 0L);
				break;
			case ID_WINDOW_TILE:
				SendMessage(hwndMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);
				break;
			case ID_WINDOW_CASCADE:
				SendMessage(hwndMDIClient, WM_MDICASCADE, MDITILE_ZORDER, 0);
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

			case ID_HELP_DRAW:
			{
				HMENU h = GetSubMenu(GetMenu(hWnd), 4);
				CheckMenuItem(h, ID_HELP_SELECTOBJECT, MF_UNCHECKED);
				CheckMenuItem(h, ID_HELP_DRAW, MF_CHECKED);
				RECT rect;
				GetClientRect(activeChild, &rect);
				InvalidateRect(activeChild, &rect, TRUE);
			}
			break;

			case ID_HELP_SELECTOBJECT:
			{
				HMENU h = GetSubMenu(GetMenu(hWnd), 4);
				CheckMenuItem(h, ID_HELP_SELECTOBJECT, MF_CHECKED);
				CheckMenuItem(h, ID_HELP_DRAW, MF_UNCHECKED);
				RECT rect;
				GetClientRect(activeChild, &rect);
				InvalidateRect(activeChild, &rect, TRUE);
			}
			break;

			case ID_EDIT_COPY:
			{
				CopyObject();
			}
			break;

			case ID_EDIT_CUT:
			{
				CutObject();
			}
			break;

			case ID_EDIT_DELETE:
			{
				DeleteObject();
			}
			break;

			case ID_EDIT_PASTE:
			{
				PasteObject(activeChild);
				RECT rect;
				GetClientRect(activeChild, &rect);
				InvalidateRect(activeChild, &rect, TRUE);
			}
			break;

            default:
				return DefFrameProc(hWnd, hwndMDIClient, message, wParam, lParam);
            }
        }
        break;
	case WM_NOTIFY:
		doToolBar_NotifyHandle(lParam);
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

	case WM_SIZE:
	{
		UINT w, h;
		w = LOWORD(lParam);
		h = HIWORD(lParam);
		MoveWindow(hwndMDIClient, 0, 32, w, h, TRUE);
	}

	break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
		return DefFrameProc(hWnd, hwndMDIClient, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK  MDICloseProc(HWND hChildWnd, LPARAM lParam)
{
	SendMessage(hwndMDIClient, WM_MDIDESTROY, (WPARAM)hChildWnd, 0L);
	return 1;
}



LRESULT CALLBACK DrawWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PDRAW p;
	Line* line;
	Rec* rectangle;
	Row* row;
	Text* text = new Text;
	
	switch (message)
	{

	case WM_MDIACTIVATE:
	{
		activeChild = hWnd;
		break;
	}
	break;

	case WM_MOUSEMOVE:
	{
		p = (PDRAW)GetWindowLongPtr(hWnd, 0);
		p->currentColor = cc.rgbResult;

		if (GetMenuState(hMenu, ID_HELP_SELECTOBJECT, MF_BYCOMMAND) == MF_CHECKED)
		{
			if (p->selectIndex != -1)
				OnEditMouseMove(hWnd, lParam, wParam, p->store.at(p->selectIndex), p);
		}
		else
		{
			OnMouseMove(hWnd, wParam, lParam, X1, Y1, X2, Y2, p->currentColor);
		}
		break;
	}
	case WM_LBUTTONDBLCLK:
	{
		p= (PDRAW)GetWindowLongPtr(hWnd, 0);

		if (GetMenuState(hMenu, ID_HELP_SELECTOBJECT, MF_BYCOMMAND) == MF_CHECKED)
		{
			if (p->selectIndex != -1)
			{
				if (p->store.at(p->selectIndex)->isText() == TRUE)
				{
					if (p->store.at(p->selectIndex)->makeChange(hWnd, lParam) != -1)
					{
						ZeroMemory(&cf, sizeof(cf));
						cf.lStructSize = sizeof(cf);
						cf.hwndOwner = hWnd;
						cf.lpLogFont = &lf;
						cf.rgbColors = rgbCurrent;
						cf.Flags = CF_SCREENFONTS | CF_EFFECTS;
						if (!ChooseFont(&cf)) MessageBox(hWnd, L"Error cant choose font", NULL, MB_OK);
						lf = *cf.lpLogFont;
						p->store.at(p->selectIndex)->setFont(lf);

						HDC hdc = GetDC(hWnd);
						HFONT font = CreateFontIndirect(&lf);
						SelectObject(hdc, font);
						SIZE size;
						const wchar_t* str = p->store.at(p->selectIndex)->getString();
						GetTextExtentPoint32(hdc, str, lstrlen(str), &size);

						RECT rec1 = p->store.at(p->selectIndex)->getPos();
						rec1.right = rec1.left + size.cx;
						rec1.bottom = rec1.top + size.cy;
						p->store.at(p->selectIndex)->setPos(rec1);
						// update pos
						RECT rec2;
						GetClientRect(hWnd, &rec2);
						InvalidateRect(hWnd, &rec2, TRUE);

						ZeroMemory(&cf, sizeof(cf));
					}
				}
			}
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		p = (PDRAW)GetWindowLongPtr(hWnd, 0);

		if (GetMenuState(hMenu,ID_HELP_SELECTOBJECT, MF_BYCOMMAND) == MF_CHECKED)
		{
			OnEditLButtonDown(hWnd, lParam, p);
		}
		else
		{
			OnLButtonDown(hWnd, lParam, X1, Y1, X2, Y2, text, p);
		}
		break;
	}

	case WM_CREATE:
	{
		p = (PDRAW)VirtualAlloc(NULL, sizeof(DRAW), MEM_COMMIT, PAGE_READWRITE);
		p->currentColor = cc.rgbResult;
		p->currentFont = lf;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)p);

		p->selectIndex = -1;
		if (openFile)
		{
			std::wstring path(ofn.lpstrFile);
			OpenFile(hWnd, path.c_str(), p);
			RECT rect;
			GetClientRect(hWnd, &rect);
			InvalidateRect(hWnd, &rect, TRUE);
			openFile = false;
			saveFile = true;
		}
		else
			saveFile = false;
	}
	break;

	case WM_LBUTTONUP:
	{
		p = (PDRAW)GetWindowLongPtr(hWnd, 0);
		if (GetMenuState(hMenu, ID_HELP_DRAW, MF_BYCOMMAND) == MF_CHECKED)
		{
			if (GetMenuState(hMenu, ID_DRAW_TEXT, MF_BYCOMMAND) == MF_UNCHECKED)
			{
				RECT r;
				r.left = X1;
				r.top = Y1;
				r.right = X2;
				r.bottom = Y2;
		
				if (r.right != r.left && r.top != r.bottom)
				{
					if (GetMenuState(hMenu, ID_DRAW_LINE, MF_BYCOMMAND) == MF_CHECKED)
					{
						line = new Line();
						line->setColor(p->currentColor);
						line->setPos(r);
						p->store.push_back(line);
					}
					if (GetMenuState(hMenu,ID_DRAW_RECTANGLE, MF_BYCOMMAND) == MF_CHECKED)
					{
						rectangle = new Rec();
						rectangle->setColor(p->currentColor);
						rectangle->setPos(r);
						p->store.push_back(rectangle);
					}
					if (GetMenuState(hMenu, ID_DRAW_ELLIPSE, MF_BYCOMMAND) == MF_CHECKED)
					{
						row = new Row();
						row->setColor(p->currentColor);
						row->setPos(r);
						p->store.push_back(row);
					}
				}
			}
		}
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		p= (PDRAW)GetWindowLongPtr(hWnd, 0);
		if (GetMenuState(hMenu, ID_HELP_SELECTOBJECT, MF_BYCOMMAND) == MF_CHECKED)
		{

			for (unsigned int i = 0; i < p->store.size(); i++)
			{
				if (p->store.at(i) != NULL)
				{
					p->store.at(i)->redrawObject(hWnd);
					if (p->selectIndex == i)
						p->store.at(i)->selected(hWnd);
				}
			}
			if (isBitmap)
			{
				isBitmap = false;
				DrawBitmap(hdc);
			}
		}
		else
		{
			for (unsigned int i = 0; i < p->store.size(); i++)
			{
				if (p->store.at(i) != NULL)
				{
					p->store.at(i)->redrawObject(hWnd);
				}
			}
		}
		EndPaint(hWnd, &ps);
	}
	break;
	
	case WM_DESTROY:
	{
		p = (PDRAW)GetWindowLongPtr(hWnd, 0);
		p->store.clear();
		VirtualFree(p, 0, MEM_RELEASE);
		count--;
	}
	break;

	default:
		return DefMDIChildProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



// Message handler for about box.
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

void doCreate_ToolBar(HWND hWnd)
{
	// loading Common Control DLL
	InitCommonControls();

	TBBUTTON tbButtons[] =
	{
	{ STD_FILENEW,	ID_FILE_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ STD_FILEOPEN,	ID_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ STD_FILESAVE,	ID_FILE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	};

	// create a toolbar
	hwndToolBarWnd = CreateToolbarEx(hWnd,
		WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR,
		sizeof(tbButtons) / sizeof(TBBUTTON),
		HINST_COMMCTRL,
		0,
		tbButtons,
		sizeof(tbButtons) / sizeof(TBBUTTON),
		BUTTON_WIDTH,
		BUTTON_HEIGHT,
		IMAGE_WIDTH,
		IMAGE_HEIGHT,
		sizeof(TBBUTTON));

	CheckMenuItem(GetMenu(hWnd), ID_WINDOW_VIEWHIDE, MF_CHECKED | MF_BYCOMMAND);
}

void doToolBar_NotifyHandle(LPARAM	lParam)
{
	LPTOOLTIPTEXT   lpToolTipText;
	TCHAR			szToolTipText[TOOL_TIP_MAX_LEN]; 	// ToolTipText, loaded from Stringtable resource
													// lParam: address of TOOLTIPTEXT struct
	lpToolTipText = (LPTOOLTIPTEXT)lParam;

	if (lpToolTipText->hdr.code == TTN_NEEDTEXT)
	{
		// hdr.iFrom: ID cua ToolBar button -> ID cua ToolTipText string
		LoadString(hInst, lpToolTipText->hdr.idFrom, szToolTipText, TOOL_TIP_MAX_LEN);

		lpToolTipText->lpszText = szToolTipText;
	}
}

void doToolBar_AddSTDButton()
{
	TBBUTTON tbButtons[] =
	{
	{ 0, 0,	TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0 },
	{ STD_CUT,ID_EDIT_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ STD_COPY,	ID_EDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ STD_PASTE, ID_EDIT_PASTE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ STD_DELETE,ID_EDIT_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }
	};

	SendMessage(hwndToolBarWnd, TB_ADDBUTTONS, (WPARAM) sizeof(tbButtons) / sizeof(TBBUTTON),
		(LPARAM)(LPTBBUTTON)&tbButtons);
}

void doToolBar_AddUserButton()
{

	TBBUTTON tbButtons[] =
	{
	{ 0, 0,	TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0 },
	{ 0, ID_DRAW_LINE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ 1, ID_DRAW_RECTANGLE,TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
	{ 2, ID_DRAW_ELLIPSE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0, },
	{ 3, ID_DRAW_TEXT,TBSTATE_ENABLED,TBSTYLE_BUTTON, 0, 0 },
	{ 4, ID_HELP_SELECTOBJECT,TBSTATE_ENABLED,TBSTYLE_BUTTON, 0, 0}
	};


	TBADDBITMAP	tbBitmap = { hInst,IDB_BITMAP1 };
	int idx = SendMessage(hwndToolBarWnd, TB_ADDBITMAP, (WPARAM) sizeof(tbBitmap) / sizeof(TBADDBITMAP),(LPARAM)(LPTBADDBITMAP)&tbBitmap);

	tbButtons[1].iBitmap += idx;
	tbButtons[2].iBitmap += idx;
	tbButtons[3].iBitmap += idx;
	tbButtons[4].iBitmap += idx;
	tbButtons[5].iBitmap += idx;

    SendMessage(hwndToolBarWnd, TB_ADDBUTTONS, (WPARAM) sizeof(tbButtons) / sizeof(TBBUTTON),
	(LPARAM)(LPTBBUTTON)&tbButtons);
}

void doToolBar_Style(int tbStyle)
{
	ShowWindow(hwndToolBarWnd, SW_HIDE);
	SendMessage(hwndToolBarWnd, TB_SETSTYLE, (WPARAM)0, (LPARAM)(DWORD)tbStyle | CCS_TOP);
	ShowWindow(hwndToolBarWnd, SW_SHOW);
}

void doView_ToolBar(HWND hWnd)
{
	int vFlag = GetMenuState(GetMenu(hWnd), ID_WINDOW_VIEWHIDE, MF_BYCOMMAND) & MF_CHECKED;

	if (vFlag)
	{
		ShowWindow(hwndToolBarWnd, SW_HIDE);
		vFlag = MF_UNCHECKED;
	}
	else
	{
		ShowWindow(hwndToolBarWnd, SW_SHOW);
		vFlag = MF_CHECKED;
	}
	
	CheckMenuItem(GetMenu(hWnd), ID_WINDOW_VIEWHIDE, vFlag | MF_BYCOMMAND);
}

void UpdateMenuState(HWND hWnd, int newMenuItem)
{
	HMENU hPopup = GetSubMenu(GetMenu(hWnd), 2);
	//Uncheck current selected item
	CheckMenuItem(hPopup, nCurrentSelectedMenuItem, MF_UNCHECKED | MF_BYCOMMAND);
	//check new item
	CheckMenuItem(hPopup, newMenuItem, MF_CHECKED | MF_BYCOMMAND);
	nCurrentSelectedMenuItem = newMenuItem;
}


bool OpenFile(HWND child, const wchar_t* path, PDRAW p)
{
	DWORD dwBytesWritten;
	int type;

	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	COLORREF color;
	ReadFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
	p->currentColor = color;
	LOGFONT f;
	ReadFile(hFile, &f, sizeof(LOGFONT), &dwBytesWritten, NULL);
	p->currentFont = f;

	RECT rec;

	for (;;)
	{
		ReadFile(hFile, &type, sizeof(int), &dwBytesWritten, NULL);
		if (dwBytesWritten == 0) 
			return true;

		switch (type)
		{
		case LINE:
		{
			Line* line = new Line();
			ReadFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
			line->setColor(color);
			ReadFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
			line->setPos(rec);
			p->store.push_back(line);
		}
		break;

		case REC:
		{
			Rec* rectangle = new Rec();
			ReadFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
			rectangle->setColor(color);
			ReadFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
			rectangle->setPos(rec);
			p->store.push_back(rectangle);
		}
		break;

		case ELLIPSE:
		{
			Row* row = new Row();
			ReadFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
			row->setColor(color);
			ReadFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
			row->setPos(rec);
			p->store.push_back(row);
		}
		break;

		case CTEXT:
		{
			Text* text = new Text();
			ReadFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
			text->setColor(color);
			ReadFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
			text->setPos(rec);
			WCHAR string[MAX_LOADSTRING];
			memset(string, 0, MAX_LOADSTRING);
			int stringlenght;
			ReadFile(hFile, &stringlenght, sizeof(int), &dwBytesWritten, NULL);
			ReadFile(hFile, &string, sizeof(WCHAR)*stringlenght, &dwBytesWritten, NULL);
			string[stringlenght] = '\0';
			text->setString(string);
			LOGFONT font;
			ReadFile(hFile, &font, sizeof(LOGFONT), &dwBytesWritten, NULL);
			text->setFont(font);
			p->store.push_back(text);
		}
		break;
		}
	}
	return true;
}

bool SaveFile(HWND child, const wchar_t* path)
{
	DWORD dwBytesWritten;
	int type;
	PDRAW pDraw = (PDRAW)GetWindowLongPtr(child, 0);
	HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	WriteFile(hFile, &pDraw->currentColor, sizeof(COLORREF), &dwBytesWritten, NULL);
	WriteFile(hFile, &pDraw->currentFont, sizeof(LOGFONTW), &dwBytesWritten, NULL);

	for (unsigned int i = 0; i < pDraw->store.size(); i++)
	{
		Object* tmp = pDraw->store.at(i);
		if (tmp != NULL)
		{
			if (tmp->isLine())
			{
				type = LINE;
				COLORREF color = tmp->getColor();
				RECT rec = tmp->getPos();
				WriteFile(hFile, &type, sizeof(int), &dwBytesWritten, NULL);
				WriteFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
				WriteFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
				continue;
			}
			if (tmp->isRec())
			{
				type = REC;
				COLORREF color = tmp->getColor();
				RECT rec = tmp->getPos();
				WriteFile(hFile, &type, sizeof(int), &dwBytesWritten, NULL);
				WriteFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
				WriteFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
				continue;
			}
			if (tmp->isRow())
			{
				type = ELLIPSE;
				COLORREF color = tmp->getColor();
				RECT rec = tmp->getPos();
				WriteFile(hFile, &type, sizeof(int), &dwBytesWritten, NULL);
				WriteFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
				WriteFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
				continue;
			}
			if (tmp->isText())
			{
				type = CTEXT;
				COLORREF color = tmp->getColor();
				RECT rec = tmp->getPos();
				const wchar_t* string = tmp->getString();
				WCHAR string1[MAX_LOADSTRING];
				int stringlenght = lstrlen(string);
				for (int i = 0; i < stringlenght; i++)
				{
					string1[i] = string[i];
				}
				LOGFONT font = tmp->getFont();
				WriteFile(hFile, &type, sizeof(int), &dwBytesWritten, NULL);
				WriteFile(hFile, &color, sizeof(COLORREF), &dwBytesWritten, NULL);
				WriteFile(hFile, &rec, sizeof(RECT), &dwBytesWritten, NULL);
				WriteFile(hFile, &stringlenght, sizeof(int), &dwBytesWritten, NULL);
				WriteFile(hFile, &string1, sizeof(WCHAR)*(stringlenght), &dwBytesWritten, NULL);
				WriteFile(hFile, &font, sizeof(LOGFONT), &dwBytesWritten, NULL);
				continue;
			}
		}

	}
	CloseHandle(hFile);
	return TRUE;
}

void OnEditLButtonDown(HWND hWnd, LPARAM lParam, PDRAW p)
{
	if (p->selectIndex == -1)
	{
		for (unsigned int i = 0; i < p->store.size(); i++)
		{
			if (p->store.at(i) != NULL)
			{
				if (p->store.at(i)->isSelect(lParam) == true)
				{
					p->store.at(i)->selected(hWnd);
					p->selectIndex = i;
					right = p->store.at(p->selectIndex)->makeChange(hWnd, lParam);
					break;
				}
			}

		}
	}
	else
	{
		right = p->store.at(p->selectIndex)->makeChange(hWnd, lParam);
		if (right == -1)
		{
			RECT rec;
			GetClientRect(hWnd, &rec);
			InvalidateRect(hWnd, &rec, TRUE);

			for (unsigned int i = 0; i < p->store.size(); i++)
			{
				if (p->store.at(i) != NULL)
				{
					if (p->store.at(i)->isSelect(lParam) == true)
					{
						p->store.at(i)->selected(hWnd);
						p->selectIndex = i;
						right = p->store.at(i)->makeChange(hWnd, lParam);
						return;
					}
				}
			}
			p->selectIndex = -1;
		}
	}
}

void OnEditMouseMove(HWND hWnd, LPARAM lParam, WPARAM wParam, Object* obj, PDRAW p)
{
	if (right != -1)
	{
		obj->reSize(hWnd, lParam, wParam, right);
	}
}

void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam, int x1, int y1, int &x2, int &y2, COLORREF pcolor)
{
	if (GetMenuState(hMenu, ID_DRAW_LINE, MF_BYCOMMAND) == MF_CHECKED)
	{
		if (!(wParam & MK_LBUTTON)) 
		{
			endDraw = true; 
			return; 
		}

		HDC hdc = GetDC(hWnd);
		HPEN hPen = CreatePen(PS_SOLID, 2, pcolor);
		SelectObject(hdc, hPen);

		SetROP2(hdc, R2_NOTXORPEN);
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);

		x2 = LOWORD(lParam);
		y2 = HIWORD(lParam);
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);

		ReleaseDC(hWnd, hdc);
	}

	if (GetMenuState(hMenu, ID_DRAW_RECTANGLE, MF_BYCOMMAND) == MF_CHECKED)
	{
		if (!(wParam & MK_LBUTTON)) return;
		HDC hdc = GetDC(hWnd);

		HPEN pen = CreatePen(PS_SOLID, 2, pcolor);
		SelectObject(hdc, pen);

		SetROP2(hdc, R2_NOTXORPEN);
		MoveToEx(hdc, x1, y1, NULL);
		Rectangle(hdc, x1, y1, x2, y2);

		x2 = LOWORD(lParam);
		y2 = HIWORD(lParam);
		MoveToEx(hdc, x1, y1, NULL);
		Rectangle(hdc, x1, y1, x2, y2);

		ReleaseDC(hWnd, hdc);
	}

	if (GetMenuState(hMenu, ID_DRAW_ELLIPSE, MF_BYCOMMAND) == MF_CHECKED)
	{
		if (!(wParam & MK_LBUTTON)) return;
		HDC hdc = GetDC(hWnd);
		
		HPEN hPen = CreatePen(PS_SOLID, 2, pcolor);
		SelectObject(hdc, hPen);

		SetROP2(hdc, R2_NOTXORPEN);
		MoveToEx(hdc, x1, y1, NULL);
		Ellipse(hdc, x1, y1, x2, y2);

		x2 = LOWORD(lParam);
		y2 = HIWORD(lParam);
		MoveToEx(hdc, x1, y1, NULL);
		Ellipse(hdc, x1, y1, x2, y2);

		ReleaseDC(hWnd, hdc);
	}
}

void OnLButtonDown(HWND hWnd, LPARAM lParam, int &x1, int &y1, int &x2, int &y2, Text* text, PDRAW p)
{
	POINTS pos = MAKEPOINTS(lParam);

	x1 = x2 = pos.x;
	y1 = y2 = pos.y;
	beginDraw = true;

	if (GetMenuState(hMenu, ID_DRAW_TEXT, MF_BYCOMMAND) == MF_CHECKED)
	{
		DialogBox(hInst, MAKEINTRESOURCE(IDD_TEXTDLG), hWnd, GetText);
		HDC hdc = GetDC(hWnd);
		HFONT hFont = CreateFontIndirect(cf.lpLogFont);
		SetTextColor(hdc, cc.rgbResult);
		SelectObject(hdc, hFont);

		TextOut(hdc, x1, y1, inputText, lstrlen(inputText));
		SIZE size;
		GetTextExtentPoint32(hdc, inputText, lstrlen(inputText), &size);

		p->currentColor = cc.rgbResult;
		if (cf.lpLogFont != NULL)
		{
			p->currentFont = *cf.lpLogFont;
		}
		else
		{
			p->currentFont = lf;
		}
		text->setColor(cc.rgbResult);
		text->setFont(p->currentFont);

		RECT rec;
		rec.left = x1;
		rec.right = rec.left + size.cx;
		rec.top = y1;
		rec.bottom = rec.top + size.cy;

		text->setPos(rec);
		text->setString(inputText);

		p->store.push_back(text);

		memset(inputText, 0, sizeof(inputText));
		ReleaseDC(hWnd, hdc);
	}
}

BOOL CALLBACK PaintAll(HWND hwnd, LPARAM lParam)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	InvalidateRect(GetParent(hwnd), &rect, TRUE);
	return TRUE;
}

void DrawBitmap(HDC hdc)
{
	if (OpenClipboard(NULL))
	{
		HANDLE hGlb = GetClipboardData(CF_BITMAP);
		if (hGlb == NULL)
		{
			CloseClipboard();
			return;
		}
		HBITMAP hBmp = /*(HBITMAP)GlobalLock(hglb)*/(HBITMAP)hGlb;

		HBITMAP hOldBmp;
		BITMAP bitmap;
		HDC memDC;

		GetObject(hBmp, sizeof(BITMAP), &bitmap);
		memDC = CreateCompatibleDC(hdc);

		hOldBmp = (HBITMAP)SelectObject(memDC, hBmp);

		BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, memDC, 0, 0, SRCCOPY);

		SelectObject(memDC, hOldBmp);
		DeleteObject(hBmp);
		DeleteDC(memDC);

		//GlobalUnlock(hglb);
		CloseClipboard();
		//copyType = -1;
	}
}

INT_PTR CALLBACK GetText(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT, L"Input your text here");
		return (INT_PTR)TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT, inputText, 256);
			EndDialog(hDlg, LOWORD(wParam)); break;
		case IDCANCEL:
			memset(inputText, 0, sizeof(inputText));
			EndDialog(hDlg, LOWORD(wParam)); break;
		}
	}
	if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
	{
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	}
	break;
	}
	return (INT_PTR)FALSE;
}


void CopyObject()
{
	PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);

	int selectObject = pDraw->selectIndex;

	if (selectObject != -1)
	{
		Object *tmp = pDraw->store[selectObject];

		OpenClipboard(NULL);
		EmptyClipboard();

		if (tmp->isLine())
		{
			copyType = LINE;
			hGlobal = GlobalAlloc(NULL, sizeof(Line));

			if (hGlobal)
			{
				Line *pshData = (Line*)GlobalLock(hGlobal);
				RECT rec = tmp->getPos();
				COLORREF color = tmp->getColor();

				pshData->pos = rec;
				pshData->color = color;
				SetClipboardData(CF_LINE, hGlobal);

				if (pshData)
				{
					GlobalUnlock(hGlobal);
				}
			}
		}

		if (tmp->isRec())
		{
			copyType = REC;
			hGlobal = GlobalAlloc(NULL, sizeof(Rec));
			if (hGlobal)
			{
				Rec *pshData = (Rec*)GlobalLock(hGlobal);
				RECT rec = tmp->getPos();
				COLORREF color = tmp->getColor();

				pshData->pos = rec;
				pshData->color = color;

				SetClipboardData(CF_RECT, hGlobal);

				if (pshData)
				{
					GlobalUnlock(hGlobal);
				}
			}
		}

		if (tmp->isRow())
		{
			copyType = ELLIPSE;
			hGlobal = GlobalAlloc(NULL, sizeof(Row));
			if (hGlobal)
			{
				Row *pshData = (Row*)GlobalLock(hGlobal);
				RECT rec = tmp->getPos();
				COLORREF color = tmp->getColor();

				pshData->pos = rec;
				pshData->color = color;

				SetClipboardData(CF_CIRCLE, hGlobal);

				if (pshData)
				{
					GlobalUnlock(hGlobal);
				}
			}
		}

		if (tmp->isText())
		{
			copyType = CTEXT;
			hGlobal = GlobalAlloc(NULL, sizeof(Text));
			if (hGlobal)
			{
				Text *pshData = (Text*)GlobalLock(hGlobal);
				RECT rec = tmp->getPos();
				COLORREF color = tmp->getColor();
				LOGFONT font = tmp->getFont();
				WCHAR* str = (WCHAR*)tmp->getString();

				pshData->pos = rec;
				pshData->color = color;
				pshData->cfont = font;
				pshData->text_str = str;

				SetClipboardData(CF_TEXTCB, hGlobal);

				if (pshData)
				{
					GlobalUnlock(hGlobal);
				}
			}
		}

		CloseClipboard();
	}
}

void CutObject()
{
	CopyObject();
	DeleteObject();
}

void DeleteObject()
{
	PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);
	int selectObject = pDraw->selectIndex;

	if (selectObject != -1 && pDraw->store.at(selectObject) != NULL)
	{
		delete pDraw->store.at(selectObject);
		pDraw->store.at(selectObject) = NULL;
		pDraw->selectIndex = -1;
		InvalidateRect(NULL, NULL, true);
	}
}

void PasteObject(HWND hWnd)
{
	if (OpenClipboard(NULL))
	{
		switch (copyType)
		{
		case LINE:
		{
			hGlb = GetClipboardData(CF_LINE);

			if (hGlb == 0)
			{
				MessageBox(hWnd, L"Clipboard is empty", L"Notification", NULL);
				CloseClipboard();
			}
			else
			{
				PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);
				Line *lpstr = (Line*)GlobalLock(hGlb);
				Line *tmp = new Line();
				tmp->setColor(lpstr->color);
				tmp->setPos(lpstr->pos);

				pDraw->store.push_back(tmp);
				GlobalUnlock(hGlb);
				EmptyClipboard();
				CloseClipboard();
			}
			copyType = -1;
		}
		break;

		case REC:
		{
			hGlb = GetClipboardData(CF_RECT);
			if (hGlb == 0)
			{
				MessageBox(hWnd, L"Clipboard is empty", L"Error", MB_ICONERROR);
				CloseClipboard();
			}
			else
			{
				PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);
				Rec *lpstr = (Rec*)GlobalLock(hGlb);
				Rec *tmp = new Rec();
				tmp->setColor(lpstr->color);
				tmp->setPos(lpstr->pos);

				pDraw->store.push_back(tmp);
				GlobalUnlock(hGlb);
				EmptyClipboard();
				CloseClipboard();
			}
			copyType = -1;
		}
		break;

		case ELLIPSE:
		{
			hGlb = GetClipboardData(CF_CIRCLE);
			if (hGlb == 0)
			{
				MessageBox(hWnd, L"Clipboard is empty", L"Error", MB_ICONERROR);
				CloseClipboard();
			}
			else
			{
				PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);
				Row *lpstr = (Row*)GlobalLock(hGlb);
				Row *tmp = new Row();
				tmp->setColor(lpstr->color);
				tmp->setPos(lpstr->pos);

				pDraw->store.push_back(tmp);
				GlobalUnlock(hGlb);
				EmptyClipboard();
				CloseClipboard();
			}
			copyType = -1;
		}
		break;

		case CTEXT:
		{
			hGlb = GetClipboardData(CF_TEXTCB);

			if (hGlb == 0)
			{
				MessageBox(hWnd, L"Clipboard is empty", L"Error", MB_ICONERROR);
				CloseClipboard();
			}
			else
			{
				PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);
				Text *lpstr = (Text*)GlobalLock(hGlb);
				Text *tmp = new Text();
				tmp->setColor(lpstr->color);
				tmp->setPos(lpstr->pos);
				tmp->setFont(lpstr->cfont);
				tmp->setString((WCHAR*)lpstr->text_str.c_str());

				pDraw->store.push_back(tmp);
				GlobalUnlock(hGlb);
				EmptyClipboard();
				CloseClipboard();
			}
			copyType = -1;
		}
		break;

		default:
		{
			HDC hdc = GetDC(hWnd);

			if (OpenClipboard(NULL))
			{
				if (IsClipboardFormatAvailable(CF_BITMAP))
				{
					copyType = -1;
					isBitmap = true;
					CloseClipboard();
					return;
				}
				else
				{
					HANDLE hglb = GetClipboardData(CF_UNICODETEXT);
					if (hglb == NULL)
					{
						MessageBox(hWnd, L"Clipboard is empty", L"Error", MB_ICONERROR);
						copyType = -1;
						CloseClipboard();
						break;
					}
					LPWSTR text = (LPWSTR)GlobalLock(hglb);
					PDRAW pDraw = (PDRAW)GetWindowLongPtr(activeChild, 0);
					Text *lpstr = (Text*)GlobalLock(hglb);
					Text *tmp = new Text();

					tmp->setColor(lpstr->color);
					tmp->setFont(lpstr->cfont);
					tmp->setString((WCHAR*)lpstr->text_str.c_str());

					RECT rec;
					rec.left = 0;
					rec.top = 0;
					SIZE size;
					GetTextExtentPoint32(hdc, text, lstrlen(text), &size);
					rec.right = rec.left + size.cx;
					rec.bottom = rec.top + size.cy;

					tmp->setPos(rec);
					pDraw->store.push_back(tmp);

					GlobalUnlock(hglb);
					CloseClipboard();
					copyType = -1;
					break;
				}
			}
		}
		}
	}
}