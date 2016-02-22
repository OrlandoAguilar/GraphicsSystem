
#pragma once
#include "Debug.h"
#include "Delegate.h"
#include <vector>
namespace GE{
		namespace Graphics{
			class Sphere {

				
			public:
				typedef siglot::Callable<unsigned int, std::vector<float>&, std::vector<unsigned int>&> tdelegate;
				static void Create(float radius, int rings, int sectors, tdelegate &);

				
			};
	}
}