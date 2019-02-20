#ifndef PF_PLUGIN_HELPER_H
#define PF_PLUGIN_HELPER_H

#include "plugin.h"
#include "base.h"

class PluginHelper
{  
  struct RegisterParams : public PF_RegisterParams
  {    
    RegisterParams(PF_PluginAPI_Version v,
                         PF_CreateFunc cf,
                         PF_DestroyFunc df,
                         PF_ProgrammingLanguage pl)
    {
      version=v;
      createFunc=cf;
      destroyFunc=df;
      programmingLanguage=pl;
    }
  };  
public:
  PluginHelper(const PF_PlatformServices * params) : 
    params_(params),
    result_(exitPlugin)
  {
  }

  PF_ExitFunc getResult()
  {
    return result_;
  }  

  template <typename T>
  void registerObject(const apr_byte_t * objectType, 
                      PF_ProgrammingLanguage pl=PF_ProgrammingLanguage_C)
  {
    PF_PluginAPI_Version v = {1, 0};
    
    // Version check
    try
    {
      CHECK (params_->version.major >= v.major) 
        << "Version mismatch. PluginManager version must be "
        << "at least " << v.major << "." << v.minor;

      RegisterParams rp(v, T::create, T::destroy, pl);
      apr_int32_t rc = params_->registerObject(objectType, &rp);
      
      CHECK (rc >= 0) 
        << "Registration of object type " 
        << objectType << "failed. "
         << "Error code=" << rc;
    }
    catch (...)
    {
      result_ = NULL;
    }
  }

  static apr_int32_t exitPlugin()
  {
    return 0;
  }

private:
  const PF_PlatformServices * params_;
  PF_ExitFunc result_;
}; 

#endif // PF_PLUGIN_HELPER_H

