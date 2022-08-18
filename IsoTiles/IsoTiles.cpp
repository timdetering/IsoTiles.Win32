// IsoTiles.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "IsoTiles.h"
#include <stdio.h>
#include <math.h>

struct ARGB 
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
};

#define MAX_LOADSTRING 100

// Global Variables:
HWND hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

const int sideLength = 14;
const int xCells = 52;
const int yCells = 52;
const int cellDiagonal = 20;
unsigned char grid[yCells][xCells];

int xOrg=((xCells+1)/2);
int yOrg=((yCells+1)/2);
int xOrgX;
int yOrgY;

int dxX;
int dyX;
int dxY;
int dyY;

struct Object
{
    unsigned int m_nWidth, m_nHeight;
    unsigned int m_nAnchorCol, m_nAnchorRow;
    unsigned int m_nNumCols, m_nNumRows;
    unsigned int m_nOffsetX, m_nOffsetY;
    char m_szBmpFileName[32];
    HBITMAP m_hBmp;

    Object():m_nWidth(80), m_nHeight(40), 
        m_nAnchorCol(0), m_nAnchorRow(0), 
        m_nNumCols(1), m_nNumRows(1), 
        m_nOffsetX(0), m_nOffsetY(0){}
};

HBITMAP LoadTileImage(wchar_t *szFileName);

void DrawObject(HDC hdc, int x, int y, Object *pObject);
void DrawTile(HDC hdc, int x, int y, HBITMAP hTileBmp);
void DrawCell(HDC hdc, int x, int y, int index);
void DrawOrg(HDC hdc);

void DrawGrid(HWND hWnd, HDC hdc);
void DrawObjectsBackToFront(HDC hMemDC);

void PickTile(int x, int y, int &tx, int &ty);

HDC hTileDC;
BLENDFUNCTION bfn = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
HBITMAP hTileImage[256];
Object rgObject[10];
int tileX, tileY;
int tx, ty;

int curTileIndex = 0;

void LoadMap(wchar_t *szFileName);
void SaveMap(wchar_t *szFileName);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_ISOTILES, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    rgObject[0].m_hBmp = hTileImage[0] = LoadTileImage(L"grass.bmp");
    rgObject[1].m_hBmp = hTileImage[1] = LoadTileImage(L"stoneFenceRight.bmp");
    rgObject[1].m_nOffsetY = 46;
    rgObject[1].m_nHeight = 86;
    rgObject[1].m_nWidth = 80;
    rgObject[2].m_hBmp = hTileImage[2] = LoadTileImage(L"stoneFenceFront.bmp");
    rgObject[2].m_nOffsetX = 13;
    rgObject[2].m_nOffsetY = 43;
    rgObject[2].m_nHeight = 83;
    rgObject[2].m_nWidth = 93;
    rgObject[3].m_hBmp = hTileImage[3] = LoadTileImage(L"house.bmp");
    rgObject[3].m_nOffsetY = 134;
    rgObject[3].m_nHeight = 235;
    rgObject[3].m_nWidth = 279;
    rgObject[4].m_hBmp = hTileImage[4] = LoadTileImage(L"tree1.bmp");
    rgObject[4].m_nOffsetX = 42;
    rgObject[4].m_nOffsetY = 318;
    rgObject[4].m_nHeight = 358;
    rgObject[4].m_nWidth = 189;
    rgObject[5].m_hBmp = hTileImage[5] = LoadTileImage(L"mudgrass4.bmp");
    rgObject[6].m_hBmp = hTileImage[6] = LoadTileImage(L"mudgrass5.bmp");
    rgObject[7].m_hBmp = hTileImage[7] = LoadTileImage(L"mudgrass6.bmp");
    rgObject[8].m_hBmp = hTileImage[8] = LoadTileImage(L"mudgrass7.bmp");
    rgObject[9].m_hBmp = hTileImage[9] = LoadTileImage(L"mudgrass8.bmp");

    BITMAP bm;
    GetObject(hTileImage[0], sizeof(bm), &bm);
    tileX = bm.bmWidth;
    tileY = bm.bmHeight;
    HDC hdc = GetDC(NULL);
    hTileDC = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ISOTILES));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
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
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
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
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ISOTILES));
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ISOTILES);
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

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
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONUP:
        PickTile(LOWORD(lParam), HIWORD(lParam), tx, ty);
        grid[ty][tx] = curTileIndex;
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_KEYDOWN:
        switch(wParam)
        {
        case VK_UP:
            if(xOrg > 0)
            {
                --xOrg;
            }
            if(yOrg < (yCells-1))
            {
                ++yOrg;
            }
            break;
        case VK_DOWN:
            if(xOrg < (xCells-1))
            {
                ++xOrg;
            }
            if(yOrg > 0)
            {
                --yOrg;
            }
            break;

        case VK_RIGHT:
            if(xOrg < (xCells-1))
            {
                ++xOrg;
            }
            if(yOrg< (yCells-1))
            {
                ++yOrg;
            }
            break;
        case VK_LEFT:
            if(xOrg > 0)
            {
                --xOrg;
            }
            if(yOrg > 0)
            {
                --yOrg;
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            curTileIndex = wParam - '0';
            break;
        }
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        DrawGrid(hWnd, hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
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

void DrawGrid(HWND hWnd, HDC hdc)
{
    dxX = 40;
    dyX = 20;
    dxY = 40;
    dyY = -20;
    RECT rect;
    GetClientRect(hWnd, &rect);
    xOrgX = (rect.right - rect.left)/2 - (dxX + dxY)/2;
    yOrgY = (rect.bottom - rect.top)/2 /*+ (dxY + dyY)/2*/;

    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    HBITMAP hBmpOld = (HBITMAP)SelectObject(hMemDC, hBmp);

    SelectObject(hMemDC, GetStockObject(NULL_PEN));
    Rectangle(hMemDC, 0, 0, rect.right+1, rect.bottom+1);

    SelectObject(hMemDC, GetStockObject(BLACK_PEN));

    for(int y=0; y<yCells; y++)
    {
        for(int x=0; x<xCells; x++)
        {
            //DrawTile(hMemDC, x, y, hTileImage[grid[y][x]]);
            DrawObject(hMemDC, x, y, rgObject);
        }
    }
    DrawObjectsBackToFront(hMemDC);
    //for(int y=0; y<yCells; y++)
    //{
    //    for(int x=0; x<xCells; x++)
    //    {
    //        if(grid[y][x] != 0)
    //        {
    //            DrawObject(hMemDC, x, y, rgObject + grid[y][x]);
    //        }
    //    }
    //}

    //for(int y=0; y<yCells; y++)
    //{
    //    for(int x=0; x<xCells; x++)
    //    {
    //        DrawCell(hMemDC, x, y, 0);
    //    }
    //}

    DrawOrg(hMemDC);
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hBmpOld);
    DeleteObject(hBmp);
    DeleteObject(hMemDC);
}

void DrawObjectsBackToFront(HDC hMemDC)
{
    int startX = 0; 
    int startY = yCells;
    while(startY-- > 0)
    {
        int y = startY;
        int x = 0;
        while((x < xCells) && (y < yCells))
        {
            if(grid[y][x] != 0)
            {
                DrawObject(hMemDC, x, y, rgObject+ grid[y][x]);
            }
            ++x;
            ++y;
        }
    }
    while(++startX < xCells)
    {
        int y = 0;
        int x = startX;
        while((x < xCells) && (y < yCells))
        {
            if(grid[y][x] != 0)
            {
                DrawObject(hMemDC, x, y, rgObject+ grid[y][x]);
            }
            ++x;
            ++y;
        }
    }
}

void DrawCell(HDC hdc, int x, int y, int index)
{
    POINT p1, p2, p3, p4;

    int dx = x - xOrg;
    int dy = y - yOrg;

    int xContribDX = dx * dxX;
    int yContribDX = dx * dyX;
    int xContribDY = dy * dxY;
    int yContribDY = dy * dyY;
    int xContribDX1 = xContribDX + dxX;
    int yContribDX1 = yContribDX + dyX;
    int xContribDY1 = xContribDY + dxY;
    int yContribDY1 = yContribDY + dyY;

    p1.x = xOrgX + xContribDX + xContribDY;
    p1.y = yOrgY + yContribDX + yContribDY; 

    p2.x = xOrgX + xContribDX + xContribDY1;
    p2.y = yOrgY + yContribDX + yContribDY1; 

    p3.x = xOrgX + xContribDX1 + xContribDY1;
    p3.y = yOrgY + yContribDX1 + yContribDY1; 

    p4.x = xOrgX + xContribDX1 + xContribDY;
    p4.y = yOrgY + yContribDX1 + yContribDY; 

    MoveToEx(hdc, p1.x, p1.y, NULL);
    LineTo(hdc, p2.x, p2.y);
    LineTo(hdc, p3.x, p3.y);
    LineTo(hdc, p4.x, p4.y);
    LineTo(hdc, p1.x, p1.y);
}

void DrawObject(HDC hdc, int x, int y, Object *pObject)
{
    //Find the left top
    int dx = (x - pObject->m_nAnchorCol - xOrg);
    int dy = (y - pObject->m_nAnchorRow - yOrg);
    int X = xOrgX + dx*dxX + dy*dxY - pObject->m_nOffsetX;
    int Y = yOrgY + dx*dyX + dy*dyY - tileY/2 - pObject->m_nOffsetY; 
    HBITMAP hBmpOld = (HBITMAP)SelectObject(hTileDC, pObject->m_hBmp);
    AlphaBlend(hdc, X, Y, pObject->m_nWidth, pObject->m_nHeight, hTileDC, 0, 0, pObject->m_nWidth, pObject->m_nHeight, bfn);
    SelectObject(hTileDC, hBmpOld);
}

void DrawTile(HDC hdc, int x, int y, HBITMAP hTileBmp)
{
    int dx = (x - xOrg);
    int dy = (y - yOrg);
    int X = xOrgX + dx*dxX + dy*dxY;
    int Y = yOrgY + dx*dyX + dy*dyY - tileY/2; 
    HBITMAP hBmpOld = (HBITMAP)SelectObject(hTileDC, hTileBmp);
    AlphaBlend(hdc, X, Y, tileX, tileY, hTileDC, 0, 0, tileX, tileY, bfn);
    SelectObject(hTileDC, hBmpOld);
}

void DrawOrg(HDC hdc)
{
    int delta=10;
    int x = xOrgX + (dxX + dxY)/2;
    int y = yOrgY /*- (dyX + dyY)/2*/;
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
    HBRUSH hBrOld = (HBRUSH) SelectObject(hdc, hBrush);
    Ellipse(hdc, x-delta, y-delta/2, x+delta, y+delta/2);
    SelectObject(hdc, hBrOld);
}

HBITMAP LoadTileImage(wchar_t *szFileName)
{
    HBITMAP hTileBmp = NULL;
    HBITMAP hBmp = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);
    BITMAPINFO bi;
    memset(&bi, 0, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFO);
    HDC hdc = GetDC(NULL);
    if(GetDIBits(hdc, hBmp, 0, 0, NULL, &bi, DIB_RGB_COLORS))
    {
        unsigned char *pBits = NULL;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biHeight = abs(bi.bmiHeader.biHeight);
        bi.bmiHeader.biSizeImage = 0;
        hTileBmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (LPVOID *)&pBits, NULL, 0);

        if(GetDIBits(hdc, hBmp, 0, abs(bi.bmiHeader.biHeight), pBits, &bi, DIB_RGB_COLORS))
        {
            ARGB *pColor = (ARGB *)pBits;
            int index = 0;
            for(int i=0; i<abs(bi.bmiHeader.biHeight); i++)
            {
                for(int j=0; j<bi.bmiHeader.biWidth; j++)
                {
                    if((pColor[index].r != 0)|| (pColor[index].g != 0) || (pColor[index].b != 0))
                    {
                        pColor[index].a = 255;
                    }
                    else
                    {
                        pColor[index].a = 0;
                    }
                    ++index;
                }
            }
        }
    }
    ReleaseDC(NULL, hdc);
    DeleteObject(hBmp);
    return hTileBmp;
}

void PickTile(int x, int y, int &tx, int &ty)
{
    float pXRayX, pXRayY;
    float pYRayX, pYRayY;

    float magXRay, magYRay;

    magXRay = sqrtf((float)dxX*dxX + (float)dyX*dyX);
    magYRay = sqrtf((float)dxY*dxY + (float)dyY*dyY);

    pXRayX = dxX/magXRay;
    pXRayY = dyX/magXRay;

    pYRayX = dxY/magYRay;
    pYRayY = dyY/magYRay;

    float x0 = (float)(xOrgX - xOrg*dxX - yOrg*dxY);
    float y0 = (float)(yOrgY - xOrg*dyX - yOrg*dyY);

    float t1, t2;
    
    float num = y - y0 -(pXRayY*(x-x0)/pXRayX);
    float denom = pYRayY - (pXRayY*pYRayX)/pXRayX;
    t2 = num/denom;

    t1 = (x- x0 - t2*pYRayX)/pXRayX;

    tx = (int)(t1/magXRay);
    ty = (int)(t2/magYRay);
}

void LoadMap(wchar_t *szFileName)
{
    FILE *fp = _wfopen(szFileName, L"w");
    fprintf(fp, "Map %s\n", m_szName);

    fprintf(fp, "mapsize %d %d \n", m_nXCells, m_nYCells);
    fprintf(fp, "cellsize %d %d \n", m_nCellWidth, m_nCellHeight);

    fprintf(fp, "objects %d \n", m_nNumObjects);

    for(int i=0; i<m_nNumObjects; i++)
    {
        fprintf(fp, "object \n");
        fprintf(fp, "bitmap %s \n", m_rgObjects[i].m_szBmpFileName);
        fprintf(fp, "objectsize %d %d\n", m_rgObjects[i].m_nNumCols, m_rgObjects[i].m_nNumRows);
        fprintf(fp, "anchor %d %d\n", m_rgObjects[i].m_nAnchorRow, m_rgObjects[i].m_nAnchorCol);
        fprintf(fp, "offset %d %d\n", m_rgObjects[i].m_nOffsetX, m_rgObjects[i].m_nOffsetY);
        fprintf(fp, "endobject\n");
    }

    fclose(fp);
}

/*
Coding guidelines

Hungarian notation
bool        b
float       f
int         n
long        l
short       s
char        c
unsigned    u
string      sz
pointer     p
array       rg

identifiers - camelCasing
function names - PascalCasing
member variable prefix - m_
class name prefix C
class name casing CPascalCasing
struct names ALLCAPS

Compound statements: Braces on the next line as shown below
for()
{
}

Always use braces for if, even if there is just a single statement following it
if()
{
}

Do not use goto's

Functions should have one exit point

Use descriptive variable names.
User descriptive function names.

Function names should use the convention VerbObject (for non class/struct members) and
Object.Verb for class/struct member functions.

*/

typedef unsigned short Cell;
class Map
{
private:
    char m_szName[32];
    unsigned int m_nXCells;
    unsigned int m_nYCells;
    unsigned char m_nCellWidth;
    unsigned char m_nCellHeight;

    //Object table
    unsigned short m_nNumObjects;
    Object *m_rgObjects;

    //Map Cells
    Cell *m_rgCells;

public:
    void Save(wchar_t *szFileName);
};

struct View
{
    unsigned int xOrg;
    unsigned int yOrg;
    unsigned int xOrgx;
    unsigned int yOrgy;
};

struct HUD
{
};

void Map::Save(wchar_t *szFileName)
{
    FILE *fp = _wfopen(szFileName, L"w");
    fprintf(fp, "Map %s\n", m_szName);

    fprintf(fp, "mapsize %d %d \n", m_nXCells, m_nYCells);
    fprintf(fp, "cellsize %d %d \n", m_nCellWidth, m_nCellHeight);

    fprintf(fp, "objects %d \n", m_nNumObjects);

    for(int i=0; i<m_nNumObjects; i++)
    {
        fprintf(fp, "object \n");
        fprintf(fp, "bitmap %s \n", m_rgObjects[i].m_szBmpFileName);
        fprintf(fp, "objectsize %d %d\n", m_rgObjects[i].m_nNumCols, m_rgObjects[i].m_nNumRows);
        fprintf(fp, "anchor %d %d\n", m_rgObjects[i].m_nAnchorRow, m_rgObjects[i].m_nAnchorCol);
        fprintf(fp, "offset %d %d\n", m_rgObjects[i].m_nOffsetX, m_rgObjects[i].m_nOffsetY);
        fprintf(fp, "endobject\n");
    }

    fclose(fp);
}