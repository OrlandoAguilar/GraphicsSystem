#pragma once
namespace GE{
	namespace Graphics{
		class Texture{
		public:
			virtual void Create(unsigned char*, int format, int components, int width, int height) = 0;
			virtual ~Texture(){}
		};
	}
}