#pragma once
#include <GLM\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GE{
	class Quaternion {
	public:
		float w;
		glm::vec3 v;

		Quaternion(float _w, glm::vec3 const& _v):w(_w),v(_v) {}
		Quaternion() :w(1.0f), v(0,0,0) {}
		Quaternion(glm::vec3 const& _v) :w(0.0f),v(_v) {}
		Quaternion(float _w,float x, float y, float z):
			w(_w),v(x,y,z){}

		explicit Quaternion(glm::mat3 const &);
		explicit Quaternion(glm::mat4 const &);

		static Quaternion FromRotation(float angle, glm::vec3 const&v);

		static Quaternion QuaternionRotation(glm::vec3 const & v1, glm::vec3 const & v2);

		Quaternion operator + (Quaternion const& q) const;

		Quaternion operator - (Quaternion const& q) const;

		Quaternion operator * (Quaternion const& q) const;

		Quaternion operator * (float c) const;

		friend Quaternion operator/ (Quaternion const & q, float c);

		friend Quaternion operator * (float c, Quaternion const & q);

		float Dot(Quaternion const& q) const;

		float Length() const;

		Quaternion Inverse() const;

		Quaternion Complement() const;

		Quaternion operator ~() const;

		Quaternion& Negate();

		glm::vec3 Rotate(glm::vec3 const& r) const;

		static glm::vec3 Rotate(float angle, glm::vec3 const& A, glm::vec3 const& r);

		glm::mat3 ToMat3() const;

		glm::mat4 ToMat4() const;

		

		Quaternion& Normalize();

		static Quaternion Normalize(Quaternion q);

		static Quaternion ToQuaternion(glm::mat3 const& m);

		static Quaternion SLerp(Quaternion  const & q1, Quaternion  q2, float t);		//in some cases, q2 needs to be modified

		static Quaternion ISLerp(Quaternion  q1, Quaternion  q2, float n,float t);
		static Quaternion ISLerpCheby(Quaternion  q1, Quaternion  q2, float n, float t);

		static Quaternion ExtractISLerpPowFactor(Quaternion q0, Quaternion qn, float n);
		
	};

	

}