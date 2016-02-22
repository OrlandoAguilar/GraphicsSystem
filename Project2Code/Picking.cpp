#include <windows.h>

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/GL.h>

#include <glm/gtc/matrix_transform.hpp>
#include <GLM\glm.hpp>
#include "Debug.h"
#include <vector>

#include "Picking.h"

namespace GE {
	
	/*
		Returns the sqrt lenght of the vector
	*/
	float Picking::SqrLength(glm::vec3 const&pos) {
		return glm::dot(pos, pos);
	}

	/*Given a list of positions, the inverse 1PV matrix (perspective, view), returns the closest point to the projection of the screen coordinates to the plane in (point [0,0,0], normal  [0,1,0])*/
	int Picking::Pick(std::vector<glm::vec3> const& positions, glm::mat4 const& VP, glm::vec2 const & spos, size_t width, size_t height,float radius) {
		float dist = std::numeric_limits<float>::max();
		glm::vec3 pos = OnWorld(VP,spos,width,height);		//Converts the coordinates from screen to world space
		int closest = 0;
		for (unsigned z = 0; z < positions.size();++z)
		{
			float dd = SqrLength(positions[z] - pos);		//distance between the point and the click position
			if (dd < dist)		//finds the closest point to the click position
			{
				dist = dd;
				closest = z;
			}
		}
		if (SqrLength(positions[closest] - pos)<=radius*radius)
			return closest;		//returns the index of the closest point to t
		return -1;
	}

	/*Given the inverse world matrix, the position on screen and the size of the screen, transforms the screen coordinates to world coordinates*/
	glm::vec3 Picking::OnWorld(glm::mat4 const& m_inv, glm::vec2 const & spos,size_t width, size_t height) {
		float x = spos.x;
		float y = spos.y;

		y = height - y;

		//InputOrigin, start of the ray for intersection with plane 
		glm::vec4 inputO = glm::vec4(x / width*2.0f - 1.0f, y / height*2.0f - 1.0f, -1.0f, 1.0f);	//transforms screen position to the unit cube range
		glm::vec4 resO = m_inv*inputO;		//transforms to view space
		if (resO.w == 0.0f)
			return glm::vec3(-1);

		resO /= resO.w;		//homogeneous division

		glm::vec4 inputE = inputO;		//inputEnd, the end of the ray
		inputE.z = 1.0;

		//End of ray to view space
		glm::vec4 resE = m_inv*inputE;

		//checks that the coordinates are correct
		if (resE.w == 0.0f)
			return glm::vec3(-1);

		resE /= resE.w;

		//ray for intersection
		glm::vec3 ray = glm::vec3(resE - resO);		//vector between z=-1 and z=1

		glm::vec3 normalRay = glm::normalize(ray);
		glm::vec3 normalPlane = glm::vec3(0, 1, 0);		//detects collision with plane 0, normal 1
		float denominator = glm::dot(normalRay, normalPlane);		
		if (denominator == 0)
			return glm::vec3(-1);

		float numerator = glm::dot(glm::vec3(resO), normalPlane);

		//intersection between ray and plane
		glm::vec3 result = glm::vec3(resO) - normalRay*(numerator / denominator);

		return result;
	}

	/*Transform 3D coordinates to screen coordinates*/
	glm::vec2 Picking::OnScreen(glm::mat4 const& VP, glm::vec3 const& coord, size_t width, size_t height) {
		glm::vec4 coordr = VP*glm::vec4(coord, 1);		//from world to screen coordinates
		coordr /= coordr.w;								//homogeneous division
		coordr.x = (coordr.x*0.5f + 0.5f)*width ;		//bringing to the correct range
		coordr.y = (coordr.y*0.5f + 0.5f)*height;
		coordr.y = height - coordr.y;					//inverting y
		return glm::vec2(coordr.x, coordr.y);
	}
}