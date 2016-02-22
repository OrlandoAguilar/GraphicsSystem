
#include "Sphere.h"

namespace GE{
	//namespace Resources{
		namespace Graphics{


			void Sphere::Create(float radius, int rings, int sectors, tdelegate & deleg){
				const double PI = 3.1415926535897;
				const double PI_2 = 3.1415926535897 / 2;

				std::vector<unsigned int> indices;

				std::vector<float> data;

				data.resize(rings * sectors * 8);
				std::vector<float>::iterator d = data.begin();

				float const R = 1.0f / (float)(rings - 1);
				float const S = 1.0f / (float)(sectors - 1);
				int r, s;

				//for (r = rings-1; r >=0; --r) 
				for (r = 0; r<rings; ++r) for (s = 0; s < sectors; ++s) {
					float const y = (float)(sin(-PI_2 + PI * r * R));
					float const x = (float)(cos(2 * PI * s * S) * sin(PI * r * R));
					float const z = (float)(sin(2 * PI * s * S) * sin(PI * r * R));

					*d++ = x * radius;
					*d++ = y * radius;
					*d++ = z * radius;
					
					*d++ = x;
					*d++ = y;
					*d++ = z;
					
					*d++ = s*S;
					*d++ = r*R;
				}

				indices.resize(rings * sectors * 6);
				std::vector<unsigned int>::iterator i = indices.begin();
				for (r = 0; r < rings - 1; r++) for (s = 0; s < sectors - 1; s++) {
					*i++ = (r + 1) * sectors + (s + 1);
					*i++ = r * sectors + (s + 1);
					*i++ = r * sectors + s;

					*i++ = r * sectors + s;
					*i++ = (r + 1) * sectors + s;
					*i++ = (r + 1) * sectors + (s + 1);
					
				}


				deleg(data, indices);
			}

		}
}