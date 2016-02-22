#pragma once
#include "Delegate.h"
#include <vector>
class Cone
{
public:
	typedef siglot::Callable<unsigned int, std::vector<float>&, std::vector<unsigned int>&> tdelegate;
	static void Create(float radius, int sectors, tdelegate &);
};

