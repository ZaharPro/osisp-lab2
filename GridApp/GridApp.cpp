#include "framework.h"
#include "GridApp.h"
#include "Windows.h"
#include <commdlg.h>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>

#define MAX_LOADSTRING 100
#define WM_BUILD_TABLE (WM_USER + 0x0001)

typedef std::vector<std::string> STRINGVECTOR;

typedef struct _TABLE {
	int rows;
	int cols;
	STRINGVECTOR strings;
} TABLE;

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR szFileName[MAX_LOADSTRING];


HINSTANCE hInst;
TABLE table;

ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL				TryGetNumFromEditCtrl(HWND, int, INT*);
VOID				GetUserFileName(HWND, WCHAR*);
VOID				LoadTextFromFile(WCHAR*, STRINGVECTOR*);
VOID				DrawTable(HWND, RECT, HDC);
INT					TryToPlace(std::string, HWND, HDC, RECT, int);
VOID				DrawLine(HDC, COLORREF, int, int, int, int);
VOID				DrawVerticalTableLines(HDC, COLORREF, INT, INT);
VOID				Repaint(HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GRIDAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRIDAPP));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRIDAPP));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GRIDAPP);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_BUILD_TABLE:
	{
		table.strings.clear();
		LoadTextFromFile(szFileName, &table.strings);
		Repaint(hWnd);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_OPENFILE:
		{
			GetUserFileName(hWnd, szFileName);

			if (szFileName[0] != '\0')
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Edit);
			}
		}
		break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		if (table.cols && table.rows) {
			RECT wndRect;
			GetClientRect(hWnd, &wndRect);

			DrawTable(hWnd, wndRect, hdc);
		}
		EndPaint(hWnd, &ps);

	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL TryGetNumFromEditCtrl(HWND hDlg, int nIDDlgItem, INT* dest) {
	WORD cchNums = (WORD)SendDlgItemMessage(hDlg, nIDDlgItem, EM_LINELENGTH, (WPARAM)0, (LPARAM)0);

	if (cchNums >= 2)
	{
		MessageBox(hDlg, L"Number very long.", L"Error", MB_OK);
		return FALSE;
	}
	else if (cchNums == 0)
	{
		MessageBox(hDlg, L". Try again.", L"Error", MB_OK);
		return FALSE;
	}

	WCHAR lpstrColums[3];
	*((LPWORD)lpstrColums) = cchNums;

	SendDlgItemMessage(hDlg, nIDDlgItem, EM_GETLINE, (WPARAM)0, (LPARAM)lpstrColums);

	lpstrColums[cchNums] = 0;

	INT num = _wtoi(lpstrColums);

	if (num == 0)
	{
		MessageBox(hDlg, L"It's not a number. Try again.", L"Error", MB_OK);
		return FALSE;
	}
	*dest = num;
	return TRUE;
}
INT_PTR CALLBACK Edit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			int rows, cols;
			if (TryGetNumFromEditCtrl(hDlg, IDC_ROWS, &rows) &&
				TryGetNumFromEditCtrl(hDlg, IDC_COLS, &cols))
			{
				table.rows = rows;
				table.cols = cols;
				SendMessage(GetParent(hDlg), WM_BUILD_TABLE, 0, 0);
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
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

VOID GetUserFileName(HWND hWnd, WCHAR* szFileName)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	WCHAR szFile[MAX_LOADSTRING];

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Text\0*.txt\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	GetOpenFileName(&ofn);
	wcscpy_s(szFileName, wcslen(szFile) + 1, szFile);
}
VOID LoadTextFromFile(WCHAR* szFileName, STRINGVECTOR* str)
{
	std::ifstream t(szFileName);
	std::string string((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	const int num = table.rows * table.cols;
	const int cellsInRow = string.size() / num;

	for (int i = 0; i < num; i++)
	{
		if (i != num - 1)
		{
			str->push_back(string.substr(i * cellsInRow, cellsInRow));
		}
		else
		{
			str->push_back(string.substr(i * cellsInRow, string.size() - (i - 1) * cellsInRow));
		}
	}
}

INT TryToPlace(std::string str, HWND hWnd, HDC hdc, RECT cellForText, int j) {
	HFONT oldFont, newFont;

	int indent = 75;

	RECT rect, winRect;
	GetWindowRect(hWnd, &winRect);

	int height = (winRect.bottom - winRect.top - indent) / table.rows;

	int width = (winRect.right - winRect.left) / table.cols;
	int weight = width * height / str.length();
	int charWidth = sqrt(weight / 2);
	newFont = CreateFont(charWidth * 2, charWidth, 0, 0, 500, 0, 0, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Arial");

	oldFont = (HFONT)SelectObject(hdc, newFont);



	DrawTextA(hdc, str.c_str(), -1, &cellForText, DT_CALCRECT | DT_WORDBREAK | DT_LEFT | DT_EDITCONTROL);
	rect.right = winRect.right / table.cols * (j + 1);

	cellForText.bottom = cellForText.top + height;

	DrawTextA(hdc, str.c_str(), -1, &cellForText, DT_WORDBREAK | DT_EDITCONTROL);

	return cellForText.bottom;
}

VOID DrawTable(HWND hWnd, RECT wndRect, HDC hdc)
{
	INT indent = 5,
		maxRowHight = 0,
		sizeOfColumn;

	RECT rect, cellForText;
	HBRUSH brush;
	COLORREF colorText = RGB(0, 0, 0),
		colorBack = RGB(255, 255, 255),
		colorLine = RGB(0, 0, 0);

	brush = CreateSolidBrush(colorBack);
	SelectObject(hdc, brush);
	Rectangle(hdc, wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
	DeleteObject(brush);

	sizeOfColumn = wndRect.right / table.cols;

	for (int i = 0; i < table.rows; i++) {

		rect.top = maxRowHight;

		for (int j = 0; j < table.cols; j++) {

			rect.left = sizeOfColumn * j;
			rect.right = wndRect.right / table.cols * (j + 1);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, colorText);

			cellForText.top = rect.top + indent;
			cellForText.right = rect.right - indent;
			cellForText.left = rect.left + indent;

			std::string str = table.strings[table.cols * i + j];

			int rectBottom = TryToPlace(str, hWnd, hdc, cellForText, j);
			if (rectBottom > maxRowHight)
				maxRowHight = rectBottom;
		}

		DrawLine(hdc, colorLine, wndRect.left, maxRowHight, wndRect.right, maxRowHight);
	}

	DrawVerticalTableLines(hdc, colorLine, sizeOfColumn, maxRowHight);

	SetBkMode(hdc, OPAQUE);
}
VOID DrawLine(HDC hdc, COLORREF color, int x1, int y1, int x2, int y2)
{
	HPEN pen = CreatePen(PS_INSIDEFRAME, 1, color);
	POINT pt;
	SelectObject(hdc, pen);
	MoveToEx(hdc, x1, y1, &pt);
	LineTo(hdc, x2, y2);
	DeleteObject(pen);
}
VOID DrawVerticalTableLines(HDC hdc, COLORREF color, INT cellSizeX, INT tableSizeY)
{
	for (int i = 1; i < table.cols; i++) {
		DrawLine(hdc, color, i * cellSizeX, 0, i * cellSizeX, tableSizeY);
	}
}

VOID Repaint(HWND hWnd)
{
	InvalidateRect(hWnd, NULL, TRUE);
}