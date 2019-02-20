
#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <vector>
#include <map>
#include <apr.h>
#include <boost/shared_ptr.hpp>
#include "plugin.h"

class DynamicLibrary;
struct IObjectAdapter;

class PluginManager
{
  typedef std::map<std::string, boost::shared_ptr<DynamicLibrary> > DynamicLibraryMap; 
  typedef std::vector<PF_ExitFunc> ExitFuncVec;  
  typedef std::vector<PF_RegisterParams> RegistrationVec; 

public:   
  typedef std::map<std::string, PF_RegisterParams> RegistrationMap;

  static PluginManager & getInstance();
  static apr_int32_t initializePlugin(PF_InitFunc initFunc);
  apr_int32_t loadAll(const std::string & pluginDirectory, PF_InvokeServiceFunc func = NULL);
  apr_int32_t loadByPath(const std::string & path);

  void * createObject(const std::string & objectType, IObjectAdapter & adapter);

  apr_int32_t shutdown();  
  static apr_int32_t registerObject(const apr_byte_t * nodeType, 
                                    const PF_RegisterParams * params);
  const RegistrationMap & getRegistrationMap();

  // TODO: make setter instead of assigning via this method returned link(an.skornyakov@gmail.com)
  // TODO: don't return reference to internal elements! (an.skornyakov@gmail.com)
  PF_PlatformServices getPlatformServices();
  void setPlatformServices(const PF_PlatformServices& services);

private:
  ~PluginManager();    
  PluginManager();
  PluginManager(const PluginManager &);
  
  DynamicLibrary * loadLibrary(const std::string & path, std::string & errorString);
private:
  bool                inInitializePlugin_;
  PF_PlatformServices platformServices_;
  DynamicLibraryMap   dynamicLibraryMap_;
  ExitFuncVec         exitFuncVec_;

  RegistrationMap     tempExactMatchMap_;   // register exact-match object types 
  RegistrationVec     tempWildCardVec_;     // wild card ('*') object types

  RegistrationMap     exactMatchMap_;   // register exact-match object types 
  RegistrationVec     wildCardVec_;     // wild card ('*') object types
};


#endif
