import 'package:flutter/material.dart';
import 'dart:async';
import 'package:nbase/nbase.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final _nbasePlugin = Nbase();

  @override
  void dispose() {
    _nbasePlugin.saveDataBase();
    super.dispose();
  }

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    await _nbasePlugin.init();

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Row(
            children: [
              TextButton(
                onPressed: () async {
                  print(await _nbasePlugin.insertRow('customer', {'name': 'Allen', 'age': '10'}));
                },
                child: const Text('Insert'),
              ),
              TextButton(
                onPressed: () async {
                  print(await _nbasePlugin.createTable('customer'));
                },
                child: const Text('create'),
              ),
              TextButton(
                onPressed: () async {
                  print(await _nbasePlugin.select('customer', {'age': '10'}, from: 0, count: 1, isAll: true));
                },
                child: const Text('select'),
              ),
              TextButton(
                onPressed: () async {
                  print(await _nbasePlugin.remove('customer', {'age': '10'}));
                },
                child: const Text('remove'),
              ),
              TextButton(
                onPressed: () async {
                  print(await _nbasePlugin.getTables());
                },
                child: const Text('getTables'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
