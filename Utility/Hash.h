#pragma once
#include <functional>
#include <string>

namespace GE{
	namespace Utility{
		class Hash{
		public:
			static unsigned int GetHash(std::string const & s){
				std::hash<std::string> hash_fn;
				return hash_fn(s);
			}
			static unsigned int GetId(std::string const & s){
				return GetHash(s);
			}

			static unsigned int GetId(unsigned int ui){
				std::hash<unsigned int> hash_fn;
				return hash_fn(ui);
			}
		};
	}
}