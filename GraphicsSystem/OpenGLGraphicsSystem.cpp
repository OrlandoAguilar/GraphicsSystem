//#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/wglew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLM\glm.hpp>
#include "Debug.h"
#include "OpenGLGraphicsSystem.h"
#include "Sphere.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Delegate.h"
#include "OpenGLDebugging.h"

namespace GE{
		namespace Graphics{

			OpenGLGraphicsSystem::OpenGLGraphicsSystem() {}

			OpenGLGraphicsSystem& OpenGLGraphicsSystem::GetInstance() {
				static OpenGLGraphicsSystem graphics;
				return graphics;
			}

			void OpenGLGraphicsSystem::Start(HWND h)
			{
				hWnd = h;
				ghDC = GetDC(hWnd);

				if (!bSetupPixelFormat(ghDC))
					PostQuitMessage(0);

				RECT rect;

				//ghRC = wglCreateContext(ghDC);
				
				// Create temporary context and make sure we have support
				HGLRC tempContext = wglCreateContext(ghDC);
				if (!tempContext) throw("Failed to create temporary context!");
				if (!wglMakeCurrent(ghDC, tempContext)) throw("Failed to activate temporary context!");

				int major, minor; glGetIntegerv(GL_MAJOR_VERSION, &major); glGetIntegerv(GL_MINOR_VERSION, &minor);
				if (major < 3 || minor < 3) throw("OpenGL 3.3 is not supported!");

				glewExperimental = true; // Needed in core profile
				if (glewInit() != GLEW_OK) {
					throw "Failed to initialize GLEW\n";
				}

				// Create forward compatible context
				int attribs[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB, major,
					WGL_CONTEXT_MINOR_VERSION_ARB, minor,
					WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB, //
					WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
					0
				};

				//PFNWGLCREATEBUFFERREGIONARBPROC wglCreateContextAttribsARB = (PFNWGLCREATEBUFFERREGIONARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
				
				int c = 0;
				if (!(_context = wglCreateContextAttribsARB(ghDC, 0, attribs))) throw("Failed to create forward compatible context!");
				
				// Remove temporary context and activate forward compatible context
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(tempContext);
				if (!wglMakeCurrent(ghDC, (HGLRC)_context)) throw("Failed to activate forward compatible context!");

				// Enable depth test
				glEnable(GL_DEPTH_TEST);
				// Accept fragment if it closer to the camera than the former one
				glDepthFunc(GL_LESS);

				glEnable(GL_BLEND);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);

				resizeSlot.Attach(this, &OpenGLGraphicsSystem::Resize);

				GetClientRect(hWnd, &rect);
				Resize(rect.right, rect.bottom);

				wglSwapIntervalEXT(0);

				// SUPER VERBOSE DEBUGGING!
				if (glDebugMessageControlARB != NULL) {
					glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
					glDebugMessageControlARB(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
					glDebugMessageCallbackARB((GLDEBUGPROCARB)ETB_GL_ERROR_CALLBACK, NULL);
				}
				
				printOpenGLError();

				refresh.Attach(this, &OpenGLGraphicsSystem::Refresh);
				uiEvent.Attach(this, &OpenGLGraphicsSystem::UiListener);
			}

			void OpenGLGraphicsSystem::Release() {
				//delete sphere;
				Destroy();
				wglDeleteContext(ghRC);
				ghRC = 0;
			}

			OpenGLGraphicsSystem::~OpenGLGraphicsSystem(){}


			BOOL OpenGLGraphicsSystem::bSetupPixelFormat(HDC hdc)
			{
				PIXELFORMATDESCRIPTOR pfd, *ppfd;
				int pixelformat;

				ppfd = &pfd;

				ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
				ppfd->nVersion = 1;
				ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
					PFD_DOUBLEBUFFER;
				ppfd->dwLayerMask = PFD_MAIN_PLANE;
				ppfd->iPixelType = PFD_TYPE_RGBA;
				ppfd->cColorBits = 32;
				ppfd->cDepthBits = 24;
				ppfd->cAccumBits = 0;
				ppfd->cStencilBits = 8;

				pixelformat = ChoosePixelFormat(hdc, ppfd);

				if ((pixelformat = ChoosePixelFormat(hdc, ppfd)) == 0)
				{
					MessageBox(NULL, TEXT("ChoosePixelFormat failed"), TEXT("Error"), MB_OK);
					return FALSE;
				}

				if (SetPixelFormat(hdc, pixelformat, ppfd) == FALSE)
				{
					MessageBox(NULL, TEXT("SetPixelFormat failed"), TEXT("Error"), MB_OK);
					return FALSE;
				}

				return TRUE;
			}

			void OpenGLGraphicsSystem::Resize(int width, int height)
			{
				m_width = width;
				m_height = height;

				glViewport(0, 0, width, height);

				mProjectionMatrix = glm::perspective(45.0f, (float)width / height, 0.1f, 100.0f);

			}
			
			void OpenGLGraphicsSystem::Draw(unsigned int index){
				auto mesh = vaa[index];
				glBindVertexArray(mesh.varrayId);
				glDrawElements(
					GL_TRIANGLES,      // mode
					mesh.size,    // count
					GL_UNSIGNED_INT,   // type
					(void*)0           // element array buffer offset
					);
			}

			unsigned int  OpenGLGraphicsSystem::CreateMesh(std::vector<float>& v, std::vector<unsigned int>& i){
				unsigned int VertexArrayID;
				glGenVertexArrays(1, (GLuint*)&VertexArrayID);
				glBindVertexArray(VertexArrayID);

				GLuint buffer;
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, v.size() *sizeof(float), &v[0], GL_STATIC_DRAW);

				const int s[] = { 3, 3, 2 };
				size_t accum = 0;
				for (int z = 0; z < 3; ++z){
					glEnableVertexAttribArray(z);
					glBindBuffer(GL_ARRAY_BUFFER, buffer);
					glVertexAttribPointer(
						z,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
						s[z],                  // number of elements
						GL_FLOAT,           // type
						GL_FALSE,           // normalized?
						8 * sizeof(float),                  // stride
						reinterpret_cast<void*>(accum*sizeof(float))
						);
					accum += s[z];
				}

				glGenBuffers(1, &buffer);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.size() *sizeof(unsigned int), &i[0], GL_STATIC_DRAW);

				glBindVertexArray(0);
				vaa.emplace_back(VertexArrayID,buffer,i.size());
				return vaa.size() - 1;
			}

			unsigned int OpenGLGraphicsSystem::CreateTextureOGL(std::string const& path){
				auto createTextureDelegate = siglot::CreateDelegate(this, &OpenGLGraphicsSystem::CreateTexture);
				return Texture2D::Create(path, createTextureDelegate);
			}


			unsigned int OpenGLGraphicsSystem::CreateTexture(unsigned char* image, int format, int components, int width, int height){
				unsigned int id;
				//InvertImage(width*sizeof(int), height, &image[0]);

				glGenTextures(1, &id);
				glBindTexture(GL_TEXTURE_2D, id);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				textureArray.emplace_back(width,height,id);

				return textureArray.size()-1;
			}

			unsigned OpenGLGraphicsSystem::GetTexture(unsigned i) const{
				return textureArray[i].id;
			}

			void OpenGLGraphicsSystem::InvertImage(int pitch, int height, unsigned char* image_pixels)
			{
				int index;
				unsigned char * temp_row;
				int height_div_2;

				temp_row = (unsigned char *)malloc(pitch);
				if (NULL == temp_row)
				{
					throw "Not enough memory for image inversion";
				}
				//if height is odd, don't need to swap middle row 
				height_div_2 = (int)(height >> 1);
				for (index = 0; index < height_div_2; ++index)    {
					memcpy((unsigned char *)temp_row, (unsigned char *)(image_pixels)+pitch * index, pitch);
					memcpy((unsigned char *)(image_pixels)+pitch * index, (unsigned char *)(image_pixels)+pitch * (height - index - 1), pitch);
					memcpy((unsigned char *)(image_pixels)+pitch * (height - index - 1), temp_row, pitch);
				}
				free(temp_row);
			}


			void FBO::CreateFBO(const int w, const int h)
			{
				width = w;
				height = h;

				glGenFramebuffersEXT(1, &fbo);
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

				// Create a render buffer, and attach it to FBO's depth attachment
				unsigned int depthBuffer;
				glGenRenderbuffersEXT(1, &depthBuffer);
				glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
				glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
					width, height);
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
					GL_RENDERBUFFER_EXT, depthBuffer);

				// Create texture and attach FBO's color 0 attachment
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height,
					0, GL_RGBA, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT,
					GL_TEXTURE_2D, texture, 0);

				// Check for completeness/correctness
				int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
				if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
					DBOUT("FBO Error: "<<status<<"\n");

				// Unbind the fbo.
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			}


			void FBO::Bind() { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); }
			void FBO::Unbind() { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); }

			void OpenGLGraphicsSystem::Refresh() {
				Update(0);		//for update purposes
			}
		}
	}

	//continue in OpenGLGraphicsSystemScene.cpp
