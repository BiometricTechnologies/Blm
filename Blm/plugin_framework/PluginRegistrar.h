
#ifndef PLUGIN_REGISTRAR_H
#define PLUGIN_REGISTRAR_H

#include "plugin_framework/PluginManager.h"

struct PluginRegistrar
{
  PluginRegistrar(PF_InitFunc initFunc)
  {
    PluginManager::initializePlugin(initFunc);
  }
};




#endif // PLUGIN_REGISTRAR_H
