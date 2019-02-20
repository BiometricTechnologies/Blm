
#include <string>

#include <apr_network_io.h> // for APR_PATH_MAX

#include "Directory.h"
#include "Path.h"
#include "PluginManager.h"
#include "DynamicLibrary.h"
#include "ObjectAdapter.h"



#if defined(PF_PLATFORM_MAC)
  static std::string dynamicLibraryExtension("dylib");
#elif defined(PF_PLATFORM_LINUX)
  static std::string dynamicLibraryExtension("so");
#elif defined(PF_PLATFORM_WINDOWS)
  static std::string dynamicLibraryExtension("dll");
#endif


// The registration params may be received from an external plugin so it is 
// crucial to validate it, because it was never subjected to our tests.
static bool isValid(const apr_byte_t * objectType, const PF_RegisterParams * params)
{
  if (!objectType || !(*objectType))
     return false;
  if (!params ||!params->createFunc)
    return false;
  
  return true;
}

// ---------------------------------------------------------------

apr_int32_t PluginManager::registerObject(const apr_byte_t * objectType, const PF_RegisterParams * params)
{
  // Check parameters
  if (!isValid(objectType, params))
    return -1;
 
  PluginManager & pm = PluginManager::getInstance();
  
  // Verify that versions match
  PF_PluginAPI_Version v = pm.platformServices_.version;
  if (v.major != params->version.major)
    return -1;
    
  std::string key((const char *)objectType);
  // If it's a wild card registration just add it
  if (key == std::string("*"))
  {
    pm.wildCardVec_.push_back(*params);
    return 0;
  }
  
  // If item already exists in eactMatc fail (only one can handle)
  if (pm.exactMatchMap_.find(key) != pm.exactMatchMap_.end())    
    return -1;
  
  pm.exactMatchMap_[key] = *params;
  return 0; 
}

// ---------------------------------------------------------------

PluginManager & PluginManager::getInstance()
{
  static PluginManager instance;
  
  return instance;
}


apr_int32_t PluginManager::loadAll(const std::string & pluginDirectory, PF_InvokeServiceFunc func)
{
  if (pluginDirectory.empty()) // Check that the path is non-empty.
    return -1;

  platformServices_.invokeService = func;

  Path dir_path(pluginDirectory);
  if (!dir_path.exists() || !dir_path.isDirectory())
    return -1;

  Directory::Entry e;
  Directory::Iterator di(dir_path);
  while (di.next(e))
  {
    Path full_path(dir_path + Path(e.path));

    // Skip directories
    if (full_path.isDirectory())
      continue;

    // Skip files with the wrong extension
    std::string ext = std::string(full_path.getExtension());
    if (ext != dynamicLibraryExtension)
      continue;

    // Ignore return value
    /*apr_int32_t res = */ loadByPath(std::string(full_path));
  }

  return 0;
}  

apr_int32_t PluginManager::initializePlugin(PF_InitFunc initFunc)
{
  PluginManager & pm = PluginManager::getInstance();

  PF_ExitFunc exitFunc = initFunc(&pm.platformServices_);
  if (!exitFunc)
    return -1;
  
  // Store the exit func so it can be called when unloading this plugin
  pm.exitFuncVec_.push_back(exitFunc);
  return 0;
}
 
PluginManager::PluginManager() : inInitializePlugin_(false)
{
  platformServices_.version.major = 1;
  platformServices_.version.minor = 0;
  platformServices_.invokeService = NULL; // can be populated during loadAll()
  platformServices_.registerObject = registerObject;
}

PluginManager::~PluginManager()
{
  // Just in case it wasn't called earlier
  shutdown();
}

apr_int32_t PluginManager::shutdown()
{
  apr_int32_t result = 0;
  for (ExitFuncVec::iterator func = exitFuncVec_.begin(); func != exitFuncVec_.end(); ++func)
  {
    try
    {
      result = (*func)();
    }
    catch (...)
    {
      result = -1;
    }
  }
  
  
  dynamicLibraryMap_.clear();
  exactMatchMap_.clear();
  wildCardVec_.clear();
  exitFuncVec_.clear();
  
  return result;
}

apr_int32_t PluginManager::loadByPath(const std::string & pluginPath)
{
    Path path(pluginPath);
    
    // Resolve symbolic links
    #ifndef WIN32 
    if (path.isSymbolicLink())
    {
      char buff[APR_PATH_MAX+1];
      int length = ::readlink(path, buff, APR_PATH_MAX);
      if (length < 0)
        return -1;
        
      path = std::string(buff, length);
    }
    #endif
             
    // Don't load the same dynamic library twice
    if (dynamicLibraryMap_.find(std::string(path)) != dynamicLibraryMap_.end())
      return -1;

    path.makeAbsolute();

    std::string errorString;
    DynamicLibrary * d = loadLibrary(std::string(path), errorString);
    if (!d) // not a dynamic library? 
      return -1;
                    
    // Get the NTA_initPlugin() function
    PF_InitFunc initFunc = (PF_InitFunc)(d->getSymbol("PF_initPlugin"));
    if (!initFunc) // dynamic library missing entry point?
      return -1;
    
    apr_int32_t res = initializePlugin(initFunc);
    if (res < 0) // failed to initalize?
      return res;
    
    return 0;
}


// ---------------------------------------------------------------

//template <typename T, typename U>
//T * PluginManager::createObject(const std::string & objectType, IObjectAdapter<T, U> & adapter)
//{
//  // "*" is not a valid object type
//  if (objectType == std::string("*"))
//    return NULL;
//  
//  // Prepare object params
//  PF_ObjectParams np;
//  np.objectType = objectType.c_str();
//  np.platformServices = &ServiceProvider::getInstance();
//
//  // Exact match found
//  if (exactMatchMap_.find(objectType) != exactMatchMap_.end())
//  {        
//    PF_RegisterParams & rp = exactMatchMap_[objectType];
//    IObject * object = createObject(rp, np, adapter);
//    if (object) // great, it worked
//      return object;
//  }
//  
//  for (Size i = 0; i < wildCardVec_.size(); ++i)
//  {
//    PF_RegisterParams & rp = wildCardVec_[i];
//    IObject * object = createObject(rp, np, adapter);
//    if (object) // great, it worked
//    {
//      // promote registration to exactMatc_ 
//      // (but keep also the wild card registration for other object types)
//      apr_int32_t res = registerObject(np.objectType, &rp);
//      if (res < 0)
//      {
//        PF_THROW << "PluginManager::createObject(" << np.objectType << "), registration failed";
//        delete object;
//        return NULL;
//      }
//      return object;
//    }      
//  }
//
//  // Too bad no one can create this objectType
//  return NULL;
//}


// ---------------------------------------------------------------


void * PluginManager::createObject(const std::string & objectType, IObjectAdapter & adapter)
{
  // "*" is not a valid object type
  if (objectType == std::string("*"))
    return NULL;
  
  // Prepare object params
  PF_ObjectParams np;
  np.objectType = (const apr_byte_t *)objectType.c_str();
  np.platformServices = &platformServices_;

  // Exact match found
  if (exactMatchMap_.find(objectType) != exactMatchMap_.end())
  {        
    PF_RegisterParams & rp = exactMatchMap_[objectType];
	std::shared_ptr<blm_login::IFingerprintServices> object = rp.createFunc(&np);
    //void * object = rp.createFunc(&np);
    if (object.get()) // great, there is an exact match
    {
	  // plain C API not supported yet
      // Adapt if necessary (wrap C objects using an adapter)
     /* if (rp.programmingLanguage == PF_ProgrammingLanguage_C)
        object = adapter.adapt(object, rp.destroyFunc);*/
        
      return object.get(); 
    }
  }
  
  // Try to find a wild card match
  for (apr_size_t i = 0; i < wildCardVec_.size(); ++i)
  {
    PF_RegisterParams & rp = wildCardVec_[i];
	std::shared_ptr<blm_login::IFingerprintServices> object = rp.createFunc(&np);
    //void * object = rp.createFunc(&np);
    if (object.get()) // great, it worked
    {
		// plain C API not supported yet
      // Adapt if necessary (wrap C objects using an adapter)
		/*
      if (rp.programmingLanguage == PF_ProgrammingLanguage_C)
        object = adapter.adapt(object, rp.destroyFunc);
		*/

      // promote registration to exactMatc_ 
      // (but keep also the wild card registration for other object types)
      apr_int32_t res = registerObject(np.objectType, &rp);
      if (res < 0)
      {  
        // Serious framework should report or log it              
        return NULL;
      }
      return object.get();
    }      
  }

  // Too bad no one can create this objectType
  return NULL;
}

DynamicLibrary * PluginManager::loadLibrary(const std::string &  path, std::string & errorString)
{
    DynamicLibrary * d = DynamicLibrary::load(path, errorString);
    if (!d) // not a dynamic library? 
      return NULL;
     
    // Add library to map, so it can be unloaded
    dynamicLibraryMap_[Path::makeAbsolute(path)] = boost::shared_ptr<DynamicLibrary>(d);
    return d;
}

const PluginManager::RegistrationMap & PluginManager::getRegistrationMap()
{
  return exactMatchMap_;
}

PF_PlatformServices PluginManager::getPlatformServices()
{
  return platformServices_;
}

void PluginManager::setPlatformServices( const PF_PlatformServices& services )
{
	platformServices_ = services;
}
