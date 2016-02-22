#include "Quaternion.h"
namespace GE {

	/*Creates a quaternion from an angle and a vector*/
	Quaternion Quaternion::FromRotation(float angle, glm::vec3 const&v) {
		float t_2 = angle / 2.0f;
		return Quaternion(::cosf(t_2), ::sin(t_2)*v);
	}

	/*From two given orientations, buils a quaternion that transforms from v1 to v2*/
	Quaternion Quaternion::QuaternionRotation(glm::vec3 const & v1, glm::vec3 const & v2) {
		Quaternion q;
		q.v = glm::cross(v1, v2);
		float lv1 = glm::length(v1), lv2 = glm::length(v2);
		q.w = sqrt(lv1 * lv2) + glm::dot(v1, v2);
		return q;
	}

	Quaternion Quaternion::operator + (Quaternion const& q) const {
		return Quaternion(q.w + w, q.v + v);
	}

	Quaternion Quaternion::operator - (Quaternion const& q) const {
		return Quaternion(q.w - w, q.v - v);
	}

	/*Quaternion multiplication*/
	Quaternion Quaternion::operator * (Quaternion const& q) const {
		return Quaternion(q.w * w - glm::dot(q.v, v), w*q.v + q.w*v + glm::cross(v, q.v));
	}

	Quaternion Quaternion::operator * (float c) const {
		return Quaternion(c*w, c*v);
	}

	
	Quaternion operator/ (Quaternion const & q, float c) {
		return Quaternion(q.w / c, q.v / c);
	}

	Quaternion operator * (float c, Quaternion const & q) {
		return Quaternion(c*q.w, c*q.v);
	}

	/*Dot product of two quaternions*/
	float Quaternion::Dot(Quaternion const& q) const {
		return q.w*w + glm::dot(q.v, v);
	}

	/*Magnitud of quaternion*/
	float Quaternion::Length() const {
		return ::sqrt(w*w + glm::dot(v, v));
	}

	/*Inverse of the quaternion*/
	Quaternion Quaternion::Inverse() const {
		return Complement() / Length();
	}

	Quaternion Quaternion::Complement() const {
		return Quaternion(w, -v);
	}
	Quaternion Quaternion::operator ~() const {
		return Complement();
	}

	/*Negates the quaternrnion (necessary for slerp)*/
	Quaternion& Quaternion::Negate() {
		w = -w;
		v = -v;
		return *this;
	}

	/*Rotates vector r using the current quaternion*/
	glm::vec3 Quaternion::Rotate(glm::vec3 const& r) const {
		return (w*w - glm::dot(v, v))* r + 2 * glm::dot(v, r)*v + 2 * w*glm::cross(v, r);
	}

	/*Rotates r using the axis A angle degrees (uses angle axis notation)*/
	glm::vec3 Quaternion::Rotate(float angle, glm::vec3 const& A, glm::vec3 const& r) {
		float c = ::cosf(angle);
		float s = ::sinf(angle);
		return c*r + (1 - c)*glm::dot(A, r)*A + s*glm::cross(A, r);
	}

	/*Normalizes the quaternion*/
	Quaternion& Quaternion::Normalize() {
		float l = Length();
		w /= l;
		v /= l;
		return *this;
	}

	Quaternion Quaternion::Normalize(Quaternion q) {
		return q.Normalize();
	}

	/*Builds a quaternion from the matrix3 m*/
	Quaternion Quaternion::ToQuaternion(glm::mat3 const& m) {
		return Quaternion(m);
	}

	/*Builds a quaternion from the matrix3 m*/
	Quaternion::Quaternion(glm::mat3 const &m)
	{
		float s = 0.5f*::sqrtf(m[0][0] + m[1][1] + m[2][2] + 1.0f);
		float x = (m[1][2] - m[2][1]) / 4.0f*s;
		float y = (m[2][0] - m[0][2]) / 4.0f*s;
		float z = (m[0][1] - m[1][0]) / 4.0f*s;

		w = s; v = glm::vec3(x, y, z);
	}

	/*Builds a quaternion from the matrix4 m*/
	Quaternion::Quaternion(glm::mat4 const &m)
	{
		float s = 0.5f*::sqrtf(m[0][0] + m[1][1] + m[2][2] + 1.0f);
		float x = (m[1][2] - m[2][1]) / 4.0f*s;
		float y = (m[2][0] - m[0][2]) / 4.0f*s;
		float z = (m[0][1] - m[1][0]) / 4.0f*s;

		w = s; v = glm::vec3(x, y, z);
	}

	/*Spherical interpolation*/
	Quaternion Quaternion::SLerp(Quaternion  const & q0, Quaternion  q1, float t) {
		assert(t >= 0 && t <= 1);
		float cosine=q0.Dot(q1);

		if (cosine < 0.0f) {		//if cosine is lesser than zero, invert signs otherwise it will interpolate by the longest path
			cosine = -cosine;
			q1.Negate();
		}

		if (cosine >= 1.0f) {		//float point problems can give a value greather than 1.0. 
			return q0;			//Besides, if the value is 1.0, that means the quaternions are the same, so I return the second one.
		}

		float a = acos(cosine);
		float sa = sin(a);
		float ta = t*a;
		auto q= (sin(a - ta)*q0 + sin(ta)*q1) / sa;
		assert(q.w == q.w);		//nan check, I had a lot of problems with this during my implementation
		return q;
	}

	/*Transforms the quaternionr to a mat3*/
	glm::mat3 Quaternion::ToMat3() const {
		float length = Length();

		assert(length > 0.9f && length < 1.1f);
		glm::mat3 m;
		m[0][0] = 1.0f - 2.0f * (v.y*v.y + v.z*v.z);
		m[0][1] = 2.0f * (v.x*v.y + w*v.z);
		m[0][2] = 2.0f * (v.x*v.z - w*v.y);

		m[1][0] = 2.0f*(v.x*v.y - w*v.z);
		m[1][1] = 1.0f - 2.0f * (v.x*v.x + v.z*v.z);
		m[1][2] = 2.0f * (v.y*v.z + w*v.x);

		m[2][0] = 2.0f * (v.x*v.z + w*v.y);
		m[2][1] = 2.0f * (v.y*v.z - w*v.x);
		m[2][2] = 1.0f - 2.0f * (v.x*v.x + v.y*v.y);
		return m;
	}

	/*transforms the quaternion to a mat4*/
	glm::mat4 Quaternion::ToMat4() const {
#ifdef _DEBUG
		//float length = Length();
		//if (!(length > 0.9f && length < 1.1f))
		//	throw "e";
		//assert(length > 0.9f && length < 1.1f);
#endif


		glm::mat4 m;
		m[0][0] = 1.0f - 2.0f * (v.y*v.y + v.z*v.z);
		m[0][1] = 2.0f * (v.x*v.y + w*v.z);
		m[0][2] = 2.0f * (v.x*v.z - w*v.y);

		m[1][0] = 2.0f*(v.x*v.y - w*v.z);
		m[1][1] = 1.0f - 2.0f * (v.x*v.x + v.z*v.z);
		m[1][2] = 2.0f * (v.y*v.z + w*v.x);

		m[2][0] = 2.0f * (v.x*v.z + w*v.y);
		m[2][1] = 2.0f * (v.y*v.z - w*v.x);
		m[2][2] = 1.0f - 2.0f * (v.x*v.x + v.y*v.y);
		return m;
	}

	/*Extracts the qc for power incremental interpolation of quaternions*/
	Quaternion Quaternion::ExtractISLerpPowFactor(Quaternion q0, Quaternion qn, float n)
	{
		float alpha = acosf(q0.Dot(qn));
		float beta = alpha / n;
		auto u = (q0.w*qn.v - qn.w*q0.v + glm::cross(q0.v, qn.v)) / sinf(alpha);
		return Quaternion(cosf(beta), sinf(beta)*u);
	}

	/*I am checking the ISlerp here, but normally the implementation 
	requieres to store initial values rather than calculate them and 
	use the for every time. This implementation is only to see the 
	results*/
	Quaternion Quaternion::ISLerp(Quaternion q0, Quaternion qn, float n, float t)
	{
		int k = (int)(t / n);
		if (k == 0)
			return q0;
		float alpha = acosf(q0.Dot(qn));
		float beta = alpha / n;
		auto u = (q0.w*qn.v - qn.w*q0.v + glm::cross(q0.v,qn.v)) / sinf(alpha);
		auto qc = Quaternion(cosf(beta), sinf(beta)*u);
		Quaternion qk = q0;
		for (int i = 1; i<k; ++i)
			qk = qc*qk;

		return qk;
	}

	/*Another implementation of incremental slerp but using the Chebyshev approach
	just to test the interpolation on the project*/
	Quaternion Quaternion::ISLerpCheby(Quaternion q0, Quaternion qn, float n, float t)
	{
		int k = (int)(t / n);
		if (k == 0)
			return q0;
		float alpha = acosf(q0.Dot(qn));
		float beta = alpha / n;
		float A = 2*cosf(beta);
		auto uq0 = ((qn - cosf(alpha)*q0) / sin(alpha)).Normalize();
		auto qk_1 = q0;
		auto qk = cosf(beta)*q0 + sinf(beta) *uq0;
		for (int i = 2; i < n; ++i) {
			auto qtmp = qk;
			qk = A*qk - qk_1; // Chebyshev recurrence
			qk_1 = qtmp;
		}

		float length = qk.Length();
		assert(length > 0.9f && length < 1.1f);

		return qk.Normalize();
	}

}