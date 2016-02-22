#pragma once

#include <windows.h> 
#include <GL/gl.h> 
#include <GL/glu.h>
#include <atlstr.h>
#include "GraphicSystem.h"
#include "Delegate.h"
#include <string>
#include <vector>
#include <list>
#include <set>
#include "GESignal.h"
#include <GLM\glm.hpp>
#include <unordered_map>

namespace GE{

		namespace Graphics{
				class Mesh;
				class Camera;
				class Shader;
			class OpenGLGraphicsSystem :
				public GraphicSystem
			{
			public:
				
				
				static OpenGLGraphicsSystem & GetInstance();
				void Start(HWND);
				void Release();
				
				virtual void Update(double);

				siglot::Slot<int, int> resizeSlot;
				siglot::Slot<> refresh;
				siglot::Slot<HWND, UINT, WPARAM, LPARAM> uiEvent;

				glm::mat4 mProjectionMatrix;
				glm::mat4 mModelMatrix;
				glm::mat4 mViewMatrix;
				glm::mat4 MVP;
				void Initialize();

				unsigned int CreateTextureOGL(std::string const& path);
				unsigned GetTexture(unsigned) const;


			private:
				int m_width, m_height;
				HWND hWnd;
				HDC		ghDC;
				HGLRC	ghRC;
				HANDLE _context;

				struct mesh{
					mesh(unsigned int i, unsigned int b, size_t s) :varrayId(i),buffer(b), size(s){}
					unsigned int varrayId,buffer;
					size_t size;
				};

				struct texture{
					texture(size_t w, size_t h, unsigned int i) : width(w), height(h), id(i){}
					size_t width, height;
					unsigned int id;
				};

				void Destroy();
				BOOL bSetupPixelFormat(HDC hdc);
				void Resize(int, int);

				Camera* m_camera;

				std::vector<mesh> vaa;
				std::vector<texture> textureArray;

				unsigned int CreateMesh(std::vector<float>&, std::vector<unsigned int>&  );
				void Draw(unsigned int index);

				void InvertImage(int pitch, int height, unsigned char* image_pixels);
				unsigned int CreateTexture(unsigned char* image, int format, int components, int width, int height);
				void DrawModel(int texture, glm::mat4 const& M, int model, Shader* shader,bool draw);
				void DrawAnimationModels();
				void DrawStaticModels(float dt);
				void CreateShadows();

				float phi , theta , radius ;
				glm::vec2 mouse;
				float wheel;

				glm::mat4 OpenGLGraphicsSystem::Camera(double dt);
				void Refresh();

				OpenGLGraphicsSystem(OpenGLGraphicsSystem const&)=delete;
				virtual ~OpenGLGraphicsSystem();
				OpenGLGraphicsSystem();

				void UiListener(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

			};

			class FBO {
			public:
				unsigned int fbo;

				unsigned int texture;
				int width, height;  // Size of the texture.

				void CreateFBO(const int w, const int h);
				void Bind();
				void Unbind();
			};

		}
}