#pragma once
namespace GE {
	class Picking {
	private:
		//given a vec3, returns the sqrt length
		static float SqrLength(glm::vec3 const&pos);

	public:
		
		/*
			For a list of points in the plane (point= 0,0,0; normal=0,1,0), returns the closest one to a position on the screen
			Parameters:
			positions -> list of points
			iVP -> inverse matrix of View and Projections
			pos -> vector containing the screen coordinates
			width -> screen width
			height -> screen height
			radius -> limiting radious for picking objects from the list of positions
		*/
		static int Pick(std::vector<glm::vec3> const & positions, glm::mat4 const & iVP, glm::vec2 const & pos, size_t width, size_t height, float radius);
		
		/*Given an inverseProjectionView matrix, the width an height of the screen, returns the screen coordinates pos
		projected on the world, on the plane (point 0,0,0, with normal 0,1,0)*/
		static glm::vec3 OnWorld(glm::mat4 const& iVP, glm::vec2 const & pos, size_t width, size_t height);

		/*Givent a ViewProjection matrix, and some coordinates in the world, transforms those coordinates to Screen coordinates*/
		static glm::vec2 OnScreen(glm::mat4 const& VP, glm::vec3 const& coord, size_t width, size_t height);
	};
}