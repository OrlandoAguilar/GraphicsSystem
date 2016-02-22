#pragma once
#include <string>
#include "Callable.h"
#include <vector>

namespace GE{
	namespace Graphics{

		class Texture2D 
		{
		public:
			typedef siglot::Callable<unsigned int, unsigned char*, int, int, int, int> tdelegate;
			static unsigned int Create(std::string const&,  tdelegate&);


		protected:
		//	unsigned char *LoadTGA(std::string const & filename, unsigned int & width, unsigned int & height, int &iComponents, int &eFormat);
			//static std::vector<unsigned char> LoadPNG(std::string const & filename, unsigned int & width, unsigned int & height, int &iComponents, int &eFormat);
		};
	}
}

