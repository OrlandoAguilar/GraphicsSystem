
//#include <GL/glext.h>
#define NOMINMAX
#include "Shader.h"
#include <stdio.h>
#include <stdlib.h>
#include "Exception.h"
#include <fstream>
#include "Debug.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace GE{
	namespace Graphics{
		Shader::Shader(std::string const & fragmentName, std::string const & vertexName){
			LoadShader(vertexName, fragmentName);
		}


		Shader::Shader(std::string const &shaderName, type t) {
			const unsigned MAX = std::numeric_limits<unsigned>::max();
			std::string shaderFile;
			GLhandleARB shaderHan[3] = { MAX,MAX,MAX };
			int shaderIndex = 0;
			switch (t)
			{
			case GE::Graphics::Shader::VGF:
				shaderFile = LoadFile(shaderName + ".geom");
				shaderHan[shaderIndex++] = CompileShader(shaderFile, GL_GEOMETRY_SHADER);
			case GE::Graphics::Shader::VF:
				shaderFile = LoadFile(shaderName + ".vert");
				shaderHan[shaderIndex++] = CompileShader(shaderFile, GL_VERTEX_SHADER);

				shaderFile = LoadFile(shaderName + ".frag");
				shaderHan[shaderIndex++] = CompileShader(shaderFile, GL_FRAGMENT_SHADER);
				break;
			default:
				throw;
			}
			

			// Linking the shaders
			shader = glCreateProgramObjectARB();
			for (int z = 0; z < shaderIndex; ++z) {
				glAttachObjectARB(shader, shaderHan[z]);
			}

			glLinkProgramARB(shader);

			if (FailedProgramOperation(shader)) {
				//We don't need the program anymore.
				glDeleteProgram(shader);
				for (int z = 0; z < shaderIndex; ++z) {
					glDeleteShader(shaderHan[z]);
				}
				throw ShaderException("Linking error");
			}

			for (int z = 0; z < shaderIndex; ++z) {
				glDetachShader(shader, shaderHan[z]);
			}

		}


		std::string Shader::LoadFile(std::string const & file){
			std::ifstream stream(file.c_str());
			std::ostringstream content;

			if (!stream.is_open())
				throw IncompleteOperationException("The file could not be open");

			std::string line;
				
			while (stream.good()){
				getline(stream,line);
				content<< line.c_str() <<"\n";
			}
			return content.str();
		}

		void Shader::LoadShader(std::string const & vertexName, std::string const & fragmentName){

				//Loading and creating the shaders
				std::string vertexFile, fragmentFile;
				vertexFile = LoadFile(vertexName);
				fragmentFile = LoadFile(fragmentName);
				GLhandleARB 	vertexShader;
				GLhandleARB 	fragmentShader;
				vertexShader = (GLhandleARB)glCreateShader(GL_VERTEX_SHADER);
				fragmentShader = (GLhandleARB)glCreateShader(GL_FRAGMENT_SHADER);

				const char* vShad = vertexFile.c_str();
				const char* fShad = fragmentFile.c_str();

				GLint length = vertexFile.length();
				glShaderSourceARB(vertexShader, 1, &vShad, &length);
				length = fragmentFile.length();
				glShaderSourceARB(fragmentShader, 1, &fShad, &length);

				// Compiling  Shaders

				glCompileShaderARB(vertexShader);

				if (FailedShaderOperation(vertexShader)){
					//Don't leak shaders either.
					glDeleteShader(vertexShader);
					throw ShaderException("The vertex shader could not be compiled");
				}

				// ------------------------------------
				// try to compile "fragment shader"
				glCompileShaderARB(fragmentShader);

				if (FailedShaderOperation(fragmentShader)){
					//Don't leak shaders either.
					glDeleteShader(vertexShader);
					glDeleteShader(fragmentShader);
					throw ShaderException("The fragment shader could not be compiled");
				}
					

				// Linking the shaders
				shader = glCreateProgramObjectARB();
				glAttachObjectARB(shader, vertexShader);
				glAttachObjectARB(shader, fragmentShader);
				glLinkProgramARB(shader);

				if (FailedProgramOperation(shader)){
					//We don't need the program anymore.
					glDeleteProgram(shader);
					//Don't leak shaders either.
					glDeleteShader(vertexShader);
					glDeleteShader(fragmentShader);
					throw ShaderException("Linking error");
				}
				

				glDetachShader(shader, vertexShader);
				glDetachShader(shader, fragmentShader);

		}

		bool Shader::FailedShaderOperation(GLhandleARB program){
			
			GLint isLinked = 0;
			glGetShaderiv(program, GL_COMPILE_STATUS, (int *)&isLinked);
			if (isLinked == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(program, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength+1);
				glGetShaderInfoLog(program, maxLength, &maxLength, &infoLog[0]);
				DBOUT("" << &infoLog[0]<<"\n");
				//In this simple program, we'll just leave
				return true;
			}

			return false;
				
		}

		bool Shader::FailedProgramOperation(GLhandleARB program){

			GLint isLinked = 0;
			glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
			if (isLinked == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength + 1);
				glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

				//In this simple program, we'll just leave
				return true;
			}

			return false;

		}

		GLhandleARB Shader::CompileShader(std::string const &vertexFile,unsigned shaderType)
		{
			GLhandleARB 	shader;
			shader = (GLhandleARB)glCreateShader(shaderType);

			const char* vShad = vertexFile.c_str();

			GLint length = vertexFile.length();
			glShaderSourceARB(shader, 1, &vShad, &length);

			// Compiling  Shaders

			glCompileShaderARB(shader);

			if (FailedShaderOperation(shader)) {
				//Don't leak shaders either.
				glDeleteShader(shader);
				throw ShaderException("The shader could not be compiled");
			}
			return shader;
		}

		Shader::~Shader(){
			/*if (vertexShader){
				glDetachObjectARB(shader, vertexShader);
				glDeleteObjectARB(vertexShader);
				vertexShader = NULL;
			}

			if (fragmentShader){
				glDetachObjectARB(shader, fragmentShader);
				glDeleteObjectARB(fragmentShader);
				fragmentShader = NULL;
			}*/

			if (shader){
				glDeleteObjectARB(shader);
				shader = NULL;
			}
		}

		void 	Shader::SetShaderActive(bool b){
				if (b){
					glUseProgramObjectARB(shader);
				}
				else {
					glUseProgramObjectARB(0);
				}
		}

		void 	Shader::SetUniform(std::string const & name, float value){
			glUniform1fARB(glGetUniformLocationARB(shader, name.c_str()), value);
		}

		void 	Shader::SetUniform(std::string const & name, int value){
			glUniform1iARB(glGetUniformLocationARB(shader, name.c_str()), value);
		}
		void 	Shader::SetUniform(std::string const & name, float value, float value2){
			glUniform2fARB(glGetUniformLocationARB(shader, name.c_str()), value, value2);
		}
		void 	Shader::SetUniform(std::string const & name, float value, float value2, float value3){
			glUniform3fARB(glGetUniformLocationARB(shader, name.c_str()), value, value2, value3);
		}
		void    Shader::SetUniform1fv(std::string const & name, int count, float const * value){
			glUniform1fvARB(glGetUniformLocationARB(shader, name.c_str()), count, value);
		}
		void    Shader::SetUniform3fv(std::string const & name, int count, float const * value){
			glUniform3fvARB(glGetUniformLocationARB(shader, name.c_str()), count, value);
		}
		void    Shader::SetUniform4fv(std::string const &name, int count, float const * value){
			glUniform4fvARB(glGetUniformLocationARB(shader, name.c_str()), count, value);
		}

		void Shader::SetUniform4mv(std::string const &name, float const * value){
			glUniformMatrix4fv(glGetUniformLocationARB(shader, name.c_str()), 1, GL_FALSE, value);
		}

		

		void Shader::SetVertexAttrib(std::string const & name, float v1, float v2, float v3){
			glVertexAttrib3fARB(glGetAttribLocationARB(shader, name.c_str()), v1, v2, v3);
		}
	}
}
