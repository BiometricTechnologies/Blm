#include "base.h"
#include <apr.h>
#include <apr_errno.h>

namespace base
{
  std::string getErrorMessage()
  {
    char buff[1024];
    apr_status_t st = apr_get_os_error();
    ::apr_strerror(st , buff, 1024);
    return std::string(buff);
  }
}


