#include "Cone.h"


void Cone::Create(float radius,  int sectors, tdelegate & deleg){
	const float PI = 3.1415926535897f;
	const float PI_2 = 3.1415926535897f / 2;
	const int rings = 3;

	std::vector<unsigned int> indices;

	std::vector<float> data;

	data.resize(rings * sectors * 8);
	std::vector<float>::iterator d = data.begin();

	float const R = 1.0f / (float)(rings - 1);
	float const S = 1.0f / (float)(sectors - 1);
	int r, s;

	int index = -1;

	const float radius_2 = radius / 2.0f;

	r = 0;
	for (s = 0; s < sectors; ++s) {
		float x = cos(2 * PI * s * S);
		float z = sin(2 * PI * s * S);

		//vertices
		*d++ = 0.0f;
		*d++ = radius_2;
		*d++ = 0.0f;

		//normal
		*d++ = x / 1.4142f;
		*d++ = 1.0f / 1.4142f;
		*d++ = z / 1.4142f;

		//texture coordinates			
		*d++ = s * S;
		*d++ = r * R;
	}

	r = 1;
	for (s = 0; s < sectors; ++s) {
		float x = cos(2 * PI * s * S);
		float z = sin(2 * PI * s * S);

		//vertices
		*d++ = x * radius_2;
		*d++ = -radius_2;
		*d++ = z * radius_2;

		//normal
		*d++ = x;
		*d++ = 0.0f;
		*d++ = z;

		//texture coordinates			
		*d++ = s * S;
		*d++ = r * R;
	}

	r = 3;
	for (s = 0; s < sectors; ++s) {
		float x = cos(2 * PI * s * S);
		float z = sin(2 * PI * s * S);

		//vertices
		*d++ = 0;
		*d++ = -radius_2;
		*d++ = 0;

		//normal
		*d++ = 0;
		*d++ = -1;
		*d++ = 0;

		//texture coordinates			
		*d++ = s * S;
		*d++ = r * R;
	}

	indices.resize(rings * sectors * 6);
	std::vector<unsigned int>::iterator i = indices.begin();
	for (r = 0; r < rings - 1; r++) for (s = 0; s < sectors - 1; s++) {
		*i++ = r * sectors + s;
		*i++ = r * sectors + (s + 1);
		*i++ = (r + 1) * sectors + (s + 1);

		*i++ = (r + 1) * sectors + (s + 1);
		*i++ = (r + 1) * sectors + s;
		*i++ = r * sectors + s;
	}


	deleg(data, indices);
}
