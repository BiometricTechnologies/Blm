#ifndef OBJECT_ADAPTER_H
#define OBJECT_ADAPTER_H

#include "plugin.h"

// This interface is used to adapt C plugin objects to C++ plugin objects.
// It must be passed to the PluginManager::createObject() function.
struct IObjectAdapter
{
  virtual ~IObjectAdapter() {}
  virtual void * adapt(void * object, PF_DestroyFunc df) = 0;
};

// This template should be used if the object model implements the
// dual C/C++ object design pattern. Otherwise you need to provide
// your own object adapter class that implements IObjectAdapter
template<typename T, typename U>
struct ObjectAdapter : public IObjectAdapter
{
  virtual void * adapt(void * object, PF_DestroyFunc df)
  {
    return new T((U *)object, df);
  }
};

#endif // OBJECT_ADAPTER_H
