import 'package:http/http.dart' as http;
import 'dart:convert';
import '../models/device.dart';

class SmartHomeApiService {
  final String baseUrl;

  SmartHomeApiService({required this.baseUrl});

  // Toggle Pump
  Future<bool> togglePump(bool turnOn) async {
    try {
      final endpoint = turnOn ? '/pump/on' : '/pump/off';
      final response = await http.get(Uri.parse('$baseUrl$endpoint')).timeout(
        const Duration(seconds: 5),
      );
      
      if (response.statusCode == 200) {
        return true;
      }
      return false;
    } catch (e) {
      return false;
    }
  }

  // Toggle Fan
  Future<bool> toggleFan(bool turnOn) async {
    try {
      final endpoint = turnOn ? '/fan/on' : '/fan/off';
      final response = await http.get(Uri.parse('$baseUrl$endpoint')).timeout(
        const Duration(seconds: 5),
      );
      
      if (response.statusCode == 200) {
        return true;
      }
      return false;
    } catch (e) {
      return false;
    }
  }

  // Set Pump Delay
  Future<bool> setPumpDelay(int minutes) async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/pump/delay/$minutes'),
      ).timeout(const Duration(seconds: 5));
      
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Set Fan Delay
  Future<bool> setFanDelay(int minutes) async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/fan/delay/$minutes'),
      ).timeout(const Duration(seconds: 5));
      
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Get All Devices Status
  Future<Map<String, DeviceStatus>?> getAllStatus() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/status'),
      ).timeout(const Duration(seconds: 5));

      if (response.statusCode == 200) {
        final data = json.decode(response.body);
        return {
          'pump': DeviceStatus(
            isOn: data['pump']['state'] == 'on',
            delayRemaining: data['pump']['delay'] ?? 0,
          ),
          'fan': DeviceStatus(
            isOn: data['fan']['state'] == 'on',
            delayRemaining: data['fan']['delay'] ?? 0,
          ),
        };
      }
      return null;
    } catch (e) {
      return null;
    }
  }

  // Ping ESP01
  Future<bool> ping() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/'),
      ).timeout(const Duration(seconds: 3));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }
}