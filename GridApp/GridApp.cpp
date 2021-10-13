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

HPEN pen;
HBRUSH brush;;
RECT padding;

ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL				TryGetNumFromEditCtrl(HWND, int, INT*);
VOID				GetUserFileName(HWND, WCHAR*);
VOID				FromString(TABLE*, std::string);
VOID				DrawTable(HWND, HDC, RECT, TABLE);
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
		return -1;
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

	pen = CreatePen(PS_INSIDEFRAME, 1, RGB(0, 0, 0));
	brush = CreateSolidBrush(RGB(255, 255, 255));
	RECT r{ 5, 5, 5, 5 };
	padding = r;

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
		if (szFileName[0] == '\0') {
			break;
		}
		std::ifstream t(szFileName);
		std::string string((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		t.close();
		FromString(&table, string);
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

		RECT client;
		GetClientRect(hWnd, &client);

		DrawTable(hWnd, hdc, client, table);

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
VOID FromString(TABLE* table, std::string string)
{
	table->strings.clear();
	int len = string.size();
	if (len == 0)
		return;
	if (table->rows < 1 || table->cols < 1)
		return;
	int cellsCount = table->rows * table->cols;
	int stringSizeInCell = len / cellsCount;

	int start = 0;
	int end = stringSizeInCell;
	while (start < len) {
		table->strings.push_back(string.substr(start, min(end, len) - start));
		start = end;
		end += stringSizeInCell;
	}
	while (table->strings.size() < cellsCount)
		table->strings.push_back("");
}

VOID DrawTable(HWND hWnd, HDC hdc, RECT drawBounds, TABLE table)
{
	if (table.rows < 1 || table.cols < 1)
		return;
	if (table.strings.size() < (table.rows * table.cols))
		return;

	SelectObject(hdc, pen);
	SelectObject(hdc, brush);

	Rectangle(hdc, drawBounds.left, drawBounds.top, drawBounds.right, drawBounds.bottom);

	int cellWidth = drawBounds.right / table.cols;
	int cellHeight = drawBounds.bottom / table.rows;

	POINT tmp;
	int y0 = drawBounds.top;
	for (int y = 0; y < table.rows; y++) {
		int maxY = 0;
		for (int x = 0; x < table.cols; x++) {
			auto str = table.strings[(y * table.cols) + x];
			auto text = str.c_str();

			RECT cellForText;
			cellForText.left = cellWidth * x + padding.left;
			cellForText.top = y0 + padding.top;
			cellForText.right = cellForText.left + cellWidth + padding.right;
			cellForText.bottom = cellForText.top + 255;

			DrawTextA(hdc, text, -1, &cellForText, DT_CALCRECT | DT_WORDBREAK | DT_LEFT | DT_EDITCONTROL);

			int height = (cellForText.bottom - cellForText.top) + padding.bottom;
			maxY = max(maxY, height);

			DrawTextA(hdc, text, -1, &cellForText, DT_LEFT | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		}

		y0 += maxY;
		MoveToEx(hdc, drawBounds.left, y0, &tmp);
		LineTo(hdc, drawBounds.right, y0);
	}
	for (int x = 1; x < table.cols; x++) {
		int x0 = cellWidth * x;
		MoveToEx(hdc, x0, drawBounds.top, &tmp);
		LineTo(hdc, x0, y0);
	}

	SetBkMode(hdc, OPAQUE);
}


VOID Repaint(HWND hWnd)
{
	InvalidateRect(hWnd, NULL, TRUE);
}