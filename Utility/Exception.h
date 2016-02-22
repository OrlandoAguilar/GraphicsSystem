#pragma once

#include <exception>
#include <string>

namespace GE{
	class Exception
	{
	public:
		virtual ~Exception(){}

		Exception(std::string arg) :
			messagge(arg)
		{}

		operator char const*(){
			return messagge.c_str();
		}

		virtual  char const * what() const throw()
		{
			return messagge.c_str();
		}

	private:
		std::string messagge;
	};

	class MissingElementException :public Exception{
	public:
		MissingElementException(std::string arg):
		Exception("Missign Element Exception: "+arg)
		{}
	};

	class BadFormatException : public Exception{
	public:
		BadFormatException(std::string arg) :
			Exception("Bad Format Exception: " + arg)
		{}
	};

	class BadIdException : public Exception{
	public:
		BadIdException(std::string arg) :
			Exception("Bad Id Exception: " + arg)
		{}
	};

	class NoCapabilityException : public Exception{
	public:
		NoCapabilityException(std::string arg) :
			Exception("No Capability Exception: " + arg)
		{}
	};

	class IncompleteOperationException : public Exception{
	public:
		IncompleteOperationException(std::string arg) :
			Exception("Incomplete Operation Exception: " + arg)
		{}
	};

	class ShaderException : public Exception{
	public:
		ShaderException(std::string arg) :
			Exception("Shader Exception: " + arg)
		{}
	};

	class SoundException : public Exception{
	public:
		SoundException(std::string arg) :
			Exception("Sound Exception: " + arg)
		{}
	};

	class UnknownElementException : public Exception{
	public:
		UnknownElementException(std::string arg) :
			Exception("Unknown Element Exception: " + arg)
		{}
	};

	class BadParameterException : public Exception{
	public:
		BadParameterException(std::string arg) :
			Exception("Bad Parameter Exception: " + arg)
		{}
	};
	
}

