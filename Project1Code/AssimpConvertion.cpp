#include "AssimpConvetion.h"
namespace GE {
	// constructor from Assimp matrix
	glm::mat4 Convert(const aiMatrix4x4& AssimpMatrix)
	{
		glm::mat4 m;
		m[0][0] = AssimpMatrix.a1; m[0][1] = AssimpMatrix.b1; m[0][2] = AssimpMatrix.c1; m[0][3] = AssimpMatrix.d1;
		m[1][0] = AssimpMatrix.a2; m[1][1] = AssimpMatrix.b2; m[1][2] = AssimpMatrix.c2; m[1][3] = AssimpMatrix.d2;
		m[2][0] = AssimpMatrix.a3; m[2][1] = AssimpMatrix.b3; m[2][2] = AssimpMatrix.c3; m[2][3] = AssimpMatrix.d3;
		m[3][0] = AssimpMatrix.a4; m[3][1] = AssimpMatrix.b4; m[3][2] = AssimpMatrix.c4; m[3][3] = AssimpMatrix.d4;
		return m;
	}

	glm::mat4 Convert(const aiMatrix3x3& AssimpMatrix)
	{
		glm::mat4 m;
		m[0][0] = AssimpMatrix.a1; m[0][1] = AssimpMatrix.b1; m[0][2] = AssimpMatrix.c1;
		m[1][0] = AssimpMatrix.a2; m[1][1] = AssimpMatrix.b2; m[1][2] = AssimpMatrix.c2;
		m[2][0] = AssimpMatrix.a3; m[2][1] = AssimpMatrix.b3; m[2][2] = AssimpMatrix.c3;
		return m;
	}

	Quaternion Convert(const aiQuaternion& q) {
		return Quaternion(q.w, q.x, q.y, q.z);
	}

	glm::vec3 Convert(const aiVector3D& v) {
		return glm::vec3(v.x, v.y, v.z);
	}
}