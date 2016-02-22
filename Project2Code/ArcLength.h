#pragma once

#include <list>
#include <GLM\glm.hpp>
#include <vector>
#include "PointParameters.h"
#include "DataType.h"

namespace GE {

	class ArcLength {
	public :
		struct tgu{
			tgu(real _u, real _a) : u(_u), arcl(_a) {}			//entry for the arc length table
			real u, arcl;		//u parameter, arcLength value
		};

		struct tcurveSegment {
			tcurveSegment(real a, real b) : ua(a), ub(b) {}
			real ua, ub;		//curve-segment entry
		};

		struct tarcLengthTable {
			std::vector<tgu> table;		//vector with the entries of the table
			int segmentsNumber;			// Number of segments in the table
			float maxDistance;			//unnormalized distance in the path, used to control the advance of the time in the function for keeping animation velocity constant regardless the length of the path
		};

		//given a pair of points (and two extra points requiered by the interpolation algorithm), creates an arc-length table
		static tarcLengthTable CalculateArcLengthTable(PointParameters const & p);

		//given the list of points of the whole path, creates an arc'lenght table made of smaller tables for every pair of points
		static tarcLengthTable CalculateArcLengthGeneralTable(std::vector<glm::vec3> const & p);

		//given an s, returns the u that best matches that s, and its index in the arc-length table
		static std::pair<real, unsigned> InverseArcLength(const tarcLengthTable & table, real s);

		//Givent a table and a u, returns the s and the index where the s was found by using binary search
		static std::pair<real, unsigned> G(const tarcLengthTable & table, real u);

	private:
		//performas binary search in the arc'length function to find u, and returns its index
		static unsigned BinarySearch(tarcLengthTable const& arr, real u);

		//given a list of arc length tables, concatenates all of them and returns one table with the concatenation
		static tarcLengthTable ConcatenateAndNormalize(std::vector<ArcLength::tarcLengthTable> tables);

		//normalizes the s factor of a table
		static void Normalize(tarcLengthTable & table);

	};

}