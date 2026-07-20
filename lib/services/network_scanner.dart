import 'dart:async';
import 'dart:io';
import 'package:network_info_plus/network_info_plus.dart';

class NetworkScanner {
  static Future<String> getLocalIp() async {
    final info = NetworkInfo();
    final wifiIP = await info.getWifiIP();
    return wifiIP ?? '192.168.1.100'; // fallback
  }

  static String getSubnet(String ip) {
    final parts = ip.split('.');
    if (parts.length == 4) {
      return '${parts[0]}.${parts[1]}.${parts[2]}.';
    }
    return '192.168.1.';
  }

  static Future<List<String>> scanEsp01({
    int timeoutSeconds = 2,
    int concurrency = 10,
  }) async {
    final localIp = await getLocalIp();
    final subnet = getSubnet(localIp);
    final List<String> foundDevices = [];
    
    // Scan 1-254 IPs
    final futures = <Future>[];
    final semaphore = Semaphore(concurrency);
    
    for (int i = 1; i <= 254; i++) {
      final targetIp = '$subnet$i';
      futures.add(semaphore.call(() async {
        if (await _pingDevice(targetIp, timeoutSeconds)) {
          foundDevices.add(targetIp);
        }
      }));
    }
    
    await Future.wait(futures);
    return foundDevices;
  }

  static Future<bool> _pingDevice(String ip, int timeoutSeconds) async {
    try {
      final socket = await Socket.connect(
        ip,
        80,
        timeout: Duration(seconds: timeoutSeconds),
      );
      socket.destroy();
      return true;
    } catch (_) {
      return false;
    }
  }
}

class Semaphore {
  final int _maxConcurrency;
  int _current = 0;

  Semaphore(this._maxConcurrency);

  Future<T> call<T>(Future<T> Function() fn) async {
    while (_current >= _maxConcurrency) {
      await Future.delayed(const Duration(milliseconds: 10));
    }
    _current++;
    try {
      return await fn();
    } finally {
      _current--;
    }
  }
}