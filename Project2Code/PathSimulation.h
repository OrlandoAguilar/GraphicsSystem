#pragma once
#include <GLM\glm.hpp>
#include <vector>
#include "ArcLength.h"
#include "DataType.h"
#include "Shader.h"

namespace GE {
	class PathSimulation {
	private:
		std::vector<glm::vec3> mPositions;		//whole list of points for the path					
		ArcLength::tarcLengthTable mG;			//arc-length table for the whole path
		std::vector<glm::vec3> positionInterpolated;
		unsigned positionBuffer;
		unsigned positionsVA;
		real t;			//time for the path
		real time;		//time of the animation
		glm::mat4 mTransform;
	public:
							//transformation of the model
		void RecalculateInfo(std::vector<glm::vec3> const& p);		//calculate arc-length table from a list of points
		void UpdatePosition(real t);								//given a t, updates the animation of the character
		const ArcLength::tarcLengthTable& GetTable() const;
		
		void Init();						//initializes the opengl buffers for the points
		void CopyPositions(std::vector<glm::vec3> const& positions);				//copies the points for the path to the gpu memory
		
		real AnimateWalking(float dt);		//calls the functionality to move the character using the dt, this is the entry point for the simulation

		static const int MAX_POINTS = 20;
		static const int MAX_POINTS_INTERPOLATED = MAX_POINTS * 8;
		void DrawPath();
		PathSimulation();
		glm::mat4 const& GetTransform()const;
	};
}
