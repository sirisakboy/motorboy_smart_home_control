import 'package:http/http.dart' as http;
import 'dart:convert';

class AiService {
  static const String _baseUrl = 'https://text.pollinations.ai/';

  Future<String> getAiInsight(String prompt) async {
    try {
      final response = await http.post(
        Uri.parse(_baseUrl),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'messages': [
            {'role': 'system', 'content': 'You are a smart home energy assistant. Analyze the given energy usage data and provide concise, actionable tips for energy saving.'},
            {'role': 'user', 'content': prompt}
          ],
          'model': 'gpt-4o',
          'seed': 42,
        }),
      );

      if (response.statusCode == 200) {
        return response.body;
      } else {
        return 'Error: Could not fetch AI insights.';
      }
    } catch (e) {
      return 'Error: $e';
    }
  }

  Future<String> getIntelligentControlSuggestion(String deviceStates) async {
    // This prompt asks the AI to decide on device states based on energy usage
    final prompt = 'Current device states and usage: $deviceStates. '
        'Based on this, suggest whether to turn devices ON or OFF for energy optimization. '
        'Return ONLY JSON format: {"device_id": "state_string"}.';
    
    return await getAiInsight(prompt);
  }
}
