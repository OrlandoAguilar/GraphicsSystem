#pragma once
#include "ISerializer.h"

namespace GE{
	namespace Utility{

		class ISerializable
		{
		public:

			virtual void Serialize(ISerializer &) = 0;

			ISerializable(){}
			virtual ~ISerializable(){}
		};

	}
}

