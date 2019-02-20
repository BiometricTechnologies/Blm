#ifndef STREAMING_EXCEPTION
#define STREAMING_EXCEPTION

#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>

class StreamingException : public std::runtime_error
{
public:
 StreamingException() :
   std::runtime_error(""),
   ss_(std::auto_ptr<std::stringstream>
       (new std::stringstream()))
 {
 }

 ~StreamingException() throw()
 {
 }

 template <typename T>
 StreamingException & operator << (const T & t)
 {
   (*ss_) << t;
   return *this;
 }

 virtual const char * what() const throw()
 {
   s_ = ss_->str();
   return s_.c_str();
 }

private:
 mutable std::auto_ptr<std::stringstream> ss_;
 mutable std::string s_;
};
#endif


