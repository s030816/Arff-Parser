// Arff_parser.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "Arff_parser.h"

#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FilesSel(HWND, UINT, WPARAM, LPARAM);
unsigned int __stdcall rfiles_t(void* data);
unsigned int __stdcall timer(void* data);
void				timer_init(void);


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR selectedArffFolder[MAX_PATH] = L"";
WCHAR atrFile[MAX_PATH];
char relationHeader[4096];
ATTRIBUTE *attribute_table;
FILE_BUFFER *file_buffer;
size_t attribute_count, files_count;
HWND listh1, listh2, btn1, btn2, btn3, btn4, btn5, btn6, static1, static2, statusb1;
uintptr_t T1, T2;
LARGE_INTEGER nStartTime, nStopTime, nFrequency;
HANDLE ghMutex;

// Busenos rezimai
//==============================
BOOL T_IN_PROGRESS = FALSE;
BOOL T_RESUME = FALSE;
BOOL T_STOP = FALSE;
BOOL T_START = FALSE;
BOOL T_DONE = FALSE;
BOOL T_ABORT = FALSE;

BOOL T_EXIT = FALSE;
//==============================


// Forward declarations of functions included in this code module:


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
    LoadStringW(hInstance, IDC_ARFF_PARSER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ARFF_PARSER));

    MSG msg;

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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ARFF_PARSER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ARFF_PARSER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
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

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 900, 550, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

// Description: 
//   Creates a status bar and divides it into the specified number of parts.
// Parameters:
//   hwndParent - parent window for the status bar.
//   idStatus - child window identifier of the status bar.
//   hinst - handle to the application instance.
//   cParts - number of parts into which to divide the status bar.
// Returns:
//   The handle to the status bar.
//
HWND DoCreateStatusBar(HWND hwndParent, int idStatus)
{
	HWND hwndStatus;
	int cyVScroll;  // Height of scroll bar arrow.
	RECT rcClient;

	// Ensure that the common control DLL is loaded.
	InitCommonControls();

	cyVScroll = GetSystemMetrics(SM_CYVSCROLL);
	// Get the coordinates of the parent window's client area.
	GetClientRect(hwndParent, &rcClient);
	// Create the status bar.
	hwndStatus = CreateWindowEx(
		0, PROGRESS_CLASS, (LPTSTR)NULL,
		WS_CHILD | WS_VISIBLE, rcClient.left,
		rcClient.bottom - cyVScroll,
		rcClient.right, cyVScroll,
		hwndParent, (HMENU)0, (HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE), NULL);                   // no window creation data
	//SendMessage(hwndStatus, PBM_SETRANGE, 0, MAKELPARAM(0, cParts));
	SendMessage(hwndStatus, PBM_SETSTEP, (WPARAM)1, 0);
	return hwndStatus;
}


INT_PTR CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			
			listh1 = reg_obj(NULL, L"LISTBOX",
				WS_CHILD | WS_VISIBLE | LBS_EXTENDEDSEL | WS_VSCROLL | WS_BORDER | WS_HSCROLL,
				50, 50, 300, 400, hWnd, (HMENU)IDC_LISTBOX);
			listh2 = reg_obj(NULL, L"LISTBOX",
				WS_CHILD | WS_VISIBLE | LBS_EXTENDEDSEL | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
				50 + 350, 50, 300, 400, hWnd, (HMENU)IDC_LISTBOX2);
			btn1 = reg_obj(L">", L"BUTTON",
				WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
				50 + 310, 200, 30, 20, hWnd, (HMENU)IDC_BUTTON1);
			btn2 = reg_obj(L"<", L"BUTTON",
				WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 
				50 + 310, 230, 30, 20, hWnd, (HMENU)IDC_BUTTON2);
			btn4 = reg_obj(L"Begin", L"BUTTON",
				WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
				50 + 700, 260, 100, 50, hWnd, (HMENU)IDC_BUTTON4);
			btn5 = reg_obj(L"Pause", L"BUTTON",
				WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
				50 + 700, 315, 100, 50, hWnd, (HMENU)IDC_BUTTON5);
			btn6 = reg_obj(L"Abort", L"BUTTON",
				WS_TABSTOP  | WS_CHILD | BS_DEFPUSHBUTTON,
				50 + 700, 380, 100, 50, hWnd, (HMENU)IDC_BUTTON6);
			static1 = reg_obj(L"No arff folder selected", L"static",
				WS_VISIBLE | WS_CHILD ,
				50, 10, 660, 20, hWnd, (HMENU)IDC_STATIC1);
			static2 = reg_obj(L"Timer", L"static",
				WS_VISIBLE | WS_CHILD,
				750, 50, 100, 20, hWnd, (HMENU)IDC_STATIC2);
			statusb1 = DoCreateStatusBar(hWnd, IDC_STATUSBAR);
			ghMutex = CreateMutex(NULL, FALSE, NULL);
			T2 = (uintptr_t)_beginthreadex(0, 0, &timer, 0, 0, 0);
			
			
			//update_lists(hWnd, attribute_table, attribute_count);
			/*
			char out_t[500];
			int i;
			for (i = 0; i < 30; ++i)
			{
				sprintf(out_t, "Testas%d", i);
				add_item(hWnd, out_t, (HMENU)IDC_LISTBOX);
			}
			*/
				/*
			SendMessageW(GetDlgItem(hWnd, IDC_LISTBOX),
				LB_ADDSTRING,
				(WPARAM)NULL,
				(LPARAM)L"Testas");
			SendMessageW(GetDlgItem(hWnd, IDC_LISTBOX),
				LB_ADDSTRING,
				(WPARAM)NULL,
				(LPARAM)L"Testas2");
				*/
		}
		break;
    case WM_COMMAND:
        {
		

            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDC_BUTTON1:
				//remove_item(hWnd, (HMENU)IDC_LISTBOX);
				insert_item(attribute_table,hWnd);
				//update_lists(hWnd, attribute_table, attribute_count);
				break;
			case IDC_BUTTON2:
				remove_item(attribute_table, hWnd);
				break;
			case IDC_BUTTON3:
				//remove_item(attribute_table, hWnd);
				break;
			case IDC_BUTTON4:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, FilesSel);
				break;
			case IDC_BUTTON5:
				if (T_STOP)
				{
					T_RESUME = TRUE;
					SendMessage(btn5, WM_SETTEXT, (WPARAM)0, (LPARAM)L"Pause");
				}
				else
				{
					T_STOP = TRUE;
					SendMessage(btn5, WM_SETTEXT, (WPARAM)0, (LPARAM)L"Resume");
				}
				break;
			case IDC_BUTTON6:
				T_ABORT = TRUE;
				break;
			case ID_FILE_SELECTFOLDER:
				if (browser(selectedArffFolder))
				{
					lstrcpyW(atrFile, selectedArffFolder);
					first_file(atrFile);
					attribute_table = read_data_attributes(atrFile, hWnd, &attribute_count);
					ShowWindow(btn1, SW_SHOW);
					ShowWindow(btn2, SW_SHOW);
					SetWindowTextW(static1, selectedArffFolder);
					ShowWindow(btn4, SW_SHOW);
				}
				//attribute_table = read_data_attributes(file_buffer[0].path, hWnd, &attribute_count);
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
				if (InterlockedCompareExchange(&T_IN_PROGRESS, T_IN_PROGRESS, T_IN_PROGRESS))
				{
					MessageBoxA(hWnd, "Cannot close while program is still processing data", "Warning", MB_OK);
					break;
				}
				T_EXIT = TRUE;
				T_START = TRUE;
				WaitForSingleObject((HANDLE)T1, (DWORD)2000);
				CloseHandle((HANDLE)T1);
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CLOSE:
		if (InterlockedCompareExchange(&T_IN_PROGRESS, T_IN_PROGRESS, T_IN_PROGRESS))
		{
			MessageBoxA(hWnd, "Cannot close while program is still processing data", "Warning", MB_OK);
			break;
		}
		T_EXIT = TRUE;
		T_START = TRUE;
		WaitForSingleObject((HANDLE)T1, (DWORD)2000);
		CloseHandle((HANDLE)T1);
		DestroyWindow(hWnd);
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
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

// Message handler for file dialog box.
INT_PTR CALLBACK FilesSel(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		file_buffer = file_search(selectedArffFolder,  &files_count, hDlg); // FIX 
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			update_selected_file_list(file_buffer, hDlg);
			//read_files(file_buffer, files_count, attribute_table, attribute_count);
			T_IN_PROGRESS = TRUE;
			T_START = TRUE;
			//T1 = (uintptr_t)_beginthreadex(0, 0, &rfiles_t, 0, 0, 0);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		case IDCANCEL:
			files_count = 0;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void timer_init(void)
{
	QueryPerformanceFrequency(&nFrequency);
	QueryPerformanceCounter(&nStartTime);
}

unsigned int __stdcall rfiles_t(void* data)
{
	
	read_files(file_buffer, files_count, attribute_table, attribute_count,statusb1);
	return 0;
}

unsigned int __stdcall timer(void* data) // TODO: perdaryti i thread manageri
{
	
	WCHAR output_buffer[400];
	double last_time = 0.0;
	for (;;)
	{

		while (!InterlockedCompareExchange(&T_START, T_START, T_START))
			Sleep(200);
		if (InterlockedCompareExchange(&T_EXIT, T_EXIT, T_EXIT))
		{
			break;
		}
		ShowWindow(btn4, SW_HIDE); // master button
		ShowWindow(btn1, SW_HIDE); // >
		ShowWindow(btn2, SW_HIDE); // <
		ShowWindow(btn5, SW_SHOW); // Pause
		ShowWindow(btn6, SW_SHOW); // Abort
		T_START = FALSE;
		timer_init(); 
		last_time = 0; // nuresetinamas timeris
		T1 = (uintptr_t)_beginthreadex(0, 0, &rfiles_t, 0, 0, 0); // nauja gija parsinimo darbui
		// Timeriui nereikia bereikalingai eikvoti resursu - 
		// Sleep kiekvieno ciklo gale, kad liktu daugiau loginiu giju pagrindiniam darbui
		for (last_time = 0.0; !InterlockedCompareExchange(&T_DONE, T_DONE, T_DONE); Sleep(300))
		{
			QueryPerformanceCounter(&nStopTime);
			if ((double)(nStopTime.QuadPart - nStartTime.QuadPart) / nFrequency.QuadPart - last_time > 0.3)
			{
				last_time = (double)(nStopTime.QuadPart - nStartTime.QuadPart) / nFrequency.QuadPart;
				swprintf(output_buffer, 400,
					L"%.1f", last_time);
				SetWindowTextW(static2, output_buffer);
			}

			if (InterlockedCompareExchange(&T_STOP, T_STOP, T_STOP))
			{
				while (!InterlockedCompareExchange(&T_RESUME, T_RESUME, T_RESUME))
					Sleep(200);
				T_RESUME = FALSE;
				T_STOP = FALSE;
			}
		}
		CloseHandle((HANDLE)T1);
		T_DONE = FALSE;
		T_IN_PROGRESS = FALSE;
		T_ABORT = FALSE;
		ShowWindow(btn4, SW_SHOW);
		ShowWindow(btn1, SW_SHOW);
		ShowWindow(btn2, SW_SHOW);
		ShowWindow(btn5, SW_HIDE);
		ShowWindow(btn6, SW_HIDE);
		free(file_buffer);
		file_buffer = NULL;
		files_count = 0;
	}
	free(output_buffer);
	return 0;
}
