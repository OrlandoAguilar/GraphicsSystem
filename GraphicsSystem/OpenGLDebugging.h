///////////////////////////////////////////////////////////////////////////////////////
/// All content (c) 2015 Digipen (USA) Corporation, all rights reserved.
/// @file OpenGLDebugging.h
/// @date 9/11/2015
/// @author Orlando Aguilar Vivanco <orlando.aguilar@digipen.edu>
///////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/wglew.h>

namespace GE {
	void __stdcall
		ETB_GL_ERROR_CALLBACK(GLenum        source,
		GLenum        type,
		GLuint        id,
		GLenum        severity,
		GLsizei       length,
		const GLchar* message,
		GLvoid*       userParam);

	const char*	ETB_GL_DEBUG_SOURCE_STR(GLenum source);
	const char*	ETB_GL_DEBUG_TYPE_STR(GLenum type);
	const char*	ETB_GL_DEBUG_SEVERITY_STR(GLenum severity);
	DWORD ETB_GL_DEBUG_SEVERITY_COLOR(GLenum severity);

}