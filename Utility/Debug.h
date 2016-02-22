#pragma once
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <iostream>
#include <sstream>
//   fprintf ( stderr, "%s", os_.str().c_str() );
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

void CreateConsole();

//void EnableMemoryLeakChecking(int breakAlloc=-1);

void EnableMemoryLeakChecking(int breakAlloc = -1);

#define printOpenGLError() printOglError(__FILE__, __LINE__)

// Breakpoints that should ALWAYS trigger (EVEN IN RELEASE BUILDS) [x86]!
#ifdef _MSC_VER
# define CriticalBreakPoint() if (IsDebuggerPresent ()) __debugbreak ();
#else
# define CriticalBreakPoint() asm (" int $3");
#endif

int printOglError(char *file, int line);

//#define DBOUT( s ) 