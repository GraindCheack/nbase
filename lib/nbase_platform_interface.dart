import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'nbase_method_channel.dart';

abstract class NbasePlatform extends PlatformInterface {
  /// Constructs a NbasePlatform.
  NbasePlatform() : super(token: _token);

  static final Object _token = Object();

  static NbasePlatform _instance = MethodChannelNbase();

  /// The default instance of [NbasePlatform] to use.
  ///
  /// Defaults to [MethodChannelNbase].
  static NbasePlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [NbasePlatform] when
  /// they register themselves.
  static set instance(NbasePlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<void> init() {
    throw UnimplementedError('init() has not been implemented.');
  }

  Future<List<String>?> getTables() {
    throw UnimplementedError('getTables() has not been implemented.');
  }

  Future<void> saveDataBase() {
    throw UnimplementedError('saveDataBase() has not been implemented.');
  }

  Future<bool?> createTable(String name) {
    throw UnimplementedError('createTable() has not been implemented.');
  }

  Future<bool?> insertRow(String tableName, Map<String, String> row) {
    throw UnimplementedError('insertRow() has not been implemented.');
  }

  Future<List<Map<String, String>>?> select(String tableName, Map<String, String> equals,
      {int? from, int? count, isAll = false}) {
    throw UnimplementedError('select() has not been implemented.');
  }

  Future<bool?> remove(String tableName, Map<String, String> equals, {isAll = false}) {
    throw UnimplementedError('remove() has not been implemented.');
  }
}
