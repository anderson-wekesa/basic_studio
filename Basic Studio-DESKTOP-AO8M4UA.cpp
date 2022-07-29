#define _CRT_SECURE_NO_WARNINGS

#include "header.h"
#include "Basic Studio.h"
#include <Windows.h>
#include <WinUser.h>
#include <cstring>
#include <wingdi.h>
#include <ShObjIdl.h>
#include <iostream>
#include <CommCtrl.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SplashProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK CompilerDiagProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void loadSplashUI(HWND hWnd);
void showSplashScreen();
void loadMainUI(HWND hwnd);
void openFile(PWSTR FilePath);
void saveFile();
void loadCompilerPath();
void registerCompilerDiag(HINSTANCE hInstance);
void displayCompilerDiag(HWND hwnd);
void loadCompilerDiagUI(HWND hwnd);
void compileSource();


int GlobalRight;
int GlobalBottom;

// Path to the Compiler
char Path[200];
char callPath[200] = "start ";

HWND hSplash, hwnd, hEditor, hCompilerDiag, hCompilerPathBox;

HRESULT hr;

HFONT hLabelFont, hCatchLineFont;

//Path to source file
LPWSTR FilePath;

HMENU hMenuBar, hFileMenu, hOptionsMenu;

RECT editorWindowRect;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdShow, int nCmdShow) {

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	const wchar_t CLASS_NAME[] = L"Basic Studio";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	void showSplashScreen();
	RegisterClass(&wc);
	registerCompilerDiag(hInstance);

	hwnd = CreateWindowEx(0, CLASS_NAME, L"Basic Studio 1.0", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	770, 440, NULL, NULL, hInstance, NULL);

	GlobalRight = 770;
	GlobalBottom = 440;

	if (hwnd == NULL) {
		return FALSE;
	}

	ShowWindow(hwnd, nCmdShow);


	MSG msg = {};

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		case WM_NCCREATE:
		{
			//showSplashScreen();
			//SetTimer(hSplash, 1, 10000, NULL);
		}
		break; 

		case WM_CREATE:
		{
			loadCompilerPath();
			loadMainUI(hwnd);
			//displayCompilerDiag(hwnd);
		}
		break;

		case WM_PAINT: 
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			
			EndPaint(hwnd, &ps);
		}
		break;

		case WM_SIZE:
		{
			long length = GetWindowTextLength(hEditor);
			wchar_t *data = new wchar_t[length];
			GetWindowText(hEditor, data, length);
			GetClientRect(hwnd, &editorWindowRect);
			int winRight = editorWindowRect.right;
			int winBottom = editorWindowRect.bottom;
			if (winRight != GlobalRight) {
				winRight -= 40;
			}
			GlobalRight = winRight;

			if (winBottom != GlobalBottom) {
				winBottom -= 40;
			}

			DestroyWindow(hEditor);
			hEditor = CreateWindowEx(0, L"Edit", data, WS_VISIBLE |WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_BORDER | ES_MULTILINE,
				20, 10, winRight, winBottom, hwnd, NULL, NULL, NULL);

			delete data;
			
			return 0;
		}
		break; 

		case WM_KEYDOWN:
		{
			switch (LOWORD(wParam)) {
				case 0x4F: //" SHIFT + 'O' " Shortcut.
				{
					if (GetKeyState(VK_SHIFT) && 0x8000) {
						IFileOpenDialog *pOpenDialog;
						hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_ALL, IID_IFileOpenDialog,
							reinterpret_cast<void**>(&pOpenDialog));

						if (SUCCEEDED(hr)) {
							hr = pOpenDialog->Show(NULL);

							if (SUCCEEDED(hr)) {
								IShellItem *pItem;
								hr = pOpenDialog->GetResult(&pItem);

								if (SUCCEEDED(hr)) {
									hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);

									if (SUCCEEDED(hr)) {

										openFile(FilePath);
										EnableMenuItem(hFileMenu, FILE_MENU_CLOSEPROJECT, MF_ENABLED);
										EnableMenuItem(hFileMenu, FILE_MENU_SAVE, MF_ENABLED);
										EnableMenuItem(hFileMenu, FILE_MENU_SAVEAS, MF_ENABLED);

									}
								}
								pItem->Release();
							}
							pOpenDialog->Release();
						}
					}
				}
				break;

				case 0x53: //" SHIFT + 'S' " Shortcut.
				{
					if (GetKeyState(VK_SHIFT) && 0x8000) {
						SendMessage(hwnd, WM_COMMAND, (WPARAM)FILE_MENU_SAVE, (LPARAM)MAKELONG(TRUE, 0));
					}
				}
				break;

			}
		}
		break;

		case WM_COMMAND:
		{
			switch (wParam) {
				case FILE_MENU_NEW:
				{
					ShowWindow(hEditor, SW_NORMAL);
					EnableMenuItem(hFileMenu, FILE_MENU_CLOSEPROJECT, MF_ENABLED);
					EnableMenuItem(hFileMenu, FILE_MENU_SAVEAS, MF_ENABLED);
				}
				break;

				case FILE_MENU_OPEN:
				{
					IFileOpenDialog *pOpenDialog;
					hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_ALL, IID_IFileOpenDialog,
						reinterpret_cast<void**>(&pOpenDialog));

					if (SUCCEEDED(hr)) {
						hr = pOpenDialog->Show(NULL);

						if (SUCCEEDED(hr)) {
							IShellItem *pItem;
							hr = pOpenDialog->GetResult(&pItem);

							if (SUCCEEDED(hr)) {
								hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);

								if (SUCCEEDED(hr)) {
								
									openFile(FilePath);
									EnableMenuItem(hFileMenu, FILE_MENU_CLOSEPROJECT, MF_ENABLED);
									EnableMenuItem(hFileMenu, FILE_MENU_SAVE, MF_ENABLED);
									EnableMenuItem(hFileMenu, FILE_MENU_SAVEAS, MF_ENABLED);
									EnableMenuItem(hMenuBar, MAIN_MENU_RUN, MF_ENABLED);

								}
							}
							pItem->Release();
						}
						pOpenDialog->Release();
					}
				}
				break;

				case FILE_MENU_CLOSEPROJECT:
				{
					ShowWindow(hEditor, SW_HIDE);
					SetWindowText(hEditor, L"");
					EnableMenuItem(hFileMenu, FILE_MENU_CLOSEPROJECT, MF_DISABLED | MF_GRAYED);
					EnableMenuItem(hFileMenu, FILE_MENU_SAVE, MF_DISABLED | MF_GRAYED);
					EnableMenuItem(hFileMenu, FILE_MENU_SAVEAS, MF_DISABLED | MF_GRAYED);
					EnableMenuItem(hMenuBar, MAIN_MENU_RUN, MF_DISABLED | MF_GRAYED);
				}
				break;

				case FILE_MENU_SAVE:
				{
					saveFile();
				}
				break;

				case FILE_MENU_SAVEAS:
				{
					IFileSaveDialog *pSaveDialog;
					hr = CoCreateInstance(__uuidof(FileSaveDialog), NULL, CLSCTX_ALL, IID_IFileSaveDialog,
						reinterpret_cast<void**>(&pSaveDialog));
					
					if (SUCCEEDED(hr)) {
						hr = pSaveDialog->Show(NULL);

						if (SUCCEEDED(hr)) {
							IShellItem *pItem;
							hr = pSaveDialog->GetResult(&pItem);

							if (SUCCEEDED(hr)) {
								hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);

								if (SUCCEEDED(hr)) {

									saveFile();
									EnableMenuItem(hFileMenu, FILE_MENU_SAVE, MF_ENABLED);
									EnableMenuItem(hMenuBar, MAIN_MENU_RUN, MF_ENABLED);

								}
							}
						}
					}
				}
				break;

				case FILE_MENU_EXIT:
				{
					UpdateWindow(hwnd);
					//SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

				case MAIN_MENU_RUN:
				{

					compileSource();

				}
				break;

				case OPTIONS_MENU_COMPILER:
				{
					displayCompilerDiag(hwnd);
				}
				break;

			}
		}
		break; 

		case WM_CLOSE:
		{
			int result = MessageBox(hwnd, L"Are you sure you want to quit?", L"Quit?", MB_OKCANCEL | MB_ICONQUESTION);

			if (result == IDOK) {
				CoTaskMemFree(FilePath);
				DestroyWindow(hwnd);
			}
			else {
				//Do nothing...
			}
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
			

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void loadCompilerPath() {

	FILE *fCompiler;
	fCompiler = _wfopen(L"CompilerPath.txt", L"rb");

	fseek(fCompiler, 0, SEEK_END);
	long size = ftell(fCompiler);
	rewind(fCompiler);

	if (size > 0) {
		char *initData = new char[size];
		fgets(initData, size + 1, fCompiler);
		strcpy(Path, initData);
		strcat(callPath, initData);
		fclose(fCompiler);
	}
}

/*void compileSource() {

	if (Path == NULL) {
		MessageBox(hwnd, L"No compiler availabe!", L"No Compiler", MB_OK | MB_ICONERROR);
	}
	else if (FilePath == NULL)
	{
		MessageBox(hwnd, L"No source file found!", L"No source file", MB_OK | MB_ICONERROR);

	}
	else {

		//std::system("start");

		strcat(callPath, " ");
		strcat(callPath, (char*)FilePath);
		std::mbsrtowcs()
		SetWindowTextA(hEditor, callPath);
		/*strcat(callPath, " ");
		strcat(callPath, "-static-libgcc -static-libstdc++");
		std::system(callPath);
		std::system("pause");

	} 

} */

void saveCompilerPath(long size) {

	FILE *fCompiler;
	fCompiler = _wfopen(L"CompilerPath.txt", L"w");
	fputs(Path, fCompiler);
	fclose(fCompiler);

}



void openFile(PWSTR FilePath) {

	FILE *fObj;
	fObj = _wfopen(FilePath, L"rb");

	fseek(fObj, 0, SEEK_END);
	long length = ftell(fObj);
	length++;
	rewind(fObj);

	char *finalText = new char[length];
	char *text = new char[length];

	//fgets(finalText, length + 1, fObj);
	fgets(finalText, length, fObj);
	fseek(fObj, 0, SEEK_CUR);
	long newLength = ftell(fObj);
	newLength = length - newLength;
	while (fgets(text, newLength, fObj)) {
		strcat(finalText, text);
	}
	

	if (!IsWindowVisible(hEditor)) {
		ShowWindow(hEditor, SW_NORMAL);
	}

	SetWindowTextA(hEditor, finalText);
	fclose(fObj);

}

void saveFile() {

	FILE *outTextStream;
	outTextStream = _wfopen(FilePath, L"w");

	int length = GetWindowTextLength(hEditor);
	wchar_t *fileData = new wchar_t[length + 1];
	GetWindowText(hEditor, fileData, length);


	fputws(fileData, outTextStream);
	fclose(outTextStream);

}


void showSplashScreen() {

	const wchar_t splashClass[] = L"Splash Screen";

	WNDCLASS splash = {};

	splash.lpfnWndProc = SplashProc;
	splash.hInstance = NULL;
	//splash.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
	splash.lpszClassName = splashClass;

	RegisterClass(&splash);

	hSplash = CreateWindowEx(0, splashClass, NULL, WS_VISIBLE | WS_BORDER, 160, 130, 700, 300, NULL,
		NULL, NULL, NULL);

}


LRESULT CALLBACK SplashProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		case WM_CREATE:
		{
			loadSplashUI(hWnd);
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hWnd, &ps);
		}
		break;

		case WM_TIMER:
		{
			KillTimer(hSplash, 1);
			DeleteObject(&hLabelFont);
			DeleteObject(&hCatchLineFont);
			DestroyWindow(hSplash);
			ShowWindow(hwnd, SW_NORMAL);
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void loadSplashUI(HWND hWnd) {

	LOGFONT hLabelLog, hCatchLineLog;

	HWND hLogoCont = CreateWindowEx(0, L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_ICON, 20, 10, 40, 40, hWnd,
		NULL, NULL, NULL);

	HICON hLogo = (HICON) LoadImage(NULL, L"small.ico", IMAGE_ICON, 200,200, LR_LOADFROMFILE);
	SendMessage(hLogoCont, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hLogo);

	HWND hLabel = CreateWindowEx(0, L"static", L"BASIC STUDIO 1.0", WS_VISIBLE | WS_CHILD | ES_CENTER,
		240, 20, 400, 50, hWnd, NULL, NULL, NULL);

	std::memset(&hLabelLog, 0, sizeof(hLabelLog));
	hLabelLog.lfWeight = FW_SEMIBOLD;
	hLabelLog.lfHeight = -48;
	hLabelFont = CreateFontIndirect(&hLabelLog);
	SendMessage(hLabel, WM_SETFONT, (WPARAM)hLabelFont, (LPARAM)MAKELONG(TRUE, 0));

	HWND hCatchLine = CreateWindowEx(0, L"Static", L"Your Trusted IDE", WS_VISIBLE | WS_CHILD | SS_CENTER,
		240, 90, 400, 50, hWnd, NULL, NULL, NULL);

	std::memset(&hCatchLineLog, 0, sizeof(hCatchLineLog));
	hCatchLineLog.lfWeight = FW_SEMIBOLD;
	hCatchLineLog.lfHeight = -20;
	hCatchLineFont = CreateFontIndirect(&hCatchLineLog);
	SendMessage(hCatchLine, WM_SETFONT, (WPARAM)hCatchLineFont, (LPARAM)MAKELONG(TRUE, 0));

	HWND hDetails = CreateWindowEx(0, L"Static", L"2019  OxysGenon Softwares", WS_VISIBLE | WS_CHILD,
		470, 235, 200, 20, hWnd, NULL, NULL, NULL);
}

void loadMainUI(HWND hwnd) {

	hEditor = CreateWindowEx(0, L"Edit", L"//Enter code here...", WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_BORDER | ES_MULTILINE,
		20, 10, 710, 350, hwnd, NULL, NULL, NULL);

	hMenuBar = CreateMenu();

	hFileMenu = CreateMenu();
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_NEW, L"New");
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_OPEN, L"Open");
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_CLOSEPROJECT, L"Close Project");
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_SAVE, L"Save");
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_SAVEAS, L"Save As");
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_EXIT, L"Exit");

	AppendMenu(hMenuBar, MF_STRING, MAIN_MENU_RUN, L"Run");

	hOptionsMenu = CreatePopupMenu();
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hOptionsMenu, L"Options");
	AppendMenu(hOptionsMenu, MF_STRING, OPTIONS_MENU_COMPILER, L"Compiler");

	if (FilePath == NULL) {
		EnableMenuItem(hFileMenu, FILE_MENU_SAVE, MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hFileMenu, FILE_MENU_SAVEAS, MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hFileMenu, FILE_MENU_CLOSEPROJECT, MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenuBar, MAIN_MENU_RUN, MF_DISABLED | MF_GRAYED);
	}

	SetMenu(hwnd, hMenuBar);

}


LRESULT CALLBACK CompilerDiagProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);
		}
		break;

		case WM_COMMAND:
		{

			switch (wParam) {

				case 1:
				{
					long length = GetWindowTextLength(hCompilerPathBox);
					length += 1;
					char *cPath = new char[length];
					GetWindowTextA(hCompilerPathBox, cPath, length);
					std::memset(Path, '\0', sizeof(Path));
					strcpy(Path, cPath);
					strcat(callPath, cPath);
					delete cPath;
					saveCompilerPath(length);
					DestroyWindow(hwnd);
				}
				break;

				case 2:
				{
					//Edit_SearchWeb(hCompilerPathBox);
					//DestroyWindow(hwnd);
				}
				break;

			}

		}
		break;
		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
		}
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

void loadCompilerDiagUI(HWND hCompilerDiag) {

	CreateWindowEx(0, L"static", L"Enter compiler path:", WS_VISIBLE | WS_CHILD, 20, 20, 140, 20,
		hCompilerDiag, NULL, NULL, NULL);
	hCompilerPathBox = CreateWindowEx(0, L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, 20, 50, 350, 20, hCompilerDiag, 
		NULL, NULL, NULL);
	CreateWindowEx(0, L"Button", L"OK", WS_VISIBLE | WS_CHILD, 210, 100, 60, 30, hCompilerDiag, (HMENU) 1, NULL, NULL);
	CreateWindowEx(0, L"Button", L"Cancel", WS_VISIBLE | WS_CHILD, 290, 100, 60, 30, hCompilerDiag, (HMENU) 2, NULL, NULL);
	Button_SetElevationRequiredState(hCompilerDiag, true);

}


void registerCompilerDiag(HINSTANCE hInstance) {
	const wchar_t DIAG_CLASS_NAME[] = L"Comp Diag";

	WNDCLASS diagClass = {};

	diagClass.lpfnWndProc = CompilerDiagProc;
	diagClass.hInstance = hInstance;
	diagClass.lpszClassName = DIAG_CLASS_NAME;

	if (!RegisterClass(&diagClass)) {
		MessageBox(hwnd, L"Compiler Dialog not registered", NULL, MB_OK);
	}
}



void displayCompilerDiag(HWND hwnd) {
	
	hCompilerDiag = CreateWindowEx(0,L"Comp Diag", L"Compiler Options", WS_VISIBLE | WS_OVERLAPPEDWINDOW , 
		230, 200, 400, 200, hwnd, NULL, NULL, NULL);
	if (hCompilerDiag == NULL) {
		MessageBox(hwnd, L"Compiler Dialog creation failed", NULL, MB_OK);
	}

	loadCompilerDiagUI(hCompilerDiag);

}


