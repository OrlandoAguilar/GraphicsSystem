
#pragma once
#include <string>
namespace GE{
	namespace Utility{
		class Directory{
		public:
			static std::string GetPath(std::string const &);
			static std::string GetNameFile(std::string const &);
			static std::string GetNameFileWithoutExtension(std::string const &);
			static std::string GetCurrenDirectory();
			static bool ChangeToDirectory(std::string const &);
		private:
			static size_t GetLastSlash(std::string const &);
		};
	}
}