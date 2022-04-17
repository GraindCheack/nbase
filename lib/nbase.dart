
import 'dart:async';

import 'package:flutter/services.dart';

class Nbase {
  static const MethodChannel _channel = MethodChannel('nbase');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}
