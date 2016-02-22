#pragma once

#include <GLM\glm.hpp>
#include <vector>
#include <algorithm>

/*Structure to pass the points for interpolation in the Intepolate class*/
struct PointParameters {
	PointParameters(glm::vec3 const& _p0, glm::vec3 const& _p1, glm::vec3 const& _p2, glm::vec3 const& _p3) :
		p0(_p0), p1(_p1), p2(_p2), p3(_p3) {}

	/*This contructor allows for a convenient way of taking the points for catmull rom 
	inerpolation by passing the whole list of control points and the index of the lowest 
	point for the pair of interpolation points*/
	PointParameters(std::vector<glm::vec3> const & p, unsigned index) {
		int const pLast = p.size() - 1;

		assert(index >= 0u && index <= (unsigned)pLast);

		//from the input list, chooses the index of the points to interpolate in the correct range
		int pi = index;
		int pi_1 = std::max(0, pi - 1);
		int pi_p1 = std::min(pLast, pi + 1);
		int pi_p2 = std::min(pLast, pi + 2);

		//stores those points in the structure
		p0 = p[pi_1];
		p1 = p[pi];
		p2 = p[pi_p1];
		p3 = p[pi_p2];
	}

	glm::vec3 p0;
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 p3;
};
