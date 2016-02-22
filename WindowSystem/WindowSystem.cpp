#include "WindowSystem.h"
#include "GESignal.h"
#include "Debug.h"
#include <commctrl.h>                   // common controls
#include "resource.h"

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "comctl32.lib")

namespace GE{

		static const int controlHeight=100;

		const char windowsClassName[] = "GEWindowClass";

		WindowSystem & WindowSystem::GetInstance()
		{
			static WindowSystem window;
			return window;
		}

		WindowSystem::WindowSystem() :
			System(WINDOW_SYSTEM) {}

		void WindowSystem::Start(std::string const & title, int width, int height)	
		{
			m_title = title;

			RECT fullWinRect = { 0, 0, width, height };
			AdjustWindowRect(&fullWinRect,			//The rectangle for the full size of the window
				WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,	//The style of the window, which must match what is passed in to CreateWindow below
				FALSE);					//Does this window have a menu?

			//Register the window class for the game.
			WNDCLASSEX wc = { sizeof(WNDCLASSEX),	//The size of this structure (passing the size allows Microsoft to update their interfaces and maintain backward compatibility)
				CS_CLASSDC,							//The style of the window class--this is the base type (one device context for all windows in the process)
				MainWndProc,						//Message handling function
				0L, 0L,								//Extra memory
				GetModuleHandle(NULL),				//Handle to the instance that has the windows procedure--NULL means use this file.
				LoadIcon(hInstance, IDI_APPLICATION), //Add an Icon as a resource and add them here
				LoadCursor(NULL, IDC_ARROW),		//Default arrow cursor
				NULL, MAKEINTRESOURCE(IDR_MENU1),							//Background brush and menu
				windowsClassName, LoadIcon(hInstance, IDI_APPLICATION) };			//The class name and the small icon (NULL just uses the default)

			RegisterClassEx(&wc);

			//Handle to the instance
			hInstance = wc.hInstance;

			// Ensure that the common control DLL is loaded, and then create 
			// the header control. 
			INITCOMMONCONTROLSEX icex;  //declare an INITCOMMONCONTROLSEX Structure
			icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icex.dwICC = ICC_LISTVIEW_CLASSES;   //set dwICC member to ICC_LISTVIEW_CLASSES    
			// this loads list-view and header control classes.
			InitCommonControlsEx(&icex);

			//WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
			main = CreateWindowEx(WS_EX_WINDOWEDGE,
				windowsClassName,
				m_title.c_str(),								//The name for the title bar
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,				//The style of the window (WS_BORDER, WS_MINIMIZEBOX, WS_MAXIMIZE, etc.) WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZE
				CW_USEDEFAULT, CW_USEDEFAULT,		//The x and y position of the window (screen coords for base windows, relative coords for child windows)
				width,	//Width of the window, including borders
				height,	//Height of the window, including borders and caption
				GetDesktopWindow(),					//The parent window
				NULL,								//The menu for the window
				hInstance,							//The handle to the instance of the window (ignored in NT/2K/XP)
				NULL);

			wc = { sizeof(WNDCLASSEX),	//The size of this structure (passing the size allows Microsoft to update their interfaces and maintain backward compatibility)
				CS_OWNDC,							//The style of the window class--this is the base type (one device context for all windows in the process)
				MainWndProc,						//Message handling function
				0L, 0L,								//Extra memory
				GetModuleHandle(NULL),				//Handle to the instance that has the windows procedure--NULL means use this file.
				LoadIcon(hInstance, IDI_APPLICATION), //Add an Icon as a resource and add them here
				LoadCursor(NULL, IDC_ARROW),		//Default arrow cursor
				NULL, NULL,							//Background brush and menu
				TEXT("OPENGL"), LoadIcon(hInstance, IDI_APPLICATION) };			//The class name and the small icon (NULL just uses the default)

			RegisterClassEx(&wc);

			op = hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
				TEXT("OPENGL"),
				"OpenGL",								//The name for the title bar
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,				//The style of the window (WS_BORDER, WS_MINIMIZEBOX, WS_MAXIMIZE, etc.) WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZE
				0, 0,		//The x and y position of the window (screen coords for base windows, relative coords for child windows)
				width,	//Width of the window, including borders
				height - controlHeight,	//Height of the window, including borders and caption
				main,					//The parent window
				NULL,								//The menu for the window
				hInstance,							//The handle to the instance of the window (ignored in NT/2K/XP)
				NULL);								//The lParam for the WM_CREATE message of this window

			buttonsStructure = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), main, dialogProcedure, NULL);

			if (!hWnd)
				throw 0;

			ShowWindow(main, SW_SHOWDEFAULT);
			UpdateWindow(main);
			ShowWindow(op, SW_SHOWDEFAULT);
			UpdateWindow(op);
			ShowWindow(buttonsStructure, SW_SHOWDEFAULT);
			UpdateWindow(buttonsStructure);

			SetDropList({ "steve","stamp" }, modelsIdControl);
			SetDropList({ "Complete","Incremental","Chebyshev" }, INTERPOLATIONID);
			
			SetFocus(main);

		}

		int WindowSystem::GetSelectedAnimation() const
		{
			return selectedAnimation;
		}

		HWND WindowSystem::getWindowHandler() const{
			return hWnd;
		}

		HWND WindowSystem::GetDownUIHandler() const
		{
			return buttonsStructure;
		}


		bool CALLBACK enumerateChildren(HWND handle, LPARAM lParam)
		{
			if (lParam == WM_CLOSE)
			{
				::SendMessage(handle, WM_CLOSE, 0, 0);      // close child windows
			}

			return true;
		}

		void WindowSystem::Release()
		{
			PostQuitMessage(0);
			HDC hDC = GetDC(hWnd);
			if (hDC)
				ReleaseDC(hWnd, hDC);
			hDC = 0;
			
			::EnumChildWindows(main, (WNDENUMPROC)enumerateChildren, (LPARAM)WM_CLOSE);
			::UnregisterClass(windowsClassName, hInstance);
			::UnregisterClass(TEXT("OpenGL"), hInstance);
			::DestroyWindow(main);   
		}

		WindowSystem::~WindowSystem(){}

		void WindowSystem::Update(double dt){
			MSG msg;
			HACCEL hAccelTable=0;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					switch (msg.message){

					case WM_QUIT:
						MessageBox(NULL, TEXT("BYE"), TEXT("Error"), MB_OK);
						closeWindowSignal();
						break;
					}
					DispatchMessage(&msg);

				}
			}

			char fps[256];
			sprintf_s(fps, 256, "%.2f -- %s", 1.0 / dt,m_title.c_str());
			SetWindowText(main, fps);
		}

		/* main window procedure */
		LONG WINAPI MainWndProc(
			HWND    hWnd,
			UINT    uMsg,
			WPARAM  wParam,
			LPARAM  lParam)
		{
			LONG    lRet = 1;
			PAINTSTRUCT    ps;
			switch (uMsg) {

			case WM_PAINT:
				BeginPaint(hWnd, &ps);
				WindowSystem::GetInstance().refresh();
				EndPaint(hWnd, &ps);
				break;
				
			case WM_SIZE: 
				if (hWnd== WindowSystem::GetInstance().main){
				const size_t width = LOWORD(lParam), height = HIWORD(lParam);
				::SetWindowPos(WindowSystem::GetInstance().op, WindowSystem::GetInstance().main, 0, 0, width, height - controlHeight, SWP_NOZORDER);
				::SetWindowPos(WindowSystem::GetInstance().buttonsStructure, WindowSystem::GetInstance().main, 0, height - controlHeight, width, controlHeight, SWP_NOZORDER);
				RedrawWindow(WindowSystem::GetInstance().main, 0, 0, RDW_ALLCHILDREN | RDW_INVALIDATE);
				}

				if (hWnd == WindowSystem::GetInstance().op){
					const size_t width = LOWORD(lParam), height = HIWORD(lParam);
					WindowSystem::GetInstance().resizeSignal(width, height);
				}
				lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
				break;

			case WM_GETMINMAXINFO:
				if (hWnd == WindowSystem::GetInstance().main){
					((MINMAXINFO *)lParam)->ptMinTrackSize.x = 800;
					((MINMAXINFO *)lParam)->ptMinTrackSize.y = 600;
				}
				break;
			case WM_CLOSE:case WM_DESTROY:
				WindowSystem::GetInstance().closeWindowSignal();
				break;
			
				case WM_KEYDOWN:
					WindowSystem::GetInstance().inputSignal(0,wParam,1);

				break;

			case WM_KEYUP:
				WindowSystem::GetInstance().inputSignal(0, wParam, 0);
				break;
				
			case WM_LBUTTONDOWN:
				SetFocus(WindowSystem::GetInstance().main);
				WindowSystem::GetInstance().inputSignal(1, 0, 1);
				SetCapture(WindowSystem::GetInstance().op);
				break;
			case WM_LBUTTONUP:
				WindowSystem::GetInstance().inputSignal(1, 0, 0);
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				SetFocus(WindowSystem::GetInstance().main);
				WindowSystem::GetInstance().inputSignal(1, 1, 1);
				break;
			case WM_RBUTTONUP:
				WindowSystem::GetInstance().inputSignal(1, 1, 0);
				break;
			case WM_MOUSEMOVE:
			{POINTS ptsEnd;
			ptsEnd = MAKEPOINTS(lParam);
			WindowSystem::GetInstance().inputSignal(2, ptsEnd.x, ptsEnd.y); }
				break;
			case WM_MOUSEWHEEL:
			{int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			WindowSystem::GetInstance().inputSignal(3, 0, zDelta);
			}
				break;

			case WM_COMMAND:
				if (LOWORD(wParam)==ID_FILE_EXIT) WindowSystem::GetInstance().closeWindowSignal();
				break;
			case WM_KILLFOCUS:
				WindowSystem::GetInstance().outOfFocus();
				lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
				break;
			default:
				lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
				break;
			}

			return lRet;
		}


		void WindowSystem::SetDropList(std::vector<std::string> const& v,int id) {
			for (unsigned z = 0; z < v.size(); ++z) {
				SendDlgItemMessage(buttonsStructure, id, CB_INSERTSTRING, z, (LPARAM)v[z].c_str());
			}
			SendDlgItemMessage(buttonsStructure, id, CB_SETCURSEL, 0, (LPARAM)0);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Dialog Procedure
		// It must return true if the message is handled.
		///////////////////////////////////////////////////////////////////////////////
		INT_PTR CALLBACK dialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			WindowSystem::GetInstance().uiEvent(hwnd, msg, wParam, lParam);
			switch (msg)
			{
				break;
			case WM_HSCROLL:{
				int value = SendDlgItemMessage(hwnd, IDC_SLIDER1, TBM_GETPOS, 0, 0);

			}
				break;
			case WM_COMMAND:
				if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam)== IDC_COMBO1)
				{
					WindowSystem::GetInstance().selectedAnimation = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				}
				return true;

			case WM_PAINT:
				::DefWindowProc(hwnd, msg, wParam, lParam);
				return true;

			case WM_KEYDOWN:
				WindowSystem::GetInstance().inputSignal(0, wParam, 1);
				break;

			case WM_KEYUP:
				WindowSystem::GetInstance().inputSignal(0, wParam, 0);
				break;

			case WM_MOUSEMOVE: case WM_TIMER: case WM_LBUTTONUP: case WM_CONTEXTMENU:
				return true;
			}
			return false;
		}

}
