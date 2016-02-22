#pragma once

#include "Hash.h"
#include "Signal.h"
#include "SystemTypes.h"


namespace GE{
	
	class GameObject;

		class System
		{
		public:
			
			virtual void Update(double)=0;
			
			
			virtual SystemType GetType() const{
				return type;
			}

			System(SystemType t) :
				type(t)
			{}
			virtual ~System(){}

		protected:
			SystemType type;
		};
	
}
