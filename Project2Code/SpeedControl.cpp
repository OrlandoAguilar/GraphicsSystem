#include "SpeedControl.h"

namespace GE {

	//returns distance and velocity given the time, t1 and t2; Vc for the easy-in-out for velocity
	std::pair<real, real> SpeedControl::SEasyInOut(real time, real t1, real t2)
	{
		const real t = time - static_cast<int>(time);		//keeps the time in the range [0,1)

		real vc = 2 / (1 - t1 + t2);			//calculates vc for the current [t1,t2]


												//Picewise function. According to t, returns integral of v (S) and also v in this order;
		if (t < t1)
			return std::make_pair(((vc*t*t) / (2 * t1)), (vc*t) / t1);

		if (t < t2)
			return std::make_pair((vc*(t - t1*0.5)), vc);

		return std::make_pair(((vc*(t - t2)*(2 - t - t2)) / (2 * (1 - t2)) + vc*(t2 - 0.5*t1)), (vc*(1 - t)) / (1.0f - t2));
	}
}

