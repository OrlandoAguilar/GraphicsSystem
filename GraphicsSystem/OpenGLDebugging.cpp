///////////////////////////////////////////////////////////////////////////////////////
/// All content (c) 2015 Digipen (USA) Corporation, all rights reserved.
/// @file OpenGLDebugging.cpp
/// @date 9/11/2015
/// @author Orlando Aguilar Vivanco <orlando.aguilar@digipen.edu>
///////////////////////////////////////////////////////////////////////////////////////

#include "OpenGLDebugging.h"
#include "Debug.h"
#include <assert.h>
namespace GE {


	// aux function to translate source to string
	std::string
		GetStringForSource(GLenum source) {

		switch (source) {
		case GL_DEBUG_SOURCE_API_ARB:
			return("API");
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
			return("Window System");
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
			return("Shader Compiler");
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			return("Third Party");
		case GL_DEBUG_SOURCE_APPLICATION_ARB:
			return("Application");
		case GL_DEBUG_SOURCE_OTHER_ARB:
			return("Other");
		default:
			return("");
		}
	}

	// aux function to translate severity to string
	std::string
		GetStringForSeverity(GLenum severity) {

		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH_ARB:
			return("High");
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			return("Medium");
		case GL_DEBUG_SEVERITY_LOW_ARB:
			return("Low");
		default:
			return("");
		}
	}

	// aux function to translate type to string
	std::string
		GetStringForType(GLenum type) {

		switch (type) {
		case GL_DEBUG_TYPE_ERROR_ARB:
			return("Error");
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			return("Deprecated Behaviour");
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			return("Undefined Behaviour");
		case GL_DEBUG_TYPE_PORTABILITY_ARB:
			return("Portability Issue");
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			return("Performance Issue");
		case GL_DEBUG_TYPE_OTHER_ARB:
			return("Other");
		default:
			return("");
		}
	}

	void __stdcall
		ETB_GL_ERROR_CALLBACK(GLenum        source,
		GLenum        type,
		GLuint        id,
		GLenum        severity,
		GLsizei,
		const GLchar* message,
		GLvoid*)
	{

		DBOUT("OpenGL Error:\n == == == == == == = \n" << GetStringForType(type).c_str() << "; Source: " <<
			GetStringForSource(source).c_str() << "; ID: " << id << "; Severity: " <<
			GetStringForSeverity(severity).c_str() << "\n" << message << "\n");

		assert(severity != GL_DEBUG_SEVERITY_HIGH_ARB);

	}

}