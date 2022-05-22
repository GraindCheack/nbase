import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'nbase_platform_interface.dart';

/// An implementation of [NbasePlatform] that uses method channels.
class MethodChannelNbase extends NbasePlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('nbase');

  @override
  Future<void> init() async {
    await methodChannel.invokeMethod<void>('init');
  }

  @override
  Future<void> saveDataBase() async {
    await methodChannel.invokeMethod<void>('saveDataBase');
  }

  @override
  Future<List<String>?> getTables() async {
    final data = await methodChannel.invokeMethod<List<Object?>>('getTables');
    return data
        ?.map((e) {
          if (e is String) {
            return e;
          }
          return '';
        })
        .where((e) => e.isNotEmpty)
        .toList();
  }

  @override
  Future<bool?> createTable(String name) async {
    return await methodChannel.invokeMethod<bool>('createTable', name);
  }

  @override
  Future<bool?> insertRow(String tableName, Map<String, String> row) async {
    return await methodChannel.invokeMethod<bool>('insertRow', {'row': row, 'name': tableName});
  }

  @override
  Future<bool?> remove(String tableName, Map<String, String> equals, {isAll = false}) async {
    return await methodChannel.invokeMethod<bool>('remove', {'equals': equals, 'name': tableName, 'isAll': isAll});
  }

  @override
  Future<List<Map<String, String>>?> select(String tableName, Map<String, String> equals,
      {int? from, int? count, isAll = false}) async {
    final data = await methodChannel.invokeMethod<List<Object?>>(
        'select', {'equals': equals, 'name': tableName, 'from': from, 'count': count, 'isAll': isAll});

    if (data == null) {
      return [];
    }
    final a = data
        .map((e) {
          if (e is Map) {
            return e.map((key, value) => MapEntry(key is String ? key : '', value is String ? value : ''));
          }
          return {'': ''};
        })
        .where((e) => e.keys.length > 1 || e.entries.first.key.isNotEmpty)
        .toList();

    return a;
  }
}
