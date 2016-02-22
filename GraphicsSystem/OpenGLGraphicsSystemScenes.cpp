/*In this file, the resources for the symulation are loaded, updated every frame and displayed. This file
Is the abstraction of the simulation with respect to the Engine. In order to create a totally different simulation,
the modification of this file only should be enogh in a lot of situations. */

#include <windows.h>

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/GL.h>

#include <algorithm>   
#include <glm/gtc/matrix_transform.hpp>
#include <GLM\glm.hpp>
#include "Debug.h"
#include "OpenGLGraphicsSystem.h"
#include "Sphere.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Delegate.h"
#include "InputSystem.h"
#include "Cube.h"
#include "Plane.h"
#include "Cone.h"
#include "SkinnedMesh.h"
#include "WindowSystem.h"
#include "resource.h"
#include <commctrl.h> 
#include "Interpolate.h"
#include "Picking.h"
#include "ArcLength.h"
#include "PathSimulation.h"

namespace GE {

	namespace Graphics {

#define MAX_BONES 100
#define COMPLETE_INTERPOLATION 0
#define INCREMENTAL_INTERPOLATION 1
#define INCREMENTAL_CHEBY_INTERPOLATION 2


		/*Shaders for the environment*/
		static Shader* ligthingShader;
		static Shader* shadowShader;
		static Shader*skinning;
		static Shader*sbones;
		static Shader*lines;

		/*Values for the position of the light*/
		static float thetaLight = 0.0f;
		static float phiLight = 0.0f;
		static float radiusLight = 8.0f;

		/*Values for the camera*/
		static glm::vec3 cameraPosition;
		static glm::vec3 cameraShift;

		/*Shadow frame buffer object*/
		static FBO shadowFbo;

		/*Mesh for the articulated object*/
		static SkinnedMesh* m;

		/*location of bones in shader*/
		static GLuint boneLocation;

		/*number of bones in the object*/
		static unsigned bones = 0;

		/*vector with the transformations for the current time*/
		static std::vector <glm::mat4> transforms;

		/*Matrices for the scene*/
		static glm::mat4 View;
		static glm::vec3 lightPosition;
		static glm::mat4 LightMatrix;
		static glm::mat4 PV;
		static glm::mat4x4 shadowMatrix;
		static const glm::mat4 Identity;

		/*Instances for the other systems. In this file, the symulation is updated (rather than in a main loop), therefore
		this references are necessary*/
		static InputSystem& inputSystem = InputSystem::GetInstance();
		static WindowSystem& windowSystem = WindowSystem::GetInstance();

		/*options of window*/
		static bool drawBones = true;
		static bool automaticAnim = true;
		static bool shadows = true;
		static bool drawCenter = true;
		static int interpolationType = COMPLETE_INTERPOLATION;

		glm::mat4 NormalMatrix(glm::mat4 M);

		/*variables for the path*/
		static int choosen = -1;
		static std::vector<glm::vec3> positions;
		
		/*class containing the information and functionality for the path simulation*/
		static PathSimulation pathSim;

		/*Initialize the scene*/
		void OpenGLGraphicsSystem::Initialize() {
			//creates shaders
			shadowShader = new Shader("shadow");
			ligthingShader = new Shader("myShader");
			skinning = new Shader("skinning");
			sbones = new Shader("Bones");
			lines = new Shader("line", Shader::VF);
			
			/*mesh delegate to register them into the system*/
			auto createMeshDelegate = siglot::Delegate<OpenGLGraphicsSystem, unsigned int, std::vector<float>&, std::vector<unsigned int>& >(this, &OpenGLGraphicsSystem::CreateMesh);

			/*creates the meshes*/
			Cube::Create(1.0f, createMeshDelegate);
			Sphere::Create(0.5f, 10, 10, createMeshDelegate);
			Plane::Create(10.0, 10.0f, createMeshDelegate);
			Cone::Create(1.0f, 10, createMeshDelegate);

			/*creates the textures*/
			auto createTextureDelegate = siglot::CreateDelegate(this, &OpenGLGraphicsSystem::CreateTexture);
			Texture2D::Create("Resource/Textures/green.png", createTextureDelegate);
			Texture2D::Create("Resource/Textures/microscheme.png", createTextureDelegate);
			Texture2D::Create("Resource/Textures/volcano_floor.png", createTextureDelegate);

			/*initialize values for input, camera and matrices*/
			mouse = inputSystem.mouse->GetPosition();
			wheel = inputSystem.mouse->GetWheel();
			cameraShift = glm::vec3(0);
			phi = 45.0f, theta = 0, radius = 5;
			mProjectionMatrix = glm::perspective(45.0f, (float)m_width / m_height, 0.1f, 50.0f);

			/*initialize information for the meshes and bones*/
			m  = new SkinnedMesh();
			m->LoadMesh("Resource/models/stamp.x"); 
			bones = m->NumBones();
			transforms.resize(bones);
			boneLocation = glGetUniformLocation(skinning->shader, "gBones");

			/*buffer object for shadow*/
			shadowFbo.CreateFBO(2048, 2048);

			/*window initialization for controllers*/
			SendDlgItemMessage(windowSystem.GetDownUIHandler(), autAnim, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(windowSystem.GetDownUIHandler(), BONESID, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(windowSystem.GetDownUIHandler(), IDC_CHECK1, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(windowSystem.GetDownUIHandler(), DrawCenterId, BM_SETCHECK, 1, 0);


			//initializes the list of points with a sin shape
			unsigned const maxPoints = PathSimulation::MAX_POINTS - 6;
			positions.resize(maxPoints);
			for (int z = 0; z < maxPoints; ++z) {
				positions[z] = glm::vec3(sinf(z*0.4f)*3.0f, 0, -4.5 + z*0.72f);
			}

			/*initialize the character class*/
			pathSim.Init();
			pathSim.CopyPositions(positions);
		}


		/*
		Updates and calculated the graphics every frame
		*/
		void OpenGLGraphicsSystem::Update(double dt) {
			/*updates camera and gets the view matrix*/
			View = Camera(dt);
			glm::mat4 viewInverse = glm::inverse(View);
			cameraPosition = glm::vec3(viewInverse*glm::vec4(0, 0, 0, 1));

			/*light configuration*/
			glm::mat4 rotY = glm::rotate(glm::mat4(), thetaLight, glm::vec3(0, 1, 0));
			glm::mat4 rotXY = glm::rotate(rotY, phiLight, glm::vec3(1, 0, 0));
			LightMatrix = glm::translate(rotXY, glm::vec3(0, radiusLight, 0));
			lightPosition = glm::vec3(LightMatrix*glm::vec4(0, 0, 0, 1));

			/*Spot Light matrices, for shadows*/
			glm::mat4 LightView = glm::lookAt(lightPosition, glm::vec3(0), glm::vec3(0, 0, 1));
			float distance = sqrtf(glm::dot(lightPosition, lightPosition));
			glm::mat4 LightProj = glm::perspective(atanf(15.0f / (distance))*57.2957795f, 1.0f, 0.10f, 100.0f);
			PV = LightProj*LightView;

			if (shadows) {
				//shadows matrix
				glm::mat4x4 BMatrix = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f)) * glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
				shadowMatrix = BMatrix*PV;
				CreateShadows();		//calculates shadows
			}

			/*Controls to modify the path*/
			if (inputSystem.mouse->GetButtonState(InputSystem::Mouse::RIGHT)) {
				auto iVP = glm::inverse(mProjectionMatrix*View);
				if (choosen == -1) {
					if (inputSystem.keyboard->GetButtonState('I') && positions.size() < PathSimulation::MAX_POINTS) {
						auto inWorld = Picking::OnWorld(iVP, inputSystem.mouse->GetPosition(), m_width, m_height);
						positions.push_back(inWorld);
						choosen = positions.size() - 1;
					}else if(inputSystem.keyboard->GetButtonState('D') && positions.size() >4) {
						positions.pop_back();
						choosen = -2;		//deleted
					}else {
						choosen = Picking::Pick(positions, iVP, inputSystem.mouse->GetPosition(), m_width, m_height, 0.2f);
					}
				}
				else if (choosen>=0){
					auto inWorld = Picking::OnWorld(iVP, inputSystem.mouse->GetPosition(), m_width, m_height);
					positions[choosen] = inWorld;
				}			
				pathSim.CopyPositions(positions); // updates the character path with the new path
			}
			else {
				choosen = -1;		//no choosen
			}

			real time = pathSim.AnimateWalking(float(dt)); //draws the path and updates the information for the model. Also, returns the time that must be used for animating the model
			m->BoneTransform(float(time), transforms, 0);		//calculates the bone transformations for the current time for the animation

			glViewport(0, 0, m_width, m_height);
			glClearColor(0.3f, 0.2f, 0.9f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			DrawStaticModels((float)dt);		//draw all of the static models

			DrawAnimationModels();		//draw all of the animated models

			SwapBuffers(ghDC);

		}

		/*Auxiliar function for drawing static models*/
		void OpenGLGraphicsSystem::DrawModel(int tx, glm::mat4 const& M, int model, Shader* shader, bool draw) {
			if (draw) {
				glActiveTexture(GL_TEXTURE1);
				int t = textureArray[tx].id;
				glBindTexture(GL_TEXTURE_2D, t);
				GLuint texture = glGetUniformLocation(shader->shader, "texture0");
				glUniform1i(texture, 1);
				glm::mat4 normalMatrix = NormalMatrix(M);
				shader->SetUniform4mv("normalMatrix", &normalMatrix[0][0]);
			}

			shader->SetUniform4mv("M", &M[0][0]);
			Draw(model);
		}

		/*Draw the animated models*/
		void OpenGLGraphicsSystem::DrawAnimationModels()
		{
			//Draw animated meshes
			skinning->SetShaderActive(true);
			glm::mat4 M = pathSim.GetTransform()*glm::scale(glm::mat4(), glm::vec3(0.02f));
			M[3][1] -= 0.1f;
			skinning->SetUniform4mv("V", &View[0][0]);
			skinning->SetUniform4mv("P", &mProjectionMatrix[0][0]);
			skinning->SetUniform4mv("M", &M[0][0]);
			skinning->SetUniform3fv("cameraPosition", 1, &cameraPosition[0]);
			skinning->SetUniform3fv("lightPosition", 1, &lightPosition[0]);
			skinning->SetUniform("shadows", (int)shadows);

			skinning->SetUniform4mv("shadowMatrix", &shadowMatrix[0][0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowFbo.texture);
			skinning->SetUniform("shadowMap", 0);

			/*sends matrices of animation to shader*/
			glUniformMatrix4fv(boneLocation, transforms.size(), GL_FALSE, &transforms[0][0][0]);
			m->Render(skinning);

			//Draw bones
			if (drawBones) {
				sbones->SetShaderActive(true);
				glDisable(GL_DEPTH_TEST);

				sbones->SetUniform4mv("V", &View[0][0]);
				sbones->SetUniform4mv("P", &mProjectionMatrix[0][0]);
				auto mesh = vaa[3];
				m->RenderBones(M, sbones, mesh.varrayId, mesh.size);

				glEnable(GL_DEPTH_TEST);
				DrawModel(0, LightMatrix, 3, sbones, true);
			}

		}

		void OpenGLGraphicsSystem::DrawStaticModels(float dt)
		{
			//normal pass
			auto M = glm::mat4();

			ligthingShader->SetShaderActive(true);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowFbo.texture);
			ligthingShader->SetUniform("shadowMap", 0);
			ligthingShader->SetUniform("shadows", (int)shadows);
			ligthingShader->SetUniform3fv("cameraPosition", 1, &cameraPosition[0]);
			ligthingShader->SetUniform3fv("lightPosition", 1, &lightPosition[0]);
			ligthingShader->SetUniform4mv("V", &View[0][0]);
			ligthingShader->SetUniform4mv("P", &mProjectionMatrix[0][0]);
			//cone is inverted

			ligthingShader->SetUniform4mv("shadowMatrix", &shadowMatrix[0][0]);

			
			M[3][1] = -0.01f; M[3][0] = 0.0f;
			DrawModel(0, M, 2, ligthingShader, true);

			M = glm::scale(Identity, glm::vec3(0.3f));

			for (unsigned z = 0; z < positions.size();++z) {
				auto pos = positions[z];
				M[3][0] = pos.x; M[3][1] = pos.y;  M[3][2] = pos.z;
				if (z==choosen) {	//highlights selected element
					M[3][1] = pos.y+0.05f;
				}
				DrawModel(1, M, 1, ligthingShader, true);
				
			}

			if (drawCenter) {
				glDisable(GL_DEPTH_TEST);
				M = glm::translate(Identity, -cameraShift)*glm::scale(Identity, glm::vec3(0.1f));
				DrawModel(2, M, 1, ligthingShader, true);
				glEnable(GL_DEPTH_TEST);
			}

			//shader for path (renders the path)
			lines->SetShaderActive(true);
			lines->SetUniform4mv("V", &View[0][0]);
			lines->SetUniform4mv("P", &mProjectionMatrix[0][0]);
			pathSim.DrawPath();	//draws the path of the character
			
		}

		/*Creates the shadow map*/
		void OpenGLGraphicsSystem::CreateShadows()
		{
			//shadow pass
			glDisable(GL_BLEND);		//if I dont disable, every color will be multiplied
			shadowFbo.Bind();
			shadowShader->SetShaderActive(true);

			glViewport(0, 0, shadowFbo.width, shadowFbo.height);
			glClearColor(0.3f, 0.1f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shadowShader->SetUniform4mv("PrVi", &PV[0][0]);

			glm::mat4 M = pathSim.GetTransform()*glm::scale(glm::mat4(), glm::vec3(0.02f));
			M[3][1] -= 0.1f;
			glUniformMatrix4fv(glGetUniformLocation(shadowShader->shader, "gBones"), transforms.size(), GL_FALSE, &transforms[0][0][0]);
			shadowShader->SetUniform4mv("M", &M[0][0]);
			shadowShader->SetUniform("numberBones", (int)1);
			m->Render(shadowShader);

			M = glm::mat4();
			shadowShader->SetUniform("numberBones", (int)0);
			
			M[3][1] = -0.01f; M[3][0] = 0.0f;
			DrawModel(0, M, 2, shadowShader, true);

			shadowFbo.Unbind();
			shadowShader->SetShaderActive(false);

			glEnable(GL_BLEND);
		}

		void OpenGLGraphicsSystem::Destroy() {
			delete ligthingShader;
			delete shadowShader;
			delete skinning;
			delete m;
			delete sbones;
			delete lines;
		}

		/*Updated the camera every frame*/
		glm::mat4 OpenGLGraphicsSystem::Camera(double dt) {
			glm::vec2 mpos = inputSystem.mouse->GetPosition();
			float auxwheel = inputSystem.mouse->GetWheel();
			float const advance = 15.0f*(float)dt;

			float dx = (mpos.x - mouse.x);
			float dy = (mpos.y - mouse.y);
			float dr = (auxwheel - wheel);
			float const range = 6.0f;

			if (inputSystem.keyboard->GetButtonState(NUM0))
				theta += advance*6.0f;
			if (inputSystem.keyboard->GetButtonState(NUM_POINT))
				theta -= advance*6.0f;

			if (inputSystem.keyboard->GetButtonState(NUM_PLUS))
				radius += advance / 10.0f;
			if (inputSystem.keyboard->GetButtonState(NUM_MINUS))
				radius -= advance / 10.0f;


			if (inputSystem.keyboard->GetButtonState(NUM4))
				cameraShift.x -= advance / 3.0f;
			if (inputSystem.keyboard->GetButtonState(NUM6))
				cameraShift.x += advance / 3.0f;

			if (inputSystem.keyboard->GetButtonState(NUM7))
				cameraShift.y += advance/3.0f;
			if (inputSystem.keyboard->GetButtonState(NUM1))
				cameraShift.y -= advance / 3.0f;

			if (inputSystem.keyboard->GetButtonState(NUM8))
				cameraShift.z += advance / 3.0f;
			if (inputSystem.keyboard->GetButtonState(NUM2))
				cameraShift.z -= advance / 3.0f;

			if (inputSystem.keyboard->GetButtonState(NUM5)) {
				cameraShift = glm::vec3(0);
				phi = 45.0f, theta = 0, radius = 5;
			}

			if (inputSystem.mouse->GetButtonState(InputSystem::Mouse::LEFT)) {
				if (inputSystem.keyboard->GetButtonState(L_SHIFT)) {
					thetaLight += glm::clamp(dx, -range, range);
					phiLight += glm::clamp(dy, -range, range);
				}
				else {
					theta += glm::clamp(dx, -range, range);
					phi += glm::clamp(dy, -range, range);
				}
			}

			if (inputSystem.keyboard->GetButtonState(L_SHIFT)) {
				radiusLight += dr / 120.0f;
			}
			else {
				radius += dr / 120.0f;
			}

			glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0, 0, -radius));
			glm::mat4 rx = glm::rotate(glm::mat4(), phi, glm::vec3(1, 0, 0));
			glm::mat4 ry = glm::rotate(glm::mat4(), theta, glm::vec3(0, 1, 0));
			glm::mat4 shift = glm::translate(glm::mat4(), cameraShift);
			glm::mat4 View = trans*rx*ry*shift;
			
			mouse = mpos;
			wheel = auxwheel;

			return View;
		}

		/*
			receives the events for the controllers and changes the properties of the simulation
		*/
		void OpenGLGraphicsSystem::UiListener(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch (msg) {
			case WM_COMMAND:
				switch (LOWORD(wParam)) {
				case IDC_CHECK1:
					if (HIWORD(wParam) == BN_CLICKED)
						shadows = SendDlgItemMessage(windowSystem.GetDownUIHandler(), IDC_CHECK1, BM_GETCHECK, 0, 0) != 0;		//Do I have shadows?

					break;

				case DrawCenterId:
					if (HIWORD(wParam) == BN_CLICKED)
						drawCenter = SendDlgItemMessage(windowSystem.GetDownUIHandler(), DrawCenterId, BM_GETCHECK, 0, 0) != 0;		//Do I have shadows?

					break;
				case autAnim:
					if (HIWORD(wParam) == BN_CLICKED)
						automaticAnim = SendDlgItemMessage(windowSystem.GetDownUIHandler(), autAnim, BM_GETCHECK, 0, 0) != 0;		//is animation automatic?
					break;
				case BONESID:
					if (HIWORD(wParam) == BN_CLICKED)
						drawBones = SendDlgItemMessage(windowSystem.GetDownUIHandler(), BONESID, BM_GETCHECK, 0, 0) != 0;		//should I render bones
					break;

				}
				break;
			}
		}
		
		/*Calculates the normal matrix*/
		glm::mat4 NormalMatrix(glm::mat4 M) {
			return glm::inverse(glm::transpose(M));
		}

	}
}