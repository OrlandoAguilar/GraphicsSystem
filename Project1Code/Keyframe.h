#pragma once
/*Includes the vqs for the keyframe and the time*/

#include "VQS.h"

namespace GE {
	class Keyframe{
	public:
		float time;
		VQS vqs;
		VQS ivqs;
		Keyframe(float _t, VQS _vqs) :time(_t), vqs(_vqs) {}
	};
}