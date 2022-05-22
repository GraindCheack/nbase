#include "nbase_plugin.h"

#include <cstddef> // NULL
#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <random>
#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace nbase
{

  std::string random_string()
  {
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(str.begin(), str.end(), generator);

    return str.substr(0, 3); // assumes 32 < number of characters in str
  }

  bool isRowEqual(std::map<std::string, std::string> row, std::map<std::string, std::string> equals)
  {
    for (auto const &equal : equals)
    {
      auto it = row.find(equal.first);
      if (it == row.end() || equal.second != it->second)
      {
        return false;
      }
    }
    return true;
  }

  class Table
  {
    friend class boost::serialization::access;
    friend std::ostream &operator<<(std::ostream &os, const Table &t);

    std::list<std::map<std::string, std::string>> dataRows;
    std::string name;
    int lastRowId;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar &lastRowId;
      ar &name;
      ar &dataRows;
    }

  public:
    Table(int _lastTableId, std::string _name) : lastRowId(_lastTableId), name(_name){};
    Table(std::string _name) : lastRowId(0), name(_name){};
    Table() : lastRowId(0), name(""){};

    bool isValid()
    {
      return name.length() > 0;
    }

    std::list<std::map<std::string, std::string>> getDataRows()
    {
      return dataRows;
    }

    int getDataRowsLength()
    {
      return static_cast<int>(dataRows.size());
    }

    std::string getName()
    {
      return name;
    }

    void addRow(std::map<std::string, std::string> row)
    {
      std::stringstream lastRorIdStrStream;
      lastRorIdStrStream << lastRowId;

      row.insert(std::make_pair("row_id", lastRorIdStrStream.str()));
      dataRows.insert(dataRows.end(), row);

      lastRowId++;
    }

    void removeRow(std::map<std::string, std::string> equals)
    {
      for (auto i = dataRows.begin(); i != dataRows.end(); i++)
      {
        if (isRowEqual(*i, equals))
        {
          dataRows.erase(i);
          return;
        }
      }
    }

    void removeAllRows(std::map<std::string, std::string> equals)
    {
      dataRows.remove_if([equals](std::map<std::string, std::string> row)
                         { return isRowEqual(row, equals); });
    }
  };
  std::ostream &operator<<(std::ostream &os, const Table &t)
  {
    std::list<std::map<std::string, std::string>>::const_iterator it;

    os << "Last table id: " << t.lastRowId << "\n";

    for (it = t.dataRows.begin(); it != t.dataRows.end(); it++)
    {
      for (auto const &pair : *it)
      {
        os << "" << pair.first << ": " << pair.second << "; ";
      }
      os << '\n';
    }
    return os;
  }

  class TableInfo
  {
    friend class boost::serialization::access;
    friend std::ostream &operator<<(std::ostream &os, const TableInfo &gp);

    std::string name;
    std::string fileName;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar &name;
      ar &fileName;
    }

  public:
    TableInfo() : name(""), fileName(""){};
    TableInfo(std::string _name) : name(_name), fileName(""){};
    TableInfo(std::string _name, std::string _fileName) : name(_name), fileName(_fileName){};

    bool isValid()
    {
      return name.length() > 0 && fileName.length() > 0;
    }

    void setFileName(std::string _fileName)
    {
      if (_fileName.length() != 0)
      {
        fileName = _fileName;
      }
    }

    std::string getName() { return name; }
    std::string getFileName() { return fileName; }
  };

  std::ostream &operator<<(std::ostream &os, const TableInfo &bs)
  {
    return os << "Name: " << bs.name << ", file name: " << bs.fileName;
  }

  class Settings
  {
    friend class boost::serialization::access;
    friend std::ostream &operator<<(std::ostream &os, const Settings &br);

    std::map<std::string, Table> activeTablesMap;

    int lastTableId;
    std::map<std::string, TableInfo> tables;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar &tables;
      ar &lastTableId;
    }

    std::string getNewTableFileName()
    {
      std::stringstream lastTableIdStrStream;
      lastTableIdStrStream << lastTableId;
      return lastTableIdStrStream.str() + "n" + random_string();
    }

  public:
    Settings(int _lastTableId) : lastTableId(_lastTableId){};
    Settings() : lastTableId(0){};

    void createTable(std::string tableName)
    {
      tables.insert(std::make_pair(tableName, TableInfo(tableName, getNewTableFileName())));
      lastTableId++;
    }

    void addActiveTable(Table *t)
    {
      const auto it = activeTablesMap.find(t->getName());
      if (it != activeTablesMap.end())
      {
        it->second = *t;
      }
      activeTablesMap.insert(std::make_pair(t->getName(), *t));
    }

    void getTable(Table &t, std::string name)
    {
      auto it = activeTablesMap.find(name);
      if (it != activeTablesMap.end())
      {
        t = it->second;
      }
    }

    void getTableInfo(TableInfo &ti, std::string tableName)
    {
      auto it = tables.find(tableName);
      if (it != tables.end())
      {
        ti = it->second;
      }
    }

    std::vector<std::string> getTableList()
    {
      std::vector<std::string> tablesList;
      for (const auto it : tables)
      {
        auto table = it.second;
        if (!table.getName().empty())
        {
          tablesList.insert(tablesList.end(), table.getName());
        }
      }
      return tablesList;
    }
  };
  std::ostream &operator<<(std::ostream &os, const Settings &br)
  {
    os << "Last table id: " << br.lastTableId;

    for (auto const &pair : br.tables)
    {
      os << "" << pair.first << ": " << pair.second << "; ";
    }
    return os;
  }

  class Nbase
  {
  private:
    Settings settings;
    const std::string mainFileName = "m";

    void _saveSettings()
    {
      std::ofstream ofs(mainFileName);
      boost::archive::text_oarchive oa(ofs);
      oa << settings;
    }

    void _restoreSettings()
    {
      std::ifstream ifs(mainFileName);

      if (!ifs)
      {
        Settings *newSettings = new Settings();

        settings = *newSettings;
        _saveSettings();
        return;
      }

      boost::archive::text_iarchive ia(ifs);
      ia >> settings;
    }

    void _saveTable(const Table &t, const char *filename)
    {
      std::ofstream ofs(filename);
      boost::archive::text_oarchive oa(ofs);
      oa << t;
    }

    void _restoreTable(Table &t, TableInfo ti)
    {
      std::ifstream ifs(ti.getFileName().c_str());

      if (!ifs)
      {
        Table *newTable = new Table(ti.getName());

        _saveTable(*newTable, ti.getFileName().c_str());
        t = *newTable;
        return;
      }

      boost::archive::text_iarchive ia(ifs);
      ia >> t;
    }

    void _insertRowInTable(Table *t, std::map<std::string, std::string> row, std::string fileName)
    {
      t->addRow(row);
      _saveTable(*t, fileName.c_str());
      settings.addActiveTable(t);
    }

    void _getTable(Table &t, TableInfo &ti, std::string tableName)
    {
      settings.getTableInfo(ti, tableName);
      settings.getTable(t, tableName);

      if (!ti.isValid())
      {
        settings.createTable(tableName);
        settings.getTableInfo(ti, tableName);
        saveBase();
      }

      if (!ti.isValid())
      {
        return;
      }

      if (!t.isValid())
      {
        _restoreTable(t, ti);
      }
    }

    std::list<std::map<std::string, std::string>> _selectRows(Table *t, int from, int count, std::map<std::string, std::string> equals)
    {
      auto rows = t->getDataRows();
      std::list<std::map<std::string, std::string>> newRows;

      for (auto const row : rows)
      {
        if (isRowEqual(row, equals) && from-- <= 0)
        {
          if (newRows.size() >= count)
          {
            break;
          }
          newRows.insert(newRows.end(), row);
        }
      }

      settings.addActiveTable(t);
      return newRows;
    }

    void _removeRow(Table *t, std::map<std::string, std::string> equals, std::string fileName)
    {
      t->removeRow(equals);
      _saveTable(*t, fileName.c_str());
      settings.addActiveTable(t);
    }

    void _removeAllRows(Table *t, std::map<std::string, std::string> equals, std::string fileName)
    {
      t->removeAllRows(equals);
      _saveTable(*t, fileName.c_str());
      settings.addActiveTable(t);
    }

  public:
    Nbase()
    {
      _restoreSettings();
    }

    void createTable(std::string name)
    {
      settings.createTable(name);
      _saveSettings();
    }

    bool insertRowInTable(std::string tableName, std::map<std::string, std::string> row)
    {
      Table t;
      TableInfo ti;

      _getTable(t, ti, tableName);

      if (t.isValid())
      {
        _insertRowInTable(&t, row, ti.getFileName());
        return true;
      }

      return false;
    }

    std::list<std::map<std::string, std::string>> selectRows(std::string tableName, int from, int count, std::map<std::string, std::string> equals)
    {
      Table t;
      TableInfo ti;

      _getTable(t, ti, tableName);

      if (t.isValid())
      {
        return _selectRows(&t, from, count, equals);
      }

      return std::list<std::map<std::string, std::string>>();
    }

    std::list<std::map<std::string, std::string>> selectRows(std::string tableName, std::map<std::string, std::string> equals)
    {
      Table t;
      TableInfo ti;

      _getTable(t, ti, tableName);

      if (t.isValid())
      {
        return _selectRows(&t, 0, t.getDataRowsLength(), equals);
      }

      return std::list<std::map<std::string, std::string>>();
    }

    bool isTableExist(std::string tableName)
    {
      TableInfo ti;
      settings.getTableInfo(ti, tableName);

      return ti.isValid();
    }

    void removeRow(std::string tableName, std::map<std::string, std::string> equals)
    {
      Table t;
      TableInfo ti;

      _getTable(t, ti, tableName);

      if (t.isValid())
      {
        return _removeRow(&t, equals, ti.getFileName());
      }
    }

    void removeAllRows(std::string tableName, std::map<std::string, std::string> equals)
    {
      Table t;
      TableInfo ti;

      _getTable(t, ti, tableName);

      if (t.isValid())
      {
        return _removeAllRows(&t, equals, ti.getFileName());
      }
    }

    std::vector<std::string> getTableList()
    {
      return settings.getTableList();
    }

    void saveBase()
    {
      _saveSettings();
    }
  };

  Nbase *dataBase;

  // static
  void NbasePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "nbase",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<NbasePlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result)
        {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  NbasePlugin::NbasePlugin() {}

  NbasePlugin::~NbasePlugin() {}

  void NbasePlugin::HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  {
    printf("asd");
    if (method_call.method_name().compare("init") == 0)
    {
      dataBase = new Nbase();
      result->Success();
    }
    else if (method_call.method_name().compare("saveDataBase") == 0)
    {
      result->Success();
    }
    else if (method_call.method_name().compare("createTable") == 0)
    {
      const auto *tableName = std::get_if<std::string>(method_call.arguments());

      if (tableName)
      {
        dataBase->createTable(*tableName);
        if (dataBase->isTableExist(*tableName))
        {
          result->Success(flutter::EncodableValue(true));
          return;
        }
      }
      result->Success(flutter::EncodableValue(false));
    }
    else if (method_call.method_name().compare("insertRow") == 0)
    {
      const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
      if (arguments)
      {
        std::string tableName = "";
        std::map<std::string, std::string> tableRow;
        auto tableRowIt = arguments->find(flutter::EncodableValue("row"));
        if (tableRowIt != arguments->end())
        {
          for (auto const it : std::get<flutter::EncodableMap>(tableRowIt->second))
          {
            tableRow.insert(std::make_pair(std::get<std::string>(it.first), std::get<std::string>(it.second)));
          }
        }

        auto tableNameIt = arguments->find(flutter::EncodableValue("name"));
        if (tableNameIt != arguments->end())
        {
          tableName = std::get<std::string>(tableNameIt->second);
        }

        if (!tableName.empty())
        {
          result->Success(flutter::EncodableValue(dataBase->insertRowInTable(tableName, tableRow)));
          return;
        }
      }
      result->Success(flutter::EncodableValue(false));
    }
    else if (method_call.method_name().compare("select") == 0)
    {
      const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
      if (arguments)
      {
        int from = -1;
        int count = -1;
        bool isAll = false;

        std::string tableName = "";
        std::map<std::string, std::string> equals;

        auto fromIt = arguments->find(flutter::EncodableValue("from"));
        if (fromIt != arguments->end())
        {
          from = std::get<int>(fromIt->second);
        }

        auto countIt = arguments->find(flutter::EncodableValue("count"));
        if (countIt != arguments->end())
        {
          count = std::get<int>(countIt->second);
        }

        auto isAllIt = arguments->find(flutter::EncodableValue("isAll"));
        if (isAllIt != arguments->end())
        {
          isAll = std::get<bool>(isAllIt->second);
        }

        auto equalsIt = arguments->find(flutter::EncodableValue("equals"));
        if (equalsIt != arguments->end())
        {
          for (auto const it : std::get<flutter::EncodableMap>(equalsIt->second))
          {
            equals.insert(std::make_pair(std::get<std::string>(it.first), std::get<std::string>(it.second)));
          }
        }

        auto tableNameIt = arguments->find(flutter::EncodableValue("name"));
        if (tableNameIt != arguments->end())
        {
          tableName = std::get<std::string>(tableNameIt->second);
        }

        if (!tableName.empty())
        {
          auto rows = isAll || from < 0 || count < 0 ? dataBase->selectRows(tableName, equals) : dataBase->selectRows(tableName, from, count, equals);
          flutter::EncodableList encodableRows;

          for (auto const row : rows)
          {
            flutter::EncodableMap encodableRow;
            for (auto const rowIt : row)
            {
              encodableRow.insert(std::make_pair(flutter::EncodableValue(rowIt.first), flutter::EncodableValue(rowIt.second)));
            }
            encodableRows.insert(encodableRows.end(), encodableRow);
          }

          result->Success(flutter::EncodableValue(encodableRows));
          return;
        }
      }
      result->Success(flutter::EncodableValue(flutter::EncodableList()));
    }
    else if (method_call.method_name().compare("remove") == 0)
    {
      const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
      if (arguments)
      {
        bool isAll = false;

        std::string tableName = "";
        std::map<std::string, std::string> equals;

        auto isAllIt = arguments->find(flutter::EncodableValue("isAll"));
        if (isAllIt != arguments->end())
        {
          isAll = std::get<bool>(isAllIt->second);
        }

        auto equalsIt = arguments->find(flutter::EncodableValue("equals"));
        if (equalsIt != arguments->end())
        {
          for (auto const it : std::get<flutter::EncodableMap>(equalsIt->second))
          {
            equals.insert(std::make_pair(std::get<std::string>(it.first), std::get<std::string>(it.second)));
          }
        }

        auto tableNameIt = arguments->find(flutter::EncodableValue("name"));
        if (tableNameIt != arguments->end())
        {
          tableName = std::get<std::string>(tableNameIt->second);
        }

        if (!tableName.empty())
        {
          if (isAll)
          {
            dataBase->removeAllRows(tableName, equals);
          }
          else
          {
            dataBase->removeRow(tableName, equals);
          }

          result->Success(flutter::EncodableValue(true));
          return;
        }
      }
      result->Success(flutter::EncodableValue(false));
    }
    else if (method_call.method_name().compare("getTables") == 0)
    {
      std::string tableName = "";
      std::map<std::string, std::string> equals;

      flutter::EncodableList tables;

      for(const auto it : dataBase->getTableList()) {
        tables.insert(tables.end(), it);
      }

      result->Success(flutter::EncodableValue(tables));
    }
    else
    {
      result->NotImplemented();
    }
  }

} // namespace nbase
