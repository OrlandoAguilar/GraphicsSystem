#include <windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/wglew.h>

#include "Debug.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

void CreateConsole(){
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_err = GetStdHandle(STD_ERROR_HANDLE);
	hCrt = _open_osfhandle((long)handle_err, _O_TEXT);
	FILE* hf_err = _fdopen(hCrt, "w");
	setvbuf(hf_err, NULL, _IONBF, 1);
	*stderr = *hf_err;


	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;
}

void EnableMemoryLeakChecking(int breakAlloc)
{
	/*	//Set the leak checking flag
	int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);

	//If a valid break alloc provided set the breakAlloc
	if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);*/
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
}



int printOglError(char *file, int line){

	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		DBOUT("glError in file " << file << " line " << line << " error " << ((char*)gluErrorString(glErr)) << "\n");
		CriticalBreakPoint();
		retCode = 1;
	}
	return retCode;
}
