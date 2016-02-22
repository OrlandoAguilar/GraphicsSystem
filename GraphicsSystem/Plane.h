#pragma once
#include "Delegate.h"
#include <vector>

class Plane{
public:
	typedef siglot::Callable<unsigned int, std::vector<float>&, std::vector<unsigned int>&> tdelegate;
	static void Create(float width, float height, tdelegate &);
private:
};