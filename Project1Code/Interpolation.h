#pragma once
/*
Functionalities for linera interpolation and elerp
*/
namespace GE {
	template<typename T,typename U>
	T Lerp(T const& p0, T const& p1, U t) {
		return (1 - t)*p0 + t*p1;
	}

	template<typename T>
	T Elerp(T const& p0, T const& p1, float t) {
		return p0*pow(p1 / p0, t);
	}
}