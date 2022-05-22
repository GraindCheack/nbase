import 'nbase_platform_interface.dart';

class Nbase {
  Future<void> init() {
    return NbasePlatform.instance.init();
  }

  Future<void> saveDataBase() {
    return NbasePlatform.instance.saveDataBase();
  }

  Future<List<String>?> getTables() {
    return NbasePlatform.instance.getTables();
  }

  Future<bool?> createTable(String name) {
    return NbasePlatform.instance.createTable(name);
  }

  Future<bool?> insertRow(String tableName, Map<String, String> row) {
    return NbasePlatform.instance.insertRow(tableName, row);
  }

  Future<List<Map<String, String>>?> select(String tableName, Map<String, String> equals,
      {int? from, int? count, isAll = false}) {
    return NbasePlatform.instance.select(tableName, equals, from: from, count: count, isAll: isAll);
  }

  Future<bool?> remove(String tableName, Map<String, String> equals, {isAll = false}) {
    return NbasePlatform.instance.remove(tableName, equals, isAll: isAll);
  }
}
