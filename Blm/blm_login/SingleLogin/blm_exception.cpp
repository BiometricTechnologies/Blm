#include <string>
#include "blm_exception.h"

namespace blm_login{

	const char *BlmException::kDefaultMessagePrefix_ = "Unknown blm_exception";

	BlmException::BlmException()
		:std::exception(kDefaultMessagePrefix_),
		isLocationKnown_(false)
	{		
	}

	BlmException::BlmException( const char* message )
		:std::exception(message),
		isLocationKnown_(false)
	{

	}
	BlmException::BlmException( const char* message, const char* fileName ,const char* functionName, std::int32_t line )
		:std::exception(message),
		isLocationKnown_(true),
		fileName_(fileName),
		functionName_(functionName),
		line_(line)
	{

	}

	BlmException::BlmException( BlmException&& orig )
		:isLocationKnown_(orig.isLocationKnown_),
		fileName_(std::move(orig.fileName_)),
		functionName_(std::move(orig.functionName_)),
		line_(line_)
	{

	}

	BlmException::~BlmException()
	{

	}

	const char* BlmException::what() const
	{
		std::string whatMessage(std::exception::what());
		if(isLocationKnown_){
			whatMessage = whatMessage + " file; " + fileName_ + " function: " + functionName_ + " line: " + std::to_string(line_);
		}
		return whatMessage.c_str();
	}



}
