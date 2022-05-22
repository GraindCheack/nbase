#include "include/nbase/nbase_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "nbase_plugin.h"

void NbasePluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  nbase::NbasePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
