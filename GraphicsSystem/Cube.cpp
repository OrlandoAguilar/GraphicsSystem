
#include "Cube.h"
#include "GraphicSystem.h"

namespace GE{
	namespace Graphics{
		void Cube::Create(float size, tdelegate & deleg){

		std::vector<unsigned int> indices = {
			// front
			0, 1, 2,
			2, 3, 0,
			// top
			4, 5, 6,
			6, 7, 4,
			// back
			8, 9, 10,
			10, 11, 8,
			// bottom
			12, 13, 14,
			14, 15, 12,
			// left
			16, 17, 18,
			18, 19, 16,
			// right
			20, 21, 22,
			22, 23, 20,
		};

		size /= 2;

		std::vector<float> data = {
			// front
			-size, -size, size, 0, 0, 1, 0.0, 0.0,
			size, -size, size, 0, 0, 1, 1.0, 0.0,
			size, size, size, 0, 0, 1, 1.0, 1.0,
			-size, size, size, 0, 0, 1, 0.0, 1.0,
			// top
			-size, size, size, 0, 1, 0, 0.0, 0.0,
			size, size, size, 0, 1, 0, 1.0, 0.0,
			size, size, -size, 0, 1, 0, 1.0, 1.0,
			-size, size, -size, 0, 1, 0, 0.0, 1.0,
			// back
			size, -size, -size, 0, 0, -1, 0.0, 0.0,
			-size, -size, -size, 0, 0, -1, 1.0, 0.0,
			-size, size, -size, 0, 0, -1, 1.0, 1.0,
			size, size, -size, 0, 0, -1, 0.0, 1.0,
			// bottom
			-size, -size, -size, 0, -1, 0, 0.0, 0.0,
			size, -size, -size, 0, -1, 0, 1.0, 0.0,
			size, -size, size, 0, -1, 0, 1.0, 1.0,
			-size, -size, size, 0, -1, 0, 0.0, 1.0,
			// left
			-size, -size, -size, -1, 0, 0, 0.0, 0.0,
			-size, -size, size, -1, 0, 0, 1.0, 0.0,
			-size, size, size, -1, 0, 0, 1.0, 1.0,
			-size, size, -size, -1, 0, 0, 0.0, 1.0,
			// right
			size, -size, size, 1, 0, 0, 0.0, 0.0,
			size, -size, -size, 1, 0, 0, 1.0, 0.0,
			size, size, -size, 1, 0, 0, 1.0, 1.0,
			size, size, size, 1, 0, 0, 0.0, 1.0,
		};


		deleg(data, indices);



	}

	}
}