#pragma once
#include <Importer.hpp>      // C++ importer interface
#include <glm/gtc/matrix_transform.hpp>
#include "Quaternion.h"

namespace GE {
	// constructor from Assimp matrix
	glm::mat4 Convert(const aiMatrix4x4& AssimpMatrix);

	glm::mat4 Convert(const aiMatrix3x3& AssimpMatrix);

	Quaternion Convert(const aiQuaternion& q);

	glm::vec3 Convert(const aiVector3D& v);
}
