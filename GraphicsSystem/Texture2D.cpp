#include "Texture2D.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "Hash.h"
#include "Exception.h"
#include "Debug.h"
#include "GraphicSystem.h"
#include "Texture.h"
#include <vector>
#include "SOIL.h"

namespace GE {
	namespace Graphics {

		unsigned int Texture2D::Create(std::string const&path, tdelegate& deleg) {

			int  iComponents;
			int width, height;

			unsigned char* tex_2d = SOIL_load_image
				(
					path.c_str(),
					&width, &height, &iComponents, SOIL_LOAD_RGBA
					);


			if (tex_2d == 0) {
				const char* description = SOIL_last_result();
				DBOUT(description << "\n");
				throw BadFormatException(path);
			}
			unsigned v = deleg(tex_2d, 4, 0, width, height);

			SOIL_free_image_data(tex_2d);
			return v;
		}
	}
}