#include "Interpolate.h"

namespace GE {

	/*Bernstain interpolation. I dont like the results, therefore I will use catmull rom.*/
	glm::vec3 Interpolate::Bernstain(PointParameters const & p, real t)
	{
		real ft = 1 - t;
		real ft2 = ft*ft;
		real ft3 = ft2*ft;

		real t2 = t*t;
		real t3 = t2*t;

		return (p.p0*(float)ft3) + (3.0f * p.p1*(float)(ft2*t)) + (3.0f * p.p2*(float)(ft*t2)) + (p.p3*(float)t3);
	}

	glm::vec3 Interpolate::Interpolation(PointParameters const & p, real t)
	{
		//return Bernstain(PointParameters(p.p1, p.p1 + 0.5f*(p.p2 - p.p0), p.p2 - 0.5f*(p.p3 - p.p1), p.p2), t);
		return CatmullRom(p, (float)t);
	}

	//catmull rom interpolation
	glm::vec3 Interpolate::CatmullRom(PointParameters const & param, float t)
	{
		//points of interpolation
		const auto & pi_2 = param.p0;
		const auto & pi_1 = param.p1;
		const auto & pi = param.p2;
		const auto & pi_p1 = param.p3;

		float tau = 0.8f;	//smoothness of curve

		//interpolation point
		return pi_1 +
			tau*t*(pi - pi_2) +
			(-3.0f * pi_1 + 3.0f * pi - 2 * tau*(pi - pi_2) - tau*(pi_p1 - pi_1))*t*t +
			(-tau*pi_2 + (2 - tau)*pi_1 + (tau - 2.0f)*pi + tau*pi_p1)*t*t*t;
	}
}