#pragma once
#include "Quaternion.h"
#include <GLM\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Interpolation.h"

namespace GE {
	class VQS {
	public:
		float s;
		glm::vec3 v;
		Quaternion q;

		VQS();
		VQS(glm::mat4 const&);
		VQS(glm::vec3 const& _v, Quaternion const& _q, float _s);
		VQS operator *(VQS const & vqs2) const;
		glm::vec3 operator *(glm::vec3 const & vec) const;
		glm::mat4 ToMat4() const;
		static VQS Interpolate(VQS const& q1, VQS const& q2, float t);
		static VQS InterpolateIncremental(VQS const& q1, VQS const& q2, float t, VQS const & iqc,int n);
		static VQS InterpolateIncrementalChevyshev(VQS const& q1, VQS const& q2, float t);

		static VQS ExtFactIncInterpol(VQS const& q1, VQS const& q2,int n);

		static const VQS Identity;
	};
}