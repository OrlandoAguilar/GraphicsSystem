#pragma once
#include "System.h"
namespace GE{

	namespace Graphics{
		class Texture;

		class GraphicSystem : public System{
		public:
			GraphicSystem() :System(GRAPHICS_SYSTEM){}

		};
	}
}