// CharlesMine.cpp : 定义应用程序的入口点。
//
#include "stdafx.h"
#include "CharlesMine.h"
#include "Game_def.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    CustomGameProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    HeroNameProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    HeroListProc(HWND, UINT, WPARAM, LPARAM);

void                UpdateINI();
void                LoadINI();
void                NewHero(int nGameType, int nTime);
void                ShowHeroes();
void                InitHeroes();

TCHAR               szHeroes[3][MAX_LOADSTRING];
int                 nHeroes[3];
extern              int nMapX, nMapY, nMine;
extern              int nGameMode;
extern              HWND hGame_MainWnd;
extern              bool bCheatEnabled;
extern              int nAdvancedState;

//////////////////////////////////////////////////
// extern Functions in MineProc.cpp
extern              void InitGame(HINSTANCE hInst, HWND hWnd);
extern				void DestroyGame();
extern              void LoadDefaultMap(int nMode);
extern              void InitMap();
extern				void Window_Paint(HWND hWnd);
extern				void LButton_Down(WPARAM wParam, LPARAM lParam);
extern				void LButton_Up(WPARAM wParam, LPARAM lParam);
extern				void RButton_Down(WPARAM wParam, LPARAM lParam);
extern				void RButton_Up(WPARAM wParam, LPARAM lParam);
extern				void Mouse_Move(WPARAM wParam, LPARAM lParam);
extern              bool ToggleMark();
extern              void LoadDefaultMap(int nMode);
extern              void RestartMap();
extern              bool LoadMap(LPCTSTR szFileName);
extern              bool SaveMap(LPCTSTR szFileName);

extern              bool AdvancedRecord(LPCTSTR szFileName);
extern              bool AdvancedPlayback(LPCTSTR szFileName);
extern              void AdvancedStop();

extern				void Key_Down(WPARAM wParam, LPARAM lParam);


extern              void EasterEgg(HWND hDlg);

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CHARLESMINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	InitHeroes();

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHARLESMINE));
	LoadINI();

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UpdateINI();
	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHARLESMINE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CHARLESMINE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= wcex.hIcon;

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindow(szWindowClass, szTitle, WS_CAPTION|WS_VISIBLE|WS_CLIPSIBLINGS|WS_SYSMENU|WS_OVERLAPPED|WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	InitGame(hInst, hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


BOOL HandleMapFile(bool bSave, UINT nFilterResID, LPCTSTR lpszDefExt, LPTSTR lpszFile)
{
	OPENFILENAME ofn;
	TCHAR szFilter[MAX_LOADSTRING];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.hwndOwner = hGame_MainWnd;
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = lpszFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	LoadString(hInst, nFilterResID, szFilter, MAX_LOADSTRING);
	for (int i=0; szFilter[i]!='\0'; i++) 
		if (szFilter[i] == '|') 
			szFilter[i] = '\0'; 	 
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrDefExt = lpszDefExt;
	if(bSave)
	{
		ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
		return GetSaveFileName(&ofn);
	}
	else
	{
		ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
		return GetOpenFileName(&ofn);
	}
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	TCHAR szFile[MAX_PATH];

	switch (message)
	{
	case WM_CREATE:
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_FILE_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_FILE_NEW:
			InitMap();
			break;
		case IDM_FILE_GAME_EASY:
			LoadDefaultMap(0);
			break;
		case IDM_FILE_GAME_MEDIUM:
			LoadDefaultMap(1);
			break;
		case IDM_FILE_GAME_HARD:
			LoadDefaultMap(2);
			break;
		case IDM_FILE_GAME_CUSTOM:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CUSTOM_GAME), hWnd, (DLGPROC)CustomGameProc);
			break;
		case IDM_FILE_HERO_LIST:
			ShowHeroes();
			break;
		case IDM_FILE_MARK:
			ToggleMark();
			break;
		case IDM_ADVANCED_LOADMAP:
			if (HandleMapFile(false, IDS_FILE_FILTER, _T("cmm"), szFile)) 
			{
				if(!LoadMap(szFile))
				{
					LoadDefaultMap(nGameMode);
					TCHAR szTemp[MAX_LOADSTRING];   
					LoadString(hInst, IDS_FILE_LOAD_ERROR, szTemp, MAX_LOADSTRING);
					MessageBox(hGame_MainWnd, szTemp, NULL, 0);
				}
				else
				{
					RestartMap();
				}
			}
			break;
		case IDM_ADVANCED_SAVEMAP:
			if (HandleMapFile(true, IDS_FILE_FILTER, _T("cmm"), szFile)) 
			{
				if(!SaveMap(szFile))
				{
					TCHAR szTemp[MAX_LOADSTRING];     
					LoadString(hInst, IDS_FILE_SAVE_ERROR, szTemp, MAX_LOADSTRING);
					MessageBox(hGame_MainWnd, szTemp, NULL, 0);
				}
			}
			break;
		case IDM_ADVANCED_RESTART:
			RestartMap();
			break;
		case IDM_ADVANCED_RECORD_RECORD:
			if (HandleMapFile(true, IDS_REPLAY_FILTER, _T("cmr"), szFile))
			{
				AdvancedRecord(szFile);
			}
			break;
		case IDM_ADVANCED_RECORD_PLAY:
			if (HandleMapFile(false, IDS_REPLAY_FILTER, _T("cmr"), szFile))
			{
				AdvancedPlayback(szFile);
			}
			break;
		case IDM_ADVANCED_RECORD_STOP:
			AdvancedStop();
			break;
		case IDM_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)About);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		Window_Paint(hWnd);  //MineProc.cpp
		break;
	case WM_LBUTTONDOWN:
		if(nAdvancedState!=2)
			LButton_Down(wParam, lParam);
		break;
	case WM_LBUTTONUP:
		if(nAdvancedState!=2)
			LButton_Up(wParam, lParam);
		break;
	case WM_RBUTTONDOWN:
		if(nAdvancedState!=2)
			RButton_Down(wParam, lParam);
		break;
	case WM_RBUTTONUP:
		if(nAdvancedState!=2)
			RButton_Up(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		if(nAdvancedState!=2)
			Mouse_Move(wParam, lParam);
		break;
	case WM_KEYDOWN:
		Key_Down(wParam, lParam);
		break;
	case WM_DESTROY:
		DestroyGame();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void InitHeroes()
{
	for(int i=0;i<3;i++)
	{
		LoadString(hInst, IDS_HERO_NAME, szHeroes[i], MAX_LOADSTRING);
		nHeroes[i] = 999;
	}
}

void LoadINI()
{
	HKEY hk;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, _TEXT("Software\\CrLF Soft\\CharlesMine"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hk, NULL))
	{
		return;
	}
	DWORD dwDataSize;
	dwDataSize=MAX_LOADSTRING;
	RegQueryValueEx(hk, _T("Name1"), 0, NULL, (BYTE *)szHeroes[0], &dwDataSize);
	RegQueryValueEx(hk, _T("Name2"), 0, NULL, (BYTE *)szHeroes[1], &dwDataSize);
	RegQueryValueEx(hk, _T("Name3"), 0, NULL, (BYTE *)szHeroes[2], &dwDataSize);
	dwDataSize=sizeof(int);     
	RegQueryValueEx(hk, _T("Time1"), 0, NULL, (BYTE *)&nHeroes[0], &dwDataSize);
	if(nHeroes[0]>999)nHeroes[0]=999;
	RegQueryValueEx(hk, _T("Time2"), 0, NULL, (BYTE *)&nHeroes[1], &dwDataSize);
	if(nHeroes[1]>999)nHeroes[1]=999;
	RegQueryValueEx(hk, _T("Time3"), 0, NULL, (BYTE *)&nHeroes[2], &dwDataSize);
	if(nHeroes[2]>999)nHeroes[2]=999;
	RegCloseKey(hk);

}

void UpdateINI()
{
	HKEY hk;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, _TEXT("Software\\CrLF Soft\\CharlesMine"), 
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL))
	{
		MessageBox(NULL, _T("Error"), 0, MB_SYSTEMMODAL);
		return;
	}
	RegSetValueEx(hk, _T("Name1"), 0, REG_SZ, (BYTE *)szHeroes[0], 50);
	RegSetValueEx(hk, _T("Name2"), 0, REG_SZ, (BYTE *)szHeroes[1], 50);
	RegSetValueEx(hk, _T("Name3"), 0, REG_SZ, (BYTE *)szHeroes[2], 50);
	RegSetValueEx(hk, _T("Time1"), 0, REG_DWORD, (BYTE *)&nHeroes[0], sizeof(int));
	RegSetValueEx(hk, _T("Time2"), 0, REG_DWORD, (BYTE *)&nHeroes[1], sizeof(int));
	RegSetValueEx(hk, _T("Time3"), 0, REG_DWORD, (BYTE *)&nHeroes[2], sizeof(int));

	RegCloseKey(hk);
}

void ShowHeroes()
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_HERO_LIST), hGame_MainWnd, (DLGPROC)HeroListProc);
}

void NewHero(int nTime)
{
	if(nGameMode<0||nGameMode>=3) return;
	if(nHeroes[nGameMode]<=nTime) return;

	TCHAR szLoadingString[MAX_LOADSTRING];
	LoadString(hInst, IDS_HERO_NAME, szLoadingString, MAX_LOADSTRING);
	_tcscpy(szHeroes[nGameMode],szLoadingString);
	nHeroes[nGameMode]=nTime;

	DialogBox(hInst, MAKEINTRESOURCE(IDD_HERO_NAME), hGame_MainWnd, (DLGPROC)HeroNameProc);

	ShowHeroes();
}

INT_PTR CALLBACK CustomGameProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TCHAR szLoadingString[MAX_LOADSTRING];
			LoadString(hInst, IDS_CUSTOMGAME, szLoadingString, MAX_LOADSTRING);
			SetWindowText(hDlg, szLoadingString); 
			LoadString(hInst, IDS_CUSTOMGAME_HEIGHT, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT1,szLoadingString);
			LoadString(hInst, IDS_CUSTOMGAME_WIDTH, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT2,szLoadingString);
			LoadString(hInst, IDS_CUSTOMGAME_MINE, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT3,szLoadingString);
			LoadString(hInst, IDOK, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDOK,szLoadingString);
			LoadString(hInst, IDCANCEL, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDCANCEL,szLoadingString);
			_stprintf(szLoadingString, _TEXT("%d"), nMapY);
			SetDlgItemText(hDlg,IDC_EDIT1, szLoadingString);
			_stprintf(szLoadingString, _TEXT("%d"), nMapX);
			SetDlgItemText(hDlg,IDC_EDIT2, szLoadingString);
			_stprintf(szLoadingString, _TEXT("%d"), nMine);
			SetDlgItemText(hDlg,IDC_EDIT3, szLoadingString);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			if(LOWORD(wParam) == IDOK)
			{
				TCHAR szLoadingString[MAX_LOADSTRING];
				GetDlgItemText(hDlg, IDC_EDIT1, szLoadingString, MAX_LOADSTRING);
				_stscanf(szLoadingString, _TEXT("%d"), &nMapY);
				if(nMapY<MAPY_MIN) nMapY=MAPY_MIN;
				if(nMapY>MAPY_MAX) nMapY=MAPY_MAX;
				GetDlgItemText(hDlg, IDC_EDIT2, szLoadingString, MAX_LOADSTRING);
				_stscanf(szLoadingString, _TEXT("%d"), &nMapX);
				if(nMapX<MAPX_MIN) nMapX=MAPX_MIN;
				if(nMapX>MAPX_MAX) nMapX=MAPX_MAX;
				GetDlgItemText(hDlg, IDC_EDIT3, szLoadingString, MAX_LOADSTRING);
				_stscanf(szLoadingString, _TEXT("%d"), &nMine);
				if(nMine<MINE_MIN)   nMine=MINE_MIN;
				if(nMine>MINE_MAX(nMapX,nMapY)) nMine=MINE_MAX(nMapX,nMapY);

				LoadDefaultMap(3);
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK HeroNameProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			if(nGameMode<0||nGameMode>=3) return (INT_PTR)FALSE;

			TCHAR szLoadingString[MAX_LOADSTRING];
			LoadString(hInst, IDS_HERO_NAME_TEXT1 + nGameMode, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT1,szLoadingString);
			LoadString(hInst, IDS_HERO_NAME, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_EDIT1,szLoadingString);
			LoadString(hInst, IDOK, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDOK,szLoadingString);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			if(LOWORD(wParam) == IDOK)
			{
				if(nGameMode<0||nGameMode>=3) return (INT_PTR)TRUE;
				TCHAR szLoadingString[MAX_LOADSTRING];
				GetDlgItemText(hDlg, IDC_EDIT1, szLoadingString, MAX_LOADSTRING);
				_tcscpy(szHeroes[nGameMode],szLoadingString);
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK HeroListProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TCHAR szLoadingString[MAX_LOADSTRING];
			LoadString(hInst, IDS_HERO_LIST, szLoadingString, MAX_LOADSTRING);
			SetWindowText(hDlg, szLoadingString); 
			LoadString(hInst, IDS_HERO_LIST_TEXT1, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT1,szLoadingString);
			LoadString(hInst, IDS_HERO_LIST_TEXT2, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT2,szLoadingString);
			LoadString(hInst, IDS_HERO_LIST_TEXT3, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT3,szLoadingString);
			LoadString(hInst, IDS_HERO_LIST_BUTTON, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_BUTTON1,szLoadingString);
			LoadString(hInst, IDOK, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDOK,szLoadingString);

			for(int i=0;i<3;i++)
			{
				SetDlgItemText(hDlg, IDC_EXTRA1+i, szHeroes[i]);
				_stprintf(szLoadingString, _TEXT("%d 秒"), nHeroes[i]);
				SetDlgItemText(hDlg, IDC_EDIT1+i, szLoadingString);

			}
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_BUTTON1)
		{
			InitHeroes();
			TCHAR szLoadingString[MAX_LOADSTRING];
			for(int i=0;i<3;i++)
			{
				SetDlgItemText(hDlg, IDC_EXTRA1+i, szHeroes[i]);
				_stprintf(szLoadingString, _TEXT("%d 秒"), nHeroes[i]);
				SetDlgItemText(hDlg, IDC_EDIT1+i, szLoadingString);

			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TCHAR szLoadingString[MAX_LOADSTRING];
			LoadString(hInst, IDS_ABOUTTEXT, szLoadingString, MAX_LOADSTRING);
			SetWindowText(hDlg, szLoadingString); 
			LoadString(hInst, IDS_ABOUTTEXT1, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT1,szLoadingString);
			LoadString(hInst, IDS_ABOUTTEXT2, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT2,szLoadingString);
			LoadString(hInst, IDS_ABOUTTEXT3, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDC_TEXT3,szLoadingString);
			LoadString(hInst, IDOK, szLoadingString, MAX_LOADSTRING);
			SetDlgItemText(hDlg,IDOK,szLoadingString);
			bCheatEnabled=false;
			//          LoadString(hInst, IDS_ABOUTTEXT4, szLoadingString, MAX_LOADSTRING);
			//          SetDlgItemText(hDlg,IDC_TEXT4,szLoadingString);
		}
		return (INT_PTR)TRUE;
	case WM_CTLCOLORSTATIC:
		if(bCheatEnabled&&(HWND)lParam==GetDlgItem(hDlg, IDC_TEXT3))
		{
			HDC hdc   =   (HDC)wParam;
			SetBkMode(hdc,   TRANSPARENT);
			SetTextColor(hdc,   RGB(255, 0, 0));
			return (INT_PTR)(COLOR_BTNFACE+1);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			int xPos = GET_X_LPARAM(lParam); 
			int yPos = GET_Y_LPARAM(lParam);
			POINT pt = {xPos, yPos};
			HWND hw = ChildWindowFromPoint(hDlg,pt);
			if(hw == GetDlgItem(hDlg, IDC_MYICON))
			{
				if(GetKeyState(VK_CONTROL)<0 && GetKeyState(VK_MENU)<0 && GetKeyState(VK_SHIFT)<0)
				{
					EasterEgg(hDlg);
				}
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}


