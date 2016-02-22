#pragma once
#include "DataType.h"
#include <utility>

namespace GE {
	class SpeedControl {
	public:
		static std::pair<real, real> SEasyInOut(real t, real t1, real t2);	//calculate the easy-in/easy-out velocity and integral (S) 
	};
}

