#pragma once

#include <GL/glew.h>
#include <string>
namespace GE{
	namespace Graphics{

		class Shader{

		public:
			enum type{VGF,VF};
			Shader(std::string const & , std::string const & );
			Shader(std::string const &,type t= Shader::type::VF);
			~Shader();

			void 	SetShaderActive(bool);

			void SetUniform(std::string const & , float);
			void SetUniform(std::string const & , int );
			void SetUniform(std::string const & , float , float );
			void SetUniform(std::string const & , float , float , float );
			void SetUniform1fv(std::string const & , int , float const* );
			void SetUniform3fv(std::string const & , int , float const * );
			void SetUniform4fv(std::string const & , int , float const * );

			void SetUniform4mv(std::string const &, float const *);
			void SetVertexAttrib(std::string const &, float, float, float);

			GLhandleARB 	shader;

		private:
			void LoadShader(std::string const &, std::string const &);
			std::string LoadFile(std::string const &);
			bool FailedShaderOperation(GLhandleARB program);
			bool FailedProgramOperation(GLhandleARB program);
			GLhandleARB CompileShader(std::string const & vertexFile, unsigned shaderType);
		};


	}
}