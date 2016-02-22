#include "VQS.h"
namespace GE {
	const VQS VQS::Identity;

	VQS::VQS() :s(1.0f) {}

	/*Creates a VQS from a mat4*/
	VQS::VQS(glm::mat4 const &m)
	{
		v = glm::vec3(m[0][3], m[1][3], m[2][3]);	
		q = Quaternion(m).Normalize();
		s = 1.0f;		//for my models, this is enogh
	}
	VQS::VQS(glm::vec3 const& _v, Quaternion const& _q, float _s) : v(_v), q(_q), s(_s) {}

	/*Concatenated two VQS*/
	VQS VQS::operator *(VQS const & vqs2) const {
		VQS result;
		VQS const & vqs1 = *this;
		result.v = vqs1*vqs2.v;
		result.q = vqs1.q*vqs2.q;
		result.s = vqs1.s*vqs2.s;
		return result;
	}

	/*Trnasforms vector vec using this VQS*/
	glm::vec3 VQS::operator *(glm::vec3 const & vec) const {
		return q.Rotate(vec*s) + v;
	}

	/*Transform this VQS to Mat4*/
	glm::mat4 VQS::ToMat4() const {
		glm::mat3 rots = q.ToMat3()*s;
		glm::mat4 transform(rots);
		transform[3][0] = v.x;
		transform[3][1] = v.y;
		transform[3][2] = v.z;
		return transform;
	}

	/*Interpolates two VQS*/
	VQS VQS::Interpolate(VQS const& q1, VQS const& q2, float t) {
		auto v = Lerp(q1.v, q2.v, t);
		float s = Elerp(q1.s, q2.s, t);
		auto q = Quaternion::SLerp(q1.q, q2.q, t);
		return VQS(v, q, s);
	}

	/*Incremental interpolation for Slerp*/
	VQS VQS::InterpolateIncremental(VQS const & q1, VQS const & q2, float t, VQS const & iqc,int n)
	{
		int k = (int)(t / n);
		float s = q1.s;
		Quaternion qk = q1.q;
		for (int i = 1; i < k; ++i) {		//the best would be to avoid the for and save the last qk and s
			qk = iqc.q*qk;
			s = s*iqc.s;
		}
		glm::vec3 v = q1.v + static_cast<float>(k)*iqc.v;

		return VQS(v, qk, s);
	}

	/*Incremental interpolation for Slerp using Chevyshev*/
	VQS VQS::InterpolateIncrementalChevyshev(VQS const & q1, VQS const & q2, float t)
	{
		auto v = Lerp(q1.v, q2.v, t);
		float s = Elerp(q1.s, q2.s, t);
		auto q = Quaternion::ISLerpCheby(q1.q, q2.q, 2, t);
		return VQS(v, q, s);
	}

	/*Extracts vc, qc and sc for the incremental interpolation*/
	VQS VQS::ExtFactIncInterpol(VQS const & vqs1, VQS const & vqs2,int n)
	{
		const float nf = (float)n;
		glm::vec3 vc = (vqs2.v - vqs1.v) / nf;
		float sc = Elerp(vqs1.s, vqs2.s, 1.0f / nf);
		auto qc = Quaternion::ExtractISLerpPowFactor(vqs1.q, vqs2.q, nf);
		return VQS(vc,qc,sc);
	}
}