#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

#include <string>

class DynamicLibrary
{
public:

  static DynamicLibrary * load(const std::string & path, 
                               std::string &errorString);
  ~DynamicLibrary();
  
  void * getSymbol(const std::string & name);

private:
  DynamicLibrary();
  
  DynamicLibrary(void * handle);
  DynamicLibrary(const DynamicLibrary &);
  
private:
  void * handle_;  
};

#endif
