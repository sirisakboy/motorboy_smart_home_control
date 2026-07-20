import 'package:http/http.dart' as http;
import 'dart:convert';

// Simplified API service for single relay ESP01
class SimpleRelayService {
  final String baseUrl;

  SimpleRelayService({required this.baseUrl});

  // Toggle relay on/off
  Future<bool> setRelay(bool turnOn) async {
    try {
      final endpoint = turnOn ? '/api/relay/on' : '/api/relay/off';
      final response = await http.get(Uri.parse('$baseUrl$endpoint')).timeout(
        const Duration(seconds: 5),
      );
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Toggle relay (switch state)
  Future<bool> toggleRelay() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/relay/toggle'),
      ).timeout(const Duration(seconds: 5));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Get relay status
  Future<bool?> getRelayStatus() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/status'),
      ).timeout(const Duration(seconds: 3));

      if (response.statusCode == 200) {
        final data = json.decode(response.body);
        return data['relay'] == 'on';
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
        Uri.parse('$baseUrl/api/status'),
      ).timeout(const Duration(seconds: 3));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Set schedule for auto on
  Future<bool> setScheduleOn(int minutes) async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/schedule/on/$minutes'),
      ).timeout(const Duration(seconds: 5));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Set schedule for auto off
  Future<bool> setScheduleOff(int minutes) async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/schedule/off/$minutes'),
      ).timeout(const Duration(seconds: 5));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Clear all schedules
  Future<bool> clearSchedule() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/schedule/clear'),
      ).timeout(const Duration(seconds: 5));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }

  // Enable schedule on ESP01
  Future<bool> enableSchedule() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/schedule/enable'),
      ).timeout(const Duration(seconds: 5));
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }
}