
#ifndef PF_PLUGIN_H
#define PF_PLUGIN_H

#include <apr_general.h>
#include "fp_device.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum PF_ProgrammingLanguage
{
  PF_ProgrammingLanguage_CPP
} PF_ProgrammingLanguage;

 
struct PF_PlatformServices;

typedef struct PF_ObjectParams
{
  const apr_byte_t * objectType;
  const struct PF_PlatformServices * platformServices;
} PF_ObjectParams;

typedef struct PF_PluginAPI_Version
{
  apr_int32_t major;
  apr_int32_t minor;
} PF_PluginAPI_Version;

typedef std::shared_ptr<blm_login::IFingerprintServices> (*PF_CreateFunc)(PF_ObjectParams *); 
typedef apr_int32_t (*PF_DestroyFunc)(void *);

typedef struct PF_RegisterParams
{
  PF_PluginAPI_Version version;
  PF_CreateFunc createFunc;
  PF_ProgrammingLanguage programmingLanguage;
} PF_RegisterParams;


typedef apr_int32_t (*PF_RegisterFunc)(const apr_byte_t * nodeType, const PF_RegisterParams * params);
typedef apr_int32_t (*PF_InvokeServiceFunc)(const apr_byte_t * serviceName, void * serviceParams);

typedef struct PF_PlatformServices
{
  PF_PluginAPI_Version version;
  PF_RegisterFunc registerObject; 
  PF_InvokeServiceFunc invokeService; 
} PF_PlatformServices;


typedef apr_int32_t (*PF_ExitFunc)();

/** Type definition of the PF_initPlugin function bellow (used by PluginManager to initialize plugins)
 * Note the return type is the PF_ExitFunc (used by PluginManager to tell plugins to cleanup). If 
 * the initialization failed for any reason the plugin may report the error via the error reporting
 * function of the provided platform services. Nevertheless, it must return NULL exit func in this case
 * to let the plugin manger that the plugin wasn't initialized properly. The plugin may use the runtime
 * services - allocate memory, log messages and of course register node types.
 *
 * @param  [const PF_PlatformServices *] params - the platform services struct 
 * @retval [PF_ExitFunc] the exit func of the plugin or NULL if initialization failed.
 */
typedef PF_ExitFunc (*PF_InitFunc)(const PF_PlatformServices *);

/** 
 * Named exported entry point into the plugin
 * This definition is required eventhough the function 
 * is loaded from a dynamic library by name
 * and casted to PF_InitFunc. If this declaration is 
 * commented out DynamicLibrary::getSymbol() fails
 *
 * The plugin's initialization function MUST be called "PF_initPlugin"
 * (and conform to the signature of course).
 *
 * @param  [const PF_PlatformServices *] params - the platform services struct 
 * @retval [PF_ExitFunc] the exit func of the plugin or NULL if initialization failed.
 */

#ifndef PLUGIN_API
  #ifdef WIN32
    #define PLUGIN_API __declspec(dllimport)
  #else
    #define PLUGIN_API
  #endif
#endif

extern
#ifdef  __cplusplus
"C" 
#endif
PLUGIN_API PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);

#ifdef  __cplusplus
}
#endif

#endif /* PF_PLUGIN_H */

