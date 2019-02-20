#ifndef STREAMING_EXCEPTION_H
#define STREAMING_EXCEPTION_H

#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <apr.h>

class StreamingException : public std::runtime_error
{
public:
  StreamingException(const std::string filename = "", apr_uint32_t = 0) :
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

public: // fields
  std::string  filename_;
  apr_uint32_t line_;

private:
  mutable std::auto_ptr<std::stringstream> ss_;
  mutable std::string s_;
};

#endif


