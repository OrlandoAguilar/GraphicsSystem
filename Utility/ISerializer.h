#pragma once
#include <string>

namespace GE{
	namespace Utility{
		class ISerializer
		{
		public:

			virtual float ReadFloat(const std::string&) = 0;
			virtual int ReadInt(const std::string&) = 0;
			virtual std::string ReadString(const std::string&) = 0;
			virtual bool ReadBool(const std::string&) = 0;
			virtual void ReadChunk(const std::string&) = 0;
			virtual bool Next() = 0;
			virtual void Out() = 0;
			virtual void Open(std::string const &) = 0;
			virtual void Close() = 0;
			virtual void OpenTable(std::string const &) = 0;
			/*virtual bool IsGood() = 0;
			virtual bool IsOpen() = 0;
			virtual std::string ReadLine() = 0;
			virtual std::string GetName() = 0;*/

			ISerializer(){}
			virtual ~ISerializer(){}

		};
	}
}
