
#pragma once
#include "Debug.h"
#include "Delegate.h"
#include <vector>
namespace GE{
	namespace Graphics{

		class Cube {

		public:
			typedef siglot::Callable<unsigned int, std::vector<float>&, std::vector<unsigned int>&> tdelegate;
			static void Create(float size, tdelegate &);
		};
	}
}