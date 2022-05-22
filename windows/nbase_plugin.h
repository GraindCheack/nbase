#ifndef FLUTTER_PLUGIN_NBASE_PLUGIN_H_
#define FLUTTER_PLUGIN_NBASE_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace nbase {

class NbasePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  NbasePlugin();

  virtual ~NbasePlugin();

  // Disallow copy and assign.
  NbasePlugin(const NbasePlugin&) = delete;
  NbasePlugin& operator=(const NbasePlugin&) = delete;

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace nbase

#endif  // FLUTTER_PLUGIN_NBASE_PLUGIN_H_
