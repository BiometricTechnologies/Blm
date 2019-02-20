// Custom exceptions hierarchy
// Anton Skornyakov an.skornyakov@gmail.com

#pragma once

#include <cstdint>
#include <exception>
#include <string>

namespace blm_login{

	class BlmException : public std::exception {
	public:
		BlmException();
		explicit BlmException(const char* message);
		BlmException(const char* message, const char* fileName ,const char* functionName, std::int32_t line);
		BlmException(BlmException&& orig);
		virtual ~BlmException();
		virtual const char* what() const;
	private:
		static const char* kDefaultMessagePrefix_;
		bool isLocationKnown_;
		std::string fileName_;
		std::string functionName_;
		std::int32_t line_;		
	};

}