#include <windows.h>

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/GL.h>

#include "PathSimulation.h"
#include "Interpolate.h"
#include <algorithm>
#include "SpeedControl.h"

namespace GE {

	//calculate arc-length table from a list of points
	void PathSimulation::RecalculateInfo(std::vector<glm::vec3> const & p)
	{
		mPositions = p;		//saves the list of interpolation points in the class
		mG = ArcLength::CalculateArcLengthGeneralTable(p);		//calculates the arc length table with the new points

	}


	/*given a t, updates the position and orientation of the transformation */
	void PathSimulation::UpdatePosition(real t)
	{
		assert(t>=0 && t <= 1.0f);		//checks ranges for t

		auto inverse = ArcLength::InverseArcLength(mG, t);		//calculates s for current t, using the inverse arc length function

		//denormalizing steps
		const int index = static_cast<int>(inverse.first*mG.segmentsNumber);	//index in arcLength function for current u
		const real localU = inverse.first*mG.segmentsNumber - index;			//local u index in the segment		

		//position of the PathSimulation
		glm::vec3 pos = Interpolate::Interpolation(PointParameters(mPositions, index), localU);

		//positions for average and smooth orientation
		glm::vec3 pos1 = Interpolate::Interpolation(PointParameters(mPositions, index), localU +0.01);
		glm::vec3 pos2 = Interpolate::Interpolation(PointParameters(mPositions, index), localU +0.001);
		glm::vec3 pos3 = Interpolate::Interpolation(PointParameters(mPositions, index), localU +0.005);

		//vector orientations for matrices
		glm::vec3 wv = glm::normalize((pos1 + pos2 + pos3)/3.0f - pos);
		glm::vec3 uv = glm::normalize(glm::cross(glm::vec3(0, 1, 0), wv));
		glm::vec3 vv = glm::normalize(glm::cross(wv, uv));

		//building matrix orientation
		mTransform[0][0] = uv.x;
		mTransform[0][1] = uv.y;
		mTransform[0][2] = uv.z;

		mTransform[1][0] = vv.x;
		mTransform[1][1] = vv.y;
		mTransform[1][2] = vv.z;

		mTransform[2][0] = wv.x;
		mTransform[2][1] = wv.y;
		mTransform[2][2] = wv.z;

		//translation
		mTransform[3][0] = pos.x;
		mTransform[3][1] = pos.y;
		mTransform[3][2] = pos.z;

		//other terms of the matrix
		mTransform[3][3] = 1.0f;

		mTransform[0][3] =0.0f;
		mTransform[1][3] =0.0f;
		mTransform[2][3] =0.0f;

	}

	const ArcLength::tarcLengthTable & PathSimulation::GetTable() const
	{
		return mG;
	}

	void PathSimulation::Init() {

		//allocates the arrays in gpu
		glGenVertexArrays(1, &positionsVA);
		glBindVertexArray(positionsVA);

		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINTS_INTERPOLATED*sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // number of elements
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			0
			);

		glBindVertexArray(0);

	}

	/*When the path is modified, the list of vertex is updated in both: the GPU and RAM, besides, 
	the information related to the animation is updated also and the points interpolated*/
	void PathSimulation::CopyPositions(std::vector<glm::vec3> const& positions) {

		int const interval = 8;		//number of interpolated points calculated between pair of choosen points
		int const pLast = positions.size() - 1;
		positionInterpolated.resize(pLast * interval + 1);		//number of points for interpolation
		
		//For every pair of points in the path
		for (int z = 0; z < pLast; ++z) {
			PointParameters param(positions, z);
			for (unsigned c = 0; c < interval; ++c) {		//interpolates 8 times that pair
				float t = c / (float)interval;
				positionInterpolated[z*interval + c] = Interpolate::Interpolation(param, t);
			}
		}

		positionInterpolated[positionInterpolated.size() - 1] = positions[pLast];	//at the end, add the last point whichi is never going to be interpolated
																					/*Updates the interpolated points in OpenGL*/
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &positionInterpolated[0], sizeof(positionInterpolated[0])*positionInterpolated.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);

		//recalculates the information for the PathSimulation animation
		RecalculateInfo(positions);
		
	}

	real PathSimulation::AnimateWalking(float dt) {
		dt = dt*0.7f;		//reducing general speed
		auto s = SpeedControl::SEasyInOut(t, 0.5, 0.8);		//time, start, end; Calculates the velocity for the current time

		UpdatePosition(s.first);					//updates the position given S, calculated from the velocity in the current time
		float vt = (float)s.second;							//velocity in current time
		//real controlStep = (std::fmodf(time, 0.5) < 0.40f ? 1.0f : 1.3f);
		time += (float)(vt * 0.935)*dt;	//time of animation += velocity of animation at currentime*pace of animation*dt

		float length = GetTable().maxDistance;		//length of the path
		t += real((dt / length)*(std::fmodf(float(time), 0.5f)<0.45f ? 1.0f : 0.8f));	//dividing by length makes the traveling speed independent of length of curve

		return time;
	}

	void PathSimulation::DrawPath() {
		/*Draws the lines for the path*/
		glBindVertexArray(positionsVA);
		glDrawArrays(GL_LINE_STRIP, 0, positionInterpolated.size());
		glBindVertexArray(0);
	}

	PathSimulation::PathSimulation() : t(1.0) {}

	glm::mat4 const & PathSimulation::GetTransform() const
	{
		return mTransform;
	}

}
