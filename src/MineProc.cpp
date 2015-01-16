#include "stdafx.h"
#include "resource.h"
#include "Game_def.h"

#define MAX_LOADSTRING 100

HWND hGame_MainWnd;
HINSTANCE hGame_Instance;
HBITMAP hGame_BlockBitmap=NULL;
HBITMAP hGame_ButtonBitmap=NULL;
HBITMAP hGame_DigitBitmap=NULL;

struct Block
{
	bool bMineExist;
	bool bBlockOpen;
	int  nBlockNumber;
	int  nBlockState;
} BlocksArray[BLOCK_YMAX][BLOCK_XMAX];

int nWndCX, nWndCY;
int nDigit1X, nDigit2X, nDigitY;
int nButtonX, nButtonY;

int nMapX, nMapY, nMine;
int nGameMode;

int nGameState;
int nStateMax;   //Mark Enable=2 Disable=1;
int nAdvancedState;

DWORD nReplay_LastEvent_Time;
int nReplay_LastEvent_Type;
WPARAM nReplay_LastEvent_WPARAM;
LPARAM nReplay_LastEvent_LPARAM;


POINT ptPressItem;
bool  bBothClicked;
bool  bNeedRelease;

UINT uTimerIndex;
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
int  nTimeCount;

int  nMineLeftCount;
int  nBlockLeftCount;

bool bCheatEnabled;

HANDLE hReplayFile;
VOID CALLBACK ReplayTimerProc(HWND, UINT, UINT_PTR, DWORD);
DWORD dwAdvancedTimeCount;
UINT uAdvancedTimerIndex;

bool bBusy = false;

#ifndef REPLAY_TIMER_USE_TICK
LARGE_INTEGER uPerformanceCount_Start, uPerformanceCount_Freq;
#else
DWORD dwTickCount_Start;

#endif

TCHAR cLastKey[5];
BOOL  bScreenShowEnabled=FALSE;

void InitGame(HINSTANCE hInst, HWND hWnd);
void LoadDefaultMap(int nMode);
void InitMap();
void DestroyGame();

void BeginGame();
void EndGame(int nEndType);

void Window_Paint(HWND hWnd);

bool ToggleMark();
void UpdateMenuInfo();

inline bool ValidateBlock(int X, int Y);
void BlastBlock(int X, int Y);
void OpenBlock(int X, int Y);
inline bool IsMine(int X, int Y);
inline int BlockDirection(int X, int Y);

inline POINT BlockFromPt(POINT pt);
inline POINT BlockFromPt(int X, int Y);
inline bool  BlockAbutting(POINT pt1, POINT pt2);
inline bool  BlockAbutting(int x1, int y1, int x2, int y2);

void LButton_Down(WPARAM wParam, LPARAM lParam);
void LButton_Up(WPARAM wParam, LPARAM lParam);
void RButton_Down(WPARAM wParam, LPARAM lParam);
void RButton_Up(WPARAM wParam, LPARAM lParam);
void Mouse_Move(WPARAM wParam, LPARAM lParam);

bool AdvancedRecord_RecordBegin();
void AdvancedRecord_AddEvent(int nType, WPARAM wParam, LPARAM lParam);
void AdvancedPlayback_UpdateEvent(bool bLoadEvent = false);
void AdvancedPlayback_ApplyEvent(int nType, WPARAM wParam, LPARAM lParam);
void AdvancedStop();
void StartAdvancedTimer();


extern void NewHero(int nTime);


////////////////////////////////////////////////////////////////////
void InitGame(HINSTANCE hInst, HWND hWnd)
{
	srand((unsigned)time(0));

	//TODO: Modify Everything Here!!!!
	bScreenShowEnabled=FALSE;
	hGame_MainWnd = hWnd;
	hGame_Instance = hInst;
	hGame_BlockBitmap = LoadBitmap(hGame_Instance,MAKEINTRESOURCE(IDB_BLOCKS));
	hGame_ButtonBitmap = LoadBitmap(hGame_Instance,MAKEINTRESOURCE(IDB_BUTTON));
	hGame_DigitBitmap = LoadBitmap(hGame_Instance,MAKEINTRESOURCE(IDB_DIGIT));
	ptPressItem.x=ptPressItem.y=-1;
	nStateMax=2;
	bBothClicked=false;
	bNeedRelease=false;
	nAdvancedState=0;
	hReplayFile = INVALID_HANDLE_VALUE;
	dwAdvancedTimeCount=0;
	LoadDefaultMap(0);
	UpdateMenuInfo();
}

void DestroyGame()
{
	if(nAdvancedState!=0)
	{
		AdvancedStop();
	}

	if(hGame_BlockBitmap)
	{
		DeleteObject(hGame_BlockBitmap);
	}
	if(hGame_ButtonBitmap)
	{
		DeleteObject(hGame_ButtonBitmap);
	}
	if(hGame_DigitBitmap)
	{
		DeleteObject(hGame_DigitBitmap);
	}
}

void LoadDefaultMap(int nMode)
{
	if(nAdvancedState!=0)
	{
		AdvancedStop();
	}
	switch(nMode)
	{
	case 0:
		nMapX = MODE_EASY_MAPX;
		nMapY = MODE_EASY_MAPY;
		nMine = MODE_EASY_MINE;
		nGameMode = 0;
		break;
	case 1:
		nMapX = MODE_NORMAL_MAPX;
		nMapY = MODE_NORMAL_MAPY;
		nMine = MODE_NORMAL_MINE;
		nGameMode = 1;
		break;
	case 2:
		nMapX = MODE_HARD_MAPX;
		nMapY = MODE_HARD_MAPY;
		nMine = MODE_HARD_MINE;
		nGameMode = 2;
		break;
	default:
		nGameMode = 3;
	}

	InitMap();
}

void UpdateBlockNumbers()
{
	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			int nMineNumber=0;
			if(IsMine(j-1,i-1)) nMineNumber++;
			if(IsMine(j,  i-1)) nMineNumber++;
			if(IsMine(j+1,i-1)) nMineNumber++;
			if(IsMine(j-2,i  )) nMineNumber++;
			if(IsMine(j-1,i  )) nMineNumber++;
			if(IsMine(j,  i  )) nMineNumber++;
			if(IsMine(j+1,i  )) nMineNumber++;
			if(IsMine(j+2,i  )) nMineNumber++;
			if(IsMine(j-1,i+1)) nMineNumber++;
			if(IsMine(j,  i+1)) nMineNumber++;
			if(IsMine(j+1,i+1)) nMineNumber++;

			if(IsMine(j-2,BlockDirection(j,i)==0 ? i-1 : i+1  )) nMineNumber++;
			if(IsMine(j+2,BlockDirection(j,i)==0 ? i-1 : i+1  )) nMineNumber++;

			BlocksArray[i][j].nBlockNumber = nMineNumber;
		}
	}     
}

void InitMap()
{
	if(nGameState ==1) KillTimer(hGame_MainWnd, uTimerIndex);
	if(nAdvancedState!=0)
	{
		AdvancedStop();
	}
	nGameState = 0;
	nWndCX = BLOCK_AREA_X + BLOCKSIZE_X + (nMapX-1) * BLOCKDELTA_X + BLOCK_AREA_EDGE_X;
	nWndCY = BLOCK_AREA_Y + BLOCKSIZE_Y + (nMapY-1) * BLOCKDELTA_Y + BLOCK_AREA_EDGE_Y;

	nDigit1X = DIGITEDGE_LEFT;
	nDigit2X = nWndCX - DIGITEDGE_RIGHT - DIGITWIDTH*DIGITCOUNT;
	nDigitY = DIGITEDGE_TOP;

	nButtonX = (nWndCX - BUTTONWIDTH)/2;
	nButtonY = BUTTONEDGE_TOP;

	RECT rect = {0, 0, nWndCX, nWndCY};
	AdjustWindowRect(&rect, WS_CAPTION|WS_VISIBLE|WS_CLIPSIBLINGS|WS_SYSMENU|WS_OVERLAPPED|WS_MINIMIZEBOX, TRUE);
	SetWindowPos(hGame_MainWnd, NULL, -1, -1, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);

	nTimeCount = 0;
	nMineLeftCount = nMine;
	nBlockLeftCount = nMapX*nMapY;

	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			BlocksArray[i][j].bMineExist = false; 
			BlocksArray[i][j].bBlockOpen = false;
			BlocksArray[i][j].nBlockNumber = -1;
			BlocksArray[i][j].nBlockState = 0;
		}
	}

	for(int i=0;i<nMine;i++)
	{
		int x, y;
		do
		{
			x = rand()%nMapX;
			y = rand()%nMapY;
		}
		while(BlocksArray[y][x].bMineExist);
		BlocksArray[y][x].bMineExist = true;
	}

	UpdateBlockNumbers();
	InvalidateRect(hGame_MainWnd, NULL, FALSE);
	UpdateMenuInfo();
}
///////////////////////////////////////////////////////////////////////////////
void BeginGame()
{
	uTimerIndex=(UINT)SetTimer(hGame_MainWnd, 2, 1000, TimerProc);
	nTimeCount = 1;
	nGameState = 1;
}

void EndGame(int nEndType)
{
	nGameState=2+nEndType;
	ptPressItem.x=ptPressItem.y=-1;
	bBothClicked = false;
	KillTimer(hGame_MainWnd, uTimerIndex);
	InvalidateRect(hGame_MainWnd, NULL, FALSE);
	if(nAdvancedState!=0)
	{
		AdvancedStop();
	}

	if(nEndType==1)
	{
		NewHero(nTimeCount);
	}
}
///////////////////////////////////////////////////////////////////////////////
// Menu Handlers

bool ToggleMark()
{
	nStateMax = 3 - nStateMax;
	UpdateMenuInfo();

	return nStateMax==2;
}

void UpdateMenuInfo()
{
	HMENU hMenu = GetMenu(hGame_MainWnd);
	if(hMenu)
	{
		CheckMenuItem(hMenu, IDM_FILE_GAME_EASY,    nGameMode==0 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_FILE_GAME_MEDIUM,  nGameMode==1 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_FILE_GAME_HARD,    nGameMode==2 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_FILE_GAME_CUSTOM,  nGameMode>=3 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_FILE_MARK,         nStateMax==2 ? MF_CHECKED : MF_UNCHECKED);
		///Advanced State
		EnableMenuItem(hMenu, IDM_FILE_MARK,        nAdvancedState==0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ADVANCED_LOADMAP,        nAdvancedState==0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ADVANCED_SAVEMAP,        nAdvancedState==0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ADVANCED_RESTART,        nAdvancedState==0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ADVANCED_RECORD_RECORD,  nAdvancedState==0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ADVANCED_RECORD_PLAY,    nAdvancedState==0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ADVANCED_RECORD_STOP,    nAdvancedState!=0 ? MF_ENABLED : MF_GRAYED);
	}
}

///////////////////////////////////////////////////////////////////////////////
inline bool ValidateBlock(int X, int Y)
{
	if(X<0||X>=nMapX||Y<0||Y>=nMapY) return false;
	return true;
}

void OpenBlock(int X, int Y)
{
	if(!ValidateBlock(X,Y)) return;
	if(BlocksArray[Y][X].bBlockOpen||BlocksArray[Y][X].nBlockState==1) return;
	BlocksArray[Y][X].bBlockOpen=true;
	if(BlocksArray[Y][X].bMineExist)
	{
		EndGame(0);
	}
	nBlockLeftCount--;
	if(!BlocksArray[Y][X].bMineExist&&BlocksArray[Y][X].nBlockNumber==0)
	{
		BlastBlock(X, Y);
	}

	if(nBlockLeftCount<=nMine&&nGameState==1)
	{
		for(int i=0;i<nMapY;i++)
		{
			for(int j=0;j<nMapX;j++)
			{
				if(BlocksArray[i][j].bBlockOpen == false && BlocksArray[i][j].nBlockState != 1)
				{
					BlocksArray[i][j].nBlockState = 1;
					nMineLeftCount--;
				}
			}
		}
		EndGame(1);
	}

}

void BlastBlock(int X, int Y)
{
	ptPressItem.x = ptPressItem.y = -1;
	bNeedRelease=true;
	if(!ValidateBlock(X,Y)) return;

	if(!BlocksArray[Y][X].bBlockOpen) return;
	//MessageBox(NULL, _T("Blast Happen"), NULL, 0 );

	int tmarkcount = 0;
	for(int i=Y-1;i<=Y+1;i++)
	{
		for(int j=X-2;j<=X+2;j++)
		{
			if(BlockAbutting(X, Y, j, i))
			{
				if(BlocksArray[i][j].nBlockState==1)
				{
					tmarkcount++;
				}
			}
		}
	}
	if(tmarkcount==BlocksArray[Y][X].nBlockNumber||BlocksArray[Y][X].nBlockNumber==0)
	{
		OpenBlock(X-1,Y-1);
		OpenBlock(X,  Y-1);
		OpenBlock(X+1,Y-1);
		OpenBlock(X-2,Y  );
		OpenBlock(X-1,Y  );
		OpenBlock(X,  Y  );
		OpenBlock(X+1,Y  );
		OpenBlock(X+2,Y  );
		OpenBlock(X-1,Y+1);
		OpenBlock(X,  Y+1);
		OpenBlock(X+1,Y+1);

		OpenBlock(X-2,BlockDirection(X,Y)==0 ? Y-1 : Y+1  );
		OpenBlock(X+2,BlockDirection(X,Y)==0 ? Y-1 : Y+1  );
	}
}


inline bool IsMine(int X, int Y)
{
	if(!ValidateBlock(X,Y)) return false;
	return BlocksArray[Y][X].bMineExist;
}

inline int BlockDirection(int X, int Y)
{
	return (X+Y)%2;
}

inline void SolidFill(HDC hdc, COLORREF crColor, RECT rect)
{
	SetBkMode(hdc, OPAQUE);
	COLORREF colorOld = SetBkColor(hdc, crColor);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	SetBkColor(hdc, colorOld);
}

inline int GetItemImage(int X, int Y)
{
	if(!ValidateBlock(X,Y)) return 0;

	if(bBothClicked && !BlocksArray[Y][X].bBlockOpen && BlocksArray[Y][X].nBlockState!=1 && BlockAbutting(ptPressItem.x, ptPressItem.y, X, Y))
	{
		return BlocksArray[Y][X].nBlockState==2 ? 6 : 19;
	}

	if(!BlocksArray[Y][X].bBlockOpen)      //Not Open
	{
		if(nGameState>=2)       //Game Is Over
		{
			if(BlocksArray[Y][X].bMineExist)
			{
				if(BlocksArray[Y][X].nBlockState!=1)
				{
					return 5;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				if(BlocksArray[Y][X].nBlockState==1)
					return 4;
			}
		}
		else if((BlocksArray[Y][X].nBlockState!=1) && X==ptPressItem.x &&Y==ptPressItem.y)
		{
			return BlocksArray[Y][X].nBlockState==2 ? 6 : 19;
		}
		return BlocksArray[Y][X].nBlockState;
	}

	if(BlocksArray[Y][X].bMineExist)       //Open But Mine;
	{
		return 3;
	}
	else
		return 19-BlocksArray[Y][X].nBlockNumber;

	return 0;
}

inline void DrawDigit(HDC hdc, HDC hMemoryDC, int digitX, int digitY, int Digit)
{
	int p=10,q=1;
	//    int k;
	int value = (Digit >= 0 ? Digit : -Digit);
	for(int i=0;i<DIGITCOUNT;i++)
	{
		if(i==DIGITCOUNT-1&&Digit<0)
		{
			BitBlt(hdc, digitX+(DIGITCOUNT-i-1)*DIGITWIDTH, digitY, DIGITWIDTH, DIGITHEIGHT, hMemoryDC, 0, 0,SRCCOPY);
			break;
		}
		BitBlt(hdc, digitX+(DIGITCOUNT-i-1)*DIGITWIDTH, digitY, DIGITWIDTH, DIGITHEIGHT, hMemoryDC, 0, DIGITHEIGHT*(11-(value%p)/q),SRCCOPY); 
		q=p;p*=10;
	}
}

inline void DrawButton(HDC hdc, HDC hMemoryDC, int buttonX, int buttonY)
{
	int t = -1;
	if(ptPressItem.x==-2&&ptPressItem.y==-2) t = 0;
	else if(nGameState >= 2) t = 4 - nGameState;
	else if(ptPressItem.x!=-1&&ptPressItem.y!=-1) t = 3;
	else t= 4;

	BitBlt(hdc, buttonX, buttonY, BUTTONWIDTH, BUTTONHEIGHT, hMemoryDC, 0, BUTTONHEIGHT*t,SRCCOPY); 
}

void Draw3DRect(HDC hdc, int x, int y, int cx, int cy, COLORREF color1, COLORREF color2)
{
	HPEN pen1 = CreatePen(PS_SOLID, 2, color1);
	HPEN pen2 = CreatePen(PS_SOLID, 2, color2);
	HPEN oldpen = (HPEN)SelectObject(hdc, (HGDIOBJ)pen1);
	MoveToEx(hdc, x+cx, y, NULL);
	LineTo(hdc, x, y);
	LineTo(hdc, x, y+cy);
	SelectObject(hdc, (HGDIOBJ)pen2);
	LineTo(hdc, x+cx,y+cy);
	LineTo(hdc, x+cx, y);
	SelectObject(hdc, (HGDIOBJ)oldpen);
	DeleteObject(pen1);
	DeleteObject(pen2);
}

void Window_Paint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(hWnd, &ps);

	HDC hMemoryDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOldObject = SelectObject(hMemoryDC, hGame_DigitBitmap);

	//TODO: Draw Frames
	Draw3DRect(hdc, 0, 0, nWndCX,
		nWndCY, RGB(255, 255, 255), RGB(128, 128, 128));

	Draw3DRect(hdc, BLOCK_AREA_X - FRAME_INNER_OFFSET_X, BUTTONEDGE_TOP - FRAME_INNER_OFFSET_Y, (nMapX-1) * BLOCKDELTA_X + BLOCKSIZE_X + FRAME_INNER_OFFSET_X * 2,
		BUTTONHEIGHT + FRAME_INNER_OFFSET_Y * 2, RGB(128, 128, 128), RGB(255, 255, 255));

	Draw3DRect(hdc, BLOCK_AREA_X - FRAME_INNER_OFFSET_X, BLOCK_AREA_Y - FRAME_INNER_OFFSET_Y, (nMapX-1) * BLOCKDELTA_X + BLOCKSIZE_X + FRAME_INNER_OFFSET_X * 2,
		(nMapY-1) * BLOCKDELTA_Y + BLOCKSIZE_Y + FRAME_INNER_OFFSET_Y * 2, RGB(128, 128, 128), RGB(255, 255, 255));
	
//	Draw3dRect(hdc, BLOCK_AREA_X, BLOCK_AREA_Y, (nMapX-1) * BLOCKDELTA_X + BLOCKSIZE_X, (nMapY-1) * BLOCKDELTA_Y + BLOCKSIZE_Y,
		//RGB(0, 0, 255), RGB(0, 0, 255));
	//TODO: Draw Timers & Buttons
	SelectObject(hMemoryDC, hGame_DigitBitmap);
	DrawDigit(hdc, hMemoryDC, nDigit1X, nDigitY, nMineLeftCount);
	DrawDigit(hdc, hMemoryDC, nDigit2X, nDigitY, nTimeCount);

	SelectObject(hMemoryDC, hGame_ButtonBitmap);
	DrawButton(hdc, hMemoryDC, nButtonX, nButtonY);

	//TODO: DrawMineSquares
	SelectObject(hMemoryDC, hGame_BlockBitmap);

	//	SolidFill(hdc, RGB(128, 128, 128), rect);
	RECT rect = {BLOCK_AREA_X, BLOCK_AREA_Y, BLOCK_AREA_X + (nMapX-1) * BLOCKDELTA_X + BLOCKSIZE_X, BLOCK_AREA_Y + (nMapY-1) * BLOCKDELTA_Y + BLOCKSIZE_Y};

	//DrawEdge(hdc, &rect, EDGE_ETCHED|EDGE_SUNKEN, BF_RECT);
//	DrawFrameControl(hdc, &rect, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_ADJUSTRECT);
	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			TransparentBlt(hdc, BLOCK_AREA_X + BLOCKDELTA_X*j, BLOCK_AREA_Y + BLOCKDELTA_Y*i, BLOCKSIZE_X, BLOCKSIZE_Y, 
				hMemoryDC, BlockDirection(j, i) ? 0 : BLOCKSIZE_X, GetItemImage(j, i) * BLOCKSIZE_Y, BLOCKSIZE_X, BLOCKSIZE_Y, TRANSPARENT_COLOR);

			TCHAR szPrintString[] = {(TCHAR)BlocksArray[i][j].nBlockNumber+'0' , '\0'};
			if(BlocksArray[i][j].bMineExist) szPrintString[0]='M';
		}
	}

	SelectObject(hMemoryDC, hOldObject);
	DeleteDC(hMemoryDC);


	EndPaint(hWnd, &ps);	
}

///////////////////////////////////////////////////////////////////////////////
inline bool  BlockAbutting(POINT pt1, POINT pt2)
{
	if(pt1.x<0||pt1.y<0||pt2.x<0||pt2.y<0) return false;
	if(pt2.x>=pt1.x-1 && pt2.x<=pt1.x+1 && pt2.y>=pt1.y-1 && pt2.y<=pt1.y+1)
	{
		return true;
	}

	if(pt2.y == (BlockDirection(pt1.x,pt1.y)==0 ? pt1.y-1 : pt1.y+1) || pt2.y == pt1.y)
	{
		if(pt2.x==pt1.x-2||pt2.x==pt1.x+2)
			return true;
	}
	return false;
}

inline bool  BlockAbutting(int x1, int y1, int x2, int y2)
{
	POINT pt1 = {x1, y1}, pt2 = {x2, y2};
	return BlockAbutting(pt1, pt2);
}

inline POINT BlockFromPt(POINT pt)
{
	POINT retValue = {-1, -1};
	if((pt.x>=nButtonX&&pt.y<(nButtonX+BUTTONWIDTH))&&(pt.y>=nButtonY&&pt.y<(nButtonY+BUTTONHEIGHT)))
	{
		retValue.x = retValue.y = -2;
		return retValue;
	}

	retValue.y = (pt.y-BLOCK_AREA_Y+BLOCKSIZE_Y)/BLOCKSIZE_Y-1;

	if(retValue.y<0||retValue.y>=nMapY)
	{
		retValue.x=-1;
		retValue.y=-1;
		return retValue;
	}

	int xmin=(pt.x-BLOCK_AREA_X-(BLOCKSIZE_X-1))/BLOCKDELTA_X;
	if(xmin<0) xmin=0;
	int xmax=(pt.x-BLOCK_AREA_X)/BLOCKDELTA_X;
	if(xmax>=nMapX) xmax=nMapX-1;

	HDC hdc;
	hdc = GetDC(hGame_MainWnd);
	HDC hMemoryDC = CreateCompatibleDC(hdc);
	SetBkColor(hMemoryDC, TRANSPARENT_COLOR);
	HGDIOBJ hOldObject = SelectObject(hMemoryDC, hGame_BlockBitmap);

	for(int i=xmin;i<=xmax;i++)
	{
		int t= pt.x-BLOCK_AREA_X-i*BLOCKDELTA_X;
		if(t<0||t>=BLOCKSIZE_X) continue;
		t+=(BlockDirection(i, retValue.y)==0 ? BLOCKSIZE_X : 0) ;


		COLORREF color = GetPixel(hMemoryDC, t, pt.y-BLOCK_AREA_Y-retValue.y*BLOCKDELTA_Y);
		if(color!=TRANSPARENT_COLOR&&color!=CLR_INVALID)
		{
			retValue.x = i;
			break;
		}
	}

	SelectObject(hMemoryDC, hOldObject);
	DeleteDC(hMemoryDC);
	DeleteDC(hdc);

	return retValue;
}

inline POINT BlockFromPt(int X, int Y)
{
	POINT pt = {X, Y};
	return BlockFromPt(pt);
}

void LButton_Down(WPARAM wParam, LPARAM lParam)
{
	if(nAdvancedState==1)
	{
		AdvancedRecord_AddEvent(1, wParam, lParam);
	}
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	POINT ptNew = BlockFromPt(xPos, yPos);

	SetCapture(hGame_MainWnd);
	if(ptNew.x==-2&&ptNew.y==-2)
	{
		ptPressItem = ptNew;
		InvalidateRect(hGame_MainWnd, NULL, FALSE);

		return;
	}
	else if(ptNew.x>=0&&ptNew.y>=0)
	{
		if(nGameState>=2) return;
		if(ptPressItem.x!=ptNew.x||ptPressItem.y!=ptNew.y)
		{
			ptPressItem = ptNew;
			InvalidateRect(hGame_MainWnd, NULL, FALSE);
		}
	}
	if(bBothClicked != (wParam&MK_LBUTTON && wParam&MK_RBUTTON))
	{
		bBothClicked = !bBothClicked;
		InvalidateRect(hGame_MainWnd, NULL, FALSE);
	}
}

void LButton_Up(WPARAM wParam, LPARAM lParam)
{
	if(nAdvancedState==1)
	{
		AdvancedRecord_AddEvent(2, wParam, lParam);
	}
	ReleaseCapture();

	if(bNeedRelease)
	{
		bNeedRelease=false;
		return;
	}
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	POINT pt = BlockFromPt(xPos, yPos);

	if(pt.x==-2&&pt.y==-2)
	{
		ptPressItem.x = ptPressItem.y = -1;

		InvalidateRect(hGame_MainWnd, NULL, FALSE);
		InitMap();
		return;
	}
	else if(pt.x>=0&&pt.y>=0)
	{
		if(nGameState>=2) return;
		if(nGameState==0) BeginGame();
		if(!BlocksArray[pt.y][pt.x].bBlockOpen && BlocksArray[pt.y][pt.x].nBlockState!=1 && !bBothClicked)
			OpenBlock(pt.x, pt.y);

		ptPressItem.x = ptPressItem.y = -1;

		InvalidateRect(hGame_MainWnd, NULL, FALSE);
	}

	if(bBothClicked != (wParam&MK_LBUTTON && wParam&MK_RBUTTON))
	{
		if(!(bBothClicked = !bBothClicked))
			BlastBlock(pt.x, pt.y);
		InvalidateRect(hGame_MainWnd, NULL, FALSE);
	}
}

void RButton_Down(WPARAM wParam, LPARAM lParam)
{
	if(nAdvancedState==1)
	{
		AdvancedRecord_AddEvent(3, wParam, lParam);
	}

	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	POINT pt = BlockFromPt(xPos, yPos);

	SetCapture(hGame_MainWnd);

	if(pt.x>=0&&pt.y>=0)
	{
		if(nGameState>=2) return;

		if(bBothClicked != (wParam&MK_LBUTTON && wParam&MK_RBUTTON))
		{
			bBothClicked = !bBothClicked;
			InvalidateRect(hGame_MainWnd, NULL, FALSE);
		}

		if(!bBothClicked &&!BlocksArray[pt.y][pt.x].bBlockOpen)
		{
			if(BlocksArray[pt.y][pt.x].nBlockState==1) nMineLeftCount++;

			BlocksArray[pt.y][pt.x].nBlockState++;
			if(BlocksArray[pt.y][pt.x].nBlockState>nStateMax) BlocksArray[pt.y][pt.x].nBlockState=0;

			if(BlocksArray[pt.y][pt.x].nBlockState==1) nMineLeftCount--;

			InvalidateRect(hGame_MainWnd, NULL, FALSE);
		}
	}

}

void RButton_Up(WPARAM wParam, LPARAM lParam)
{
	if(nAdvancedState==1)
	{
		AdvancedRecord_AddEvent(4, wParam, lParam);
	}

	ReleaseCapture();

	if(nGameState>=2) return;
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	POINT pt = BlockFromPt(xPos, yPos);

	if(bBothClicked != (wParam&MK_LBUTTON && wParam&MK_RBUTTON))
	{
		if(!(bBothClicked = !bBothClicked))
			BlastBlock(pt.x, pt.y);
		InvalidateRect(hGame_MainWnd, NULL, FALSE);
	}


}

void Mouse_Move(WPARAM wParam, LPARAM lParam)
{
	if(nAdvancedState==1)
	{
		AdvancedRecord_AddEvent(0, wParam, lParam);
	}

	if(bScreenShowEnabled)
	{
		HDC hdc = GetDC(NULL);
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam);
		POINT pt = BlockFromPt(xPos, yPos);
		SetPixel(hdc, 0, 0, pt.x>=0&&pt.y>=0&&BlocksArray[pt.y][pt.x].bMineExist ? RGB(0,0,0) : RGB(255,255,255));

		ReleaseDC(NULL, hdc);
	}

	if(wParam&MK_LBUTTON)
	{
		if(bNeedRelease) return;
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam);
		POINT ptNew = BlockFromPt(xPos, yPos);
		if(ptPressItem.x!=ptNew.x||ptPressItem.y!=ptNew.y)
		{
			ptPressItem = ptNew;
			InvalidateRect(hGame_MainWnd, NULL, FALSE);
		} 
	}
	else
	{
		bNeedRelease = false;
		if(!(wParam&MK_RBUTTON))
		{
			if(!ptPressItem.x)
			{
				POINT ptNew = {-1, -1};
				ptPressItem = ptNew;
				InvalidateRect(hGame_MainWnd, NULL, FALSE);
			}
		}
	}
}

VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if(nTimeCount<999) nTimeCount++;

	InvalidateRect(hGame_MainWnd, NULL, FALSE);
}
///////////////////////////////////////////////////////////////////////////////////
/// Cheat.

VOID Key_Down(WPARAM wParam, LPARAM lParam)
{
	if(wParam==VK_ESCAPE&&bBothClicked)
	{
		KillTimer(hGame_MainWnd, uTimerIndex);
	}
	if(wParam==VK_RETURN&&GetKeyState(VK_RSHIFT)<0)
	{
		if(cLastKey[0]=='X'&&cLastKey[1]=='Y'&&cLastKey[2]=='Z'&&cLastKey[3]=='Z'&&cLastKey[4]=='Y')
		{
			bScreenShowEnabled = TRUE;
		}
	}
	if(wParam>='A'&&wParam<='Z')
	{
		cLastKey[0]=cLastKey[1];
		cLastKey[1]=cLastKey[2];
		cLastKey[2]=cLastKey[3];
		cLastKey[3]=cLastKey[4];
		cLastKey[4]=(TCHAR)wParam;
	}
}

///////////////////////////////////////////////////////////////////////////////////
///
void RestartMap()
{
	if(nGameState ==1) KillTimer(hGame_MainWnd, uTimerIndex);
	nGameState = 0;
	nGameMode = 4;  //"Speical
	nTimeCount = 0;
	nMineLeftCount = nMine;
	nBlockLeftCount = nMapX*nMapY;

	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			BlocksArray[i][j].bBlockOpen = false;
			BlocksArray[i][j].nBlockState = 0;
		}
	}

	InvalidateRect(hGame_MainWnd, NULL, FALSE);
	UpdateMenuInfo();
}

bool WriteMap(HANDLE hFile)
{
	BYTE *buffer = new BYTE[nMapX * nMapY + 6];
	DWORD dwBufferSize = nMapX * nMapY + 6;
	DWORD dwBytesWritten = 0;
	buffer[0]='C';
	buffer[1]='r';
	buffer[2]='L';
	buffer[3]='F';
	buffer[4]=(BYTE)nMapX;
	buffer[5]=(BYTE)nMapY;
	int nWritePos=6;
	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			buffer[nWritePos++] = BlocksArray[i][j].bMineExist ? '1' : '0';
		}
	}
	WriteFile(hFile, buffer, dwBufferSize, &dwBytesWritten, NULL); 

	delete buffer;

	return dwBufferSize==dwBytesWritten;
}

bool ReadMap(HANDLE hFile)
{
	BYTE *buffer = new BYTE[BLOCK_XMAX * BLOCK_XMAX + 6];
	DWORD dwBufferSize = 0;
	ReadFile(hFile, buffer, 6, &dwBufferSize, NULL);
	if(buffer[0]!='C'||buffer[1]!='r'||buffer[2]!='L'||buffer[3]!='F')
	{
		return false;
	}
	nMapX = (int)buffer[4];
	nMapY = (int)buffer[5];

	if(nMapX<MAPX_MIN) nMapX=MAPX_MIN;
	if(nMapX>MAPX_MAX) nMapX=MAPX_MAX;
	if(nMapY<MAPY_MIN) nMapY=MAPY_MIN;
	if(nMapY>MAPY_MAX) nMapY=MAPY_MAX;
	nMine = 0;

	ReadFile(hFile, buffer, nMapX * nMapY, &dwBufferSize, NULL);

	int nReadPos=0;
	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			if(BlocksArray[i][j].bMineExist = buffer[nReadPos++]=='1') nMine++;
		}
	}

	if(nMine<MINE_MIN)
	{
		nMine=MINE_MIN;
		return false;
	}
	if(nMine>MINE_MAX(nMapX,nMapY))
	{
		nMine=MINE_MAX(nMapX,nMapY);
		return false;
	}

	delete buffer;

	nGameMode=4;
	UpdateBlockNumbers();
	return true;

}

bool SaveMap(LPCTSTR szFileName)
{
	HANDLE hFile;
	hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	bool bRetValue = WriteMap(hFile);
	CloseHandle(hFile);
	return bRetValue;

}
bool LoadMap(LPCTSTR szFileName)
{

	HANDLE hFile;
	hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return false;
	bool bRetValue = ReadMap(hFile);
	CloseHandle(hFile);    
	return bRetValue;
}
//////////////////////////////////////////////////////////////////////////////

bool AdvancedRecord(LPCTSTR szFileName)
{
	if(nAdvancedState!=0)  return FALSE;
	if(nGameState!=0) InitMap();
	hReplayFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(!AdvancedRecord_RecordBegin())
	{
		return false;
	}
	TCHAR szTemp[MAX_LOADSTRING],szCaption[MAX_LOADSTRING];
	LoadString(hGame_Instance, IDS_FILE_RECORD_START, szTemp, MAX_LOADSTRING);
	LoadString(hGame_Instance, IDS_APP_TITLE, szCaption, MAX_LOADSTRING);
	MessageBox(hGame_MainWnd, szTemp, szCaption, 0); 
	StartAdvancedTimer();

	return true;
}

bool AdvancedPlayback(LPCTSTR szFileName)
{
	if(nAdvancedState!=0)  return FALSE;

	hReplayFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(!ReadMap(hReplayFile))
	{
		return false;
	}

	DWORD dwBytesWritten = 0;
	char chMarkChar;
	ReadFile(hReplayFile, &chMarkChar, sizeof(char), &dwBytesWritten, NULL);
	if(chMarkChar=='1')
		nStateMax=2;
	else
		nStateMax=1;

	nAdvancedState=2;
	RestartMap();

	TCHAR szTemp[MAX_LOADSTRING];
	LoadString(hGame_Instance, IDS_APP_TITLE_RECORD, szTemp, MAX_LOADSTRING);
	SetWindowText(hGame_MainWnd, szTemp);

	UpdateMenuInfo();
	StartAdvancedTimer();

	AdvancedPlayback_UpdateEvent(true);
	return true;
}

void AdvancedStop()
{
	if(nAdvancedState==0)  return;
	KillTimer(hGame_MainWnd, uAdvancedTimerIndex);
	if(nAdvancedState==1)
	{
		AdvancedRecord_AddEvent(-1, nTimeCount, 0);
	}
	ClipCursor(NULL);

	if(nAdvancedState==1)
	{
		TCHAR szTemp[MAX_LOADSTRING], szCaption[MAX_LOADSTRING];
		LoadString(hGame_Instance, IDS_FILE_RECORD_FINISH, szTemp, MAX_LOADSTRING);
		MessageBox(hGame_MainWnd, szTemp, szTemp, 0);
		LoadString(hGame_Instance, IDS_APP_TITLE, szCaption, MAX_LOADSTRING);
		SetWindowText(hGame_MainWnd, szCaption);

		LoadString(hGame_Instance, IDS_FILE_RECORD_RESTART, szTemp, MAX_LOADSTRING);
		if(nGameState==2)
		{
			if(MessageBox(hGame_MainWnd, szTemp, szCaption, MB_YESNO)==IDYES)
			{
				nAdvancedState=0;
				if(nGameMode!=4)
				{
					LoadDefaultMap(nGameMode);
				}
				else
				{
					RestartMap();
				}
				if(SetFilePointer(hReplayFile, 0L, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
				{
					if(SetEndOfFile(hReplayFile))
					{
						if(AdvancedRecord_RecordBegin())
						{
							StartAdvancedTimer();
							return;
						}
					}
				}
			}
		}
	}
	else if(nAdvancedState==2)
	{
		TCHAR szTemp[MAX_LOADSTRING];
		LoadString(hGame_Instance, IDS_FILE_PLAYBACK_FINISH, szTemp, MAX_LOADSTRING);
		MessageBox(hGame_MainWnd, szTemp, szTemp, 0);
		LoadString(hGame_Instance, IDS_APP_TITLE, szTemp, MAX_LOADSTRING);
		SetWindowText(hGame_MainWnd, szTemp);
	}

	if(hReplayFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hReplayFile);
		hReplayFile=INVALID_HANDLE_VALUE;
	}

	nAdvancedState=0;
	UpdateMenuInfo();
}

bool AdvancedRecord_RecordBegin()
{
	if(!WriteMap(hReplayFile))
	{
		return false;
	}
	DWORD dwBytesWritten = 0;
	char chMarkChar;
	if(nStateMax==2)
		chMarkChar='1';
	else
		chMarkChar='0';

	WriteFile(hReplayFile, &chMarkChar, sizeof(char), &dwBytesWritten, NULL);

	nReplay_LastEvent_Type=-1;
	nReplay_LastEvent_WPARAM=0;
	nReplay_LastEvent_LPARAM=0;
	nAdvancedState=1;

	TCHAR szTemp[MAX_LOADSTRING];
	LoadString(hGame_Instance, IDS_APP_TITLE_RECORD, szTemp, MAX_LOADSTRING);
	SetWindowText(hGame_MainWnd, szTemp);

	UpdateMenuInfo();
	InvalidateRect(hGame_MainWnd, NULL, false);
	return true;
}

void AdvancedPlayback_UpdateEvent(bool bLoadEvent)
{
	if(!bLoadEvent)
	{
		if(dwAdvancedTimeCount>=nReplay_LastEvent_Time)
		{
			AdvancedPlayback_ApplyEvent(nReplay_LastEvent_Type,nReplay_LastEvent_WPARAM,nReplay_LastEvent_LPARAM);
			bLoadEvent=true;
		}
		else return;
	}
	if(bLoadEvent)
	{
		DWORD dwBytesWritten = 0;

		ReadFile(hReplayFile, &nReplay_LastEvent_Time, sizeof(nReplay_LastEvent_Time), &dwBytesWritten, NULL); 
		if(dwBytesWritten!=sizeof(nReplay_LastEvent_Time)){AdvancedStop(); return;}
		ReadFile(hReplayFile, &nReplay_LastEvent_Type, sizeof(nReplay_LastEvent_Type), &dwBytesWritten, NULL); 
		if(dwBytesWritten!=sizeof(nReplay_LastEvent_Type)){AdvancedStop(); return;}
		ReadFile(hReplayFile, &nReplay_LastEvent_WPARAM, sizeof(nReplay_LastEvent_WPARAM), &dwBytesWritten, NULL); 
		if(dwBytesWritten!=sizeof(nReplay_LastEvent_WPARAM)){AdvancedStop(); return;}
		ReadFile(hReplayFile, &nReplay_LastEvent_LPARAM, sizeof(nReplay_LastEvent_LPARAM), &dwBytesWritten, NULL); 
		if(dwBytesWritten!=sizeof(nReplay_LastEvent_LPARAM)){AdvancedStop(); return;}
		AdvancedPlayback_UpdateEvent();
	}
	//if()
}

void AdvancedPlayback_ApplyEvent(int nType, WPARAM wParam, LPARAM lParam)
{
	if(nType>=0&&nType<=4)
	{
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam);
		POINT pt={xPos,yPos};
		ClientToScreen(hGame_MainWnd, &pt);

		/*    TCHAR szTemp[MAX_LOADSTRING];
		_stprintf(szTemp, "X:%d Y:%d", xPos, yPos);
		MessageBox(hGame_MainWnd, szTemp, NULL, 0);*/

		SetCursorPos(pt.x,pt.y);
		RECT rect = {pt.x, pt.y, pt.x, pt.y};
		ClipCursor(&rect);

		//    MessageBox(hGame_MainWnd, "Move Complete", NULL, 0);
		switch(nType)
		{
		case 0:
			Mouse_Move(wParam, lParam);
			break;
		case 1:
			LButton_Down(wParam, lParam);
			break;
		case 2:
			LButton_Up(wParam, lParam);
			break;
		case 3:
			RButton_Down(wParam, lParam);
			break;
		case 4:
			RButton_Up(wParam, lParam);
			break;
		}
	}
	if(nType==-1)
	{
		if(wParam!=0) nTimeCount=(int)wParam;
		AdvancedStop();
	}
}

void AdvancedRecord_AddEvent(int nType, WPARAM wParam, LPARAM lParam)
{
	if(hReplayFile==INVALID_HANDLE_VALUE) return;

//	if(nGameState==0) return;

	if(nType==0 && nReplay_LastEvent_Type==nType && nReplay_LastEvent_WPARAM==wParam && nReplay_LastEvent_LPARAM==lParam) return;

	nReplay_LastEvent_Type = nType;
	nReplay_LastEvent_WPARAM = wParam;
	nReplay_LastEvent_LPARAM = lParam;
	DWORD dwBytesWritten = 0;

	WriteFile(hReplayFile, &dwAdvancedTimeCount, sizeof(dwAdvancedTimeCount), &dwBytesWritten, NULL); 
	WriteFile(hReplayFile, &nType, sizeof(nType), &dwBytesWritten, NULL); 
	WriteFile(hReplayFile, &wParam, sizeof(wParam), &dwBytesWritten, NULL); 
	WriteFile(hReplayFile, &lParam, sizeof(lParam), &dwBytesWritten, NULL); 

}

void StartAdvancedTimer()
{
#ifndef REPLAY_TIMER_USE_TICK
	QueryPerformanceCounter(&uPerformanceCount_Start);
	QueryPerformanceFrequency(&uPerformanceCount_Freq);
#else
	dwTickCount_Start = GetTickCount();
#endif
	uAdvancedTimerIndex=(UINT)SetTimer(hGame_MainWnd, 3, 10, ReplayTimerProc);
	dwAdvancedTimeCount = 1;

}


VOID CALLBACK ReplayTimerProc(HWND hWnd, UINT nIndex, UINT_PTR, DWORD)
{
	if(bBusy) return;

	bBusy=true;
#ifndef REPLAY_TIMER_USE_TICK
	LARGE_INTEGER uCurCount;
	//    MessageBox(NULL, "Hello", NULL, 0);
	QueryPerformanceCounter(&uCurCount);
	uCurCount.QuadPart=(uCurCount.QuadPart - uPerformanceCount_Start.QuadPart)/(uPerformanceCount_Freq.QuadPart/100); //1000/20=50


	if(uCurCount.QuadPart<ADVANCED_TIME_MAX)
		dwAdvancedTimeCount = uCurCount.LowPart;
#else
	DWORD dwCurCount = GetTickCount()-dwTickCount_Start; //This is not usable if the system has already been running for 49.7 days.
	if(dwCurCount<0) dwCurCount=ADVANCED_TIME_MAX;
	dwAdvancedTimeCount = dwCurCount/10;


#endif
	else
	{
		KillTimer(hWnd, nIndex);
	}

	if(nAdvancedState==2)
	{
		AdvancedPlayback_UpdateEvent();
	}
	bBusy=false;
}

//////////////////////////////////////////////////////////////////////////////
void EasterEgg(HWND hDlg)
{
	//   if(n)
	bCheatEnabled=true;
	POINT eggs[]={{5, 0}, {4, 1}, {5, 1}, {6, 1},
	{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, /*{5, 2}, */{6, 2}, {7, 2}, {8, 2}, {9, 2}, {10, 2},
	{1, 3}, {2, 3}, {8, 3}, {9, 3},
	{1, 4}, {2, 4}, {8, 4}, {9, 4},
	{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, /*{5, 5}, */{6, 5}, {7, 5}, {8, 5}, {9, 5}, {10, 5},
	{4, 6}, {5, 6}, {6, 6}, {5, 7}};
	int eggcount=36;
	bool bAvailable=true;
	for(int i=0;i<nMapY;i++)
	{
		for(int j=0;j<nMapX;j++)
		{
			if(BlocksArray[i][j].nBlockState==1)
			{
				bool bFound=false;
				for(int k=0;k<eggcount;k++)
				{
					if(i==eggs[k].y&&j==eggs[k].x)
					{
						bFound=true;
						break;
					}
				}
				if(!bFound)
				{
					bAvailable=false;
					break;
				}
			}
		}
	}

	if(!bAvailable)
	{
		SetDlgItemText(hDlg, IDC_TEXT3, _T("Something is wrong, and it's your mistake!!"));
	}
	else
	{
		if(nMine==nMineLeftCount)
		{
			SetDlgItemText(hDlg, IDC_TEXT3, _T("Add some flags, and you'll find something!!"));
		}
		else if(nMine-nMineLeftCount==36)
		{
			SetDlgItemText(hDlg, IDC_TEXT3, _T("Yes, you've got it. Look at the flags, have you found something?"));
		}
		else
		{
			SetDlgItemText(hDlg, IDC_TEXT3, _T("Continue, you'll work everything out!!"));
		}
	}


	InvalidateRect(hDlg, NULL, FALSE);
	return;
}


