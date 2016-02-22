
#include "Directory.h"
#include <windows.h> 
namespace GE{
	namespace Utility{
#define BUFSIZE MAX_PATH

		size_t Directory::GetLastSlash(std::string const &f){
			size_t last = f.find_last_of("/\\");
			return last == std::string::npos ? 0 : last;
		}

		std::string Directory::GetPath(std::string const &f){
			size_t last = GetLastSlash(f);
			return f.substr(0, last);
		}
		std::string Directory::GetNameFile(std::string const &f){
			size_t last = GetLastSlash(f);
			if (last != 0)
				++last;
			return f.substr(last);
		}

		std::string Directory::GetNameFileWithoutExtension(std::string const &f){
			std::string file = GetNameFile(f);
			int fpoint = file.find('.');
			if (fpoint != std::string::npos){
				file = file.substr(0, fpoint);
			}
			return file;
		}

		std::string Directory::GetCurrenDirectory(){
			char Buffer[BUFSIZE] = { 0 };
			GetCurrentDirectory(BUFSIZE, Buffer);
			return std::string(Buffer);
		}

		bool Directory::ChangeToDirectory(std::string const &path){
			return SetCurrentDirectory(path.c_str()) > 0;
		}
	}
}