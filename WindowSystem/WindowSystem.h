#pragma once

#include <windows.h> 
#include "System.h"
#include "GESignal.h"
#include <vector>

namespace GE{
		class WindowSystem :
			public System
		{
		public:
			
			static WindowSystem& GetInstance();

			
			void Release();
			void Start(std::string const & title, int width, int height);
			
			int GetSelectedAnimation()const;

			virtual void Update(double);

			siglot::Signal<int,int> resizeSignal;
			siglot::Signal<> closeWindowSignal;
			siglot::Signal<int, int, int> inputSignal;
			siglot::Signal<> outOfFocus,refresh;

			siglot::Signal<HWND, UINT,WPARAM, LPARAM> uiEvent;

			HWND getWindowHandler()const;
			HWND GetDownUIHandler()const;
			void SetDropList(std::vector<std::string> const & v,int id);
		private:
			friend LONG WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
			friend INT_PTR CALLBACK dialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			
			HWND main;
			HWND op;
			HWND buttonsStructure;
			HWND hWnd;											//The handle to the game window
			HINSTANCE hInstance;								//The handle to the instance
			std::string m_title;
			WindowSystem(WindowSystem const&)=delete;
			WindowSystem();
			virtual ~WindowSystem();
			int selectedAnimation;
		};
}
