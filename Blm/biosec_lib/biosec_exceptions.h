#ifndef __biosec_exceptions_h__
#define __biosec_exceptions_h__

#include <exception>
#include <stdexcept>


namespace blm_utils{

	class ArchiveInternalError : public std::logic_error{
	public:
		ArchiveInternalError()
			:std::logic_error("") {}
		ArchiveInternalError(const char* msg)
			:std::logic_error(msg) {}
	};


	class LibArchiveInternalError : public std::logic_error{
	public:
		LibArchiveInternalError(const char* msg)
			:std::logic_error(msg) {}
	};


	class UnknownEnumValue : public std::logic_error{
	public:
		UnknownEnumValue()
			:std::logic_error("") {}
		UnknownEnumValue(const char* msg)
			:std::logic_error(msg) {}
	};


	class InvalidArgument : public std::logic_error{
	public:
		InvalidArgument(const char* msg)
			:std::logic_error(msg) {}
	};


	class InvalidMode : public std::logic_error{
	public:
		InvalidMode(const char* msg)
			:std::logic_error(msg) {}
	};


	class NotImplemented : public std::logic_error{
	public:
		NotImplemented()
			:std::logic_error("") {}
		NotImplemented(const char* msg)
			:std::logic_error(msg) {}
	};


}


#endif // __biosec_exceptions_h__