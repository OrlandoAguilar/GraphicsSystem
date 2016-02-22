#pragma once

#include <GLM\glm.hpp>
#include <PointParameters.h>
#include "DataType.h"

namespace GE {
	class Interpolate {
	public:

		//public wrapper for interpolation, regardless the algorithm
		static glm::vec3 Interpolation(PointParameters const & param, real t);

	private:
		//Bernstain interpolation
		static glm::vec3 Bernstain(PointParameters const& param, real t);

		//Catmull rom interpolation
		static glm::vec3 CatmullRom(PointParameters const& param, float t);

	};
}
