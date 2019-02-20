

#ifndef IERROR_HANDLER_H
#define IERROR_HANDLER_H

#include <string>

struct IErrorHandler
{
  virtual ~IErrorHandler() {}    
  virtual void handleError(const std::string & filename, apr_init32_t lineno, const std::string & message) = 0;
};
  
#endif

