#include "ArcLength.h"
#include <stack>
#include "Interpolate.h"
#include "Interpolation.h"


#define EPS 0.00001f
#define ALPHA 0.001f

namespace GE {

	/*
		the param p is a structure containing the points for building the Arc-Length table

		This function builds an Arc Length table for a pair of points, but receives 4 points in order to perform the operations of P(u)
	*/
	ArcLength::tarcLengthTable ArcLength::CalculateArcLengthTable(PointParameters const& p)
	{
		tarcLengthTable table;

		std::stack<tcurveSegment> segmentList;
		
		table.table.emplace_back(tgu(0.0f, 0.0f));		//start building the table with <0,0>
		segmentList.emplace(tcurveSegment( 0.0f,1.0f ));	//stack with range of segments to work with

		do {
			const tcurveSegment range = segmentList.top();	//takes the first segment to work with
			segmentList.pop();	//delete it from the stack

			real um = (range.ua + range.ub)*0.5f;	//get the midpoint

			glm::vec3 pua = Interpolate::Interpolation(p, range.ua);	//get the position of these points in the path P(ua) P(um) P(ub)
			glm::vec3 pum = Interpolate::Interpolation(p, um);
			glm::vec3 pub = Interpolate::Interpolation(p, range.ub);

			real A = glm::length(pua - pum);		//calculate distances
			real B = glm::length(pum - pub);
			real C = glm::length(pua - pub);

			real d = A + B - C;	//differences of distances

			if (d > EPS && (range.ub - range.ua) > ALPHA) {		//if the difference in distances is grather than an epsilon, and there are no many divisions already, populate the list with more segments to split
				segmentList.emplace(tcurveSegment(um, range.ub));
				segmentList.emplace(tcurveSegment(range.ua, um));
			}
			else {		//if the difference is very close, add the segments to the arc lenght table and dont continue spliting this segments
				table.table.emplace_back(tgu( um, table.table.back().arcl + A ));
				table.table.emplace_back(tgu(range.ub, table.table.back().arcl + B));
			}

		} while (segmentList.size() != 0);	//continue spliting until the path does not need to be split any more
		
		return table;
	}

	/*
		Receives the whole list of points in the path and returns an Arc-Length table for the whole list
	*/
	ArcLength::tarcLengthTable ArcLength::CalculateArcLengthGeneralTable(std::vector<glm::vec3> const & p)
	{
		int const pLast = p.size() - 1;		//last element in the table

		std::vector<ArcLength::tarcLengthTable> tables;		//list of tables for every pair of points

		for (int z = 0; z < pLast; ++z) {		//for every pair of points
			auto table = CalculateArcLengthTable(PointParameters(p, z));		//get its table
			table.segmentsNumber = z;		//index for interpolation
			tables.push_back(table);		//add the table to the list of tables
		}

		return ConcatenateAndNormalize(tables);		//once all the tables are calculated, concatenate them and normalize the result
	}

	/*Given an arc-length table and an s, returns G{^-1}(s), that means inverse of ArcLength table for the current s.
	The return value is in the format {u,index in table}.
	This look-up is performed like a binary search into the ArcLength table, using the s column rather than the u.
	*/

	std::pair<real, unsigned> ArcLength::InverseArcLength(const tarcLengthTable & table, real s) {
		unsigned ia = 0;
		unsigned ib = table.table.size()-1;

		tgu pm= table.table[0];
		unsigned index = 0;
		do {			
			unsigned im = (ib + ia) / 2;
			pm = table.table[im];				//s in the middle of the range we for binary search
			if (pm.arcl < s)	ia = im; else ib = im;		//performs binary search, adjusting the range for new iteration
		} while (ib-ia>1);			//search in a binary way until the binary search finds the value in the table

		real du = table.table[ia + 1].u - table.table[ia].u;	
		real um = table.table[ia].u + du*((s- table.table[ia].arcl)/(table.table[ia + 1].arcl- table.table[ia].arcl));

		return std::pair<real, unsigned>(um, ia);		//returns the u and index for current s
	}

	//given the table and a u, returns the index and the s of the u position
	std::pair<real,unsigned> ArcLength::G(const tarcLengthTable & table,real u) {
		unsigned ui = BinarySearch(table, u);
		if (ui == table.table.size() - 1) 
			return std::pair<real, unsigned>(table.table.back().arcl, ui);	//ensures not out of bound exception
		real du = table.table[ui+1].u - table.table[ui].u;
		real k = (u - table.table[ui].u) / (du);
		real s = Lerp(table.table[ui].arcl, table.table[ui+1].arcl,k);		//very likely, the u does not match an entry in the table, therefore make an interpolation to return a better a.
		return std::pair<real,unsigned>(s,ui);
	}

	/*
	Given a u, performs a binary search in the table to find the index where the closest u is found in the table
	*/
	unsigned ArcLength::BinarySearch(tarcLengthTable const& arr, real u) {
		unsigned li = 0;		//beginning of table
		unsigned ti = arr.table.size() - 1;		//end of table

		while (ti - li > 1) {			//while the element is not found
			unsigned mi = (ti + li) / 2;	//get the index in the middle of the range
			if (arr.table[mi].u < u) {		//adjust the range for the binary search according to the side where the u element is located
				li = mi;
			}else{
				ti = mi;
			}
		}
		return li;		//return the index where the closest u is located
	}

	/*Given a list of arc-length tables, creates a single table concatenating and normalizing all of them*/
	ArcLength::tarcLengthTable ArcLength::ConcatenateAndNormalize(std::vector<ArcLength::tarcLengthTable> tables)
	{
		tarcLengthTable table;		//result
		real maxPrevious = 0.0f;	//the previous s for concatenation tables, starts with s=0.0f

		//adds entry 0
		tgu entry = tables[0].table[0];
		table.table.push_back(entry);		//in the beginning adds the firs entry of the first table of the input, to the output

		//iterates over all the entries of every table, in order, to concatenate them and make a single table out of the whole list
		for (unsigned z = 0; z < tables.size(); ++z) {
			for (unsigned c = 1; c < tables[z].table.size(); ++c) {
				tgu entry = tables[z].table[c];						//gets the next entry in the tables
				entry.u = (entry.u+z)/(real)(tables.size());		//normalizes u in the entry
				entry.arcl+=maxPrevious;							//concatenates s in the entry
				table.table.push_back(entry);						//adds the entry to the concatenated table
			}
			maxPrevious = table.table.back().arcl;					//when the current table has been completelly concatenated, save the last s for concatenating next table
		}
		table.segmentsNumber = tables.size();						//number of tables that this table was made of.
		table.maxDistance = float(table.table.back().arcl);				//total length of the table (used for the animation later)
		Normalize(table);											//normalize the s of the table
		return table;
	}

	/*
		Normalize the s of a table
	*/
	void ArcLength::Normalize(tarcLengthTable & table)
	{
		real normalization = table.table.back().arcl;		//takes the last s of the table (the maximum value)

		for (unsigned z = 0; z < table.table.size(); ++z)
			table.table[z].arcl /= normalization;			//normalizes the whole table
	}


}
