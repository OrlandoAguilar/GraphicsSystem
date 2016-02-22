
#include "Plane.h"
#include <vector>


void Plane::Create(float width, float height, tdelegate &deleg){

	float w2 = width / 2;
	float h2 = height / 2;

	//vertex
	std::vector<float> data = {
		// front
		-w2, 0, -h2, 0, 1, 0, 0.0, 0.0,
		 w2, 0, -h2, 0, 1, 0, 1.0, 0.0,
		 w2, 0,  h2, 0, 1, 0, 1.0, 1.0,
		-w2, 0,  h2, 0, 1, 0, 0.0, 1.0,
	};


	std::vector<unsigned int> indices = {
		// front
		3, 2, 1,
		1, 0, 3,
	};
	
	deleg(data, indices);

}
