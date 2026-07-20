import 'dart:async';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

class MqttService {
  final String username;
  final String apiKey;
  late MqttServerClient _client;
  final _statusController = StreamController<Map<String, dynamic>>.broadcast();
  
  Stream<Map<String, dynamic>> get statusStream => _statusController.stream;

  MqttService({required this.username, required this.apiKey}) {
    _client = MqttServerClient.withPort(
      'io.adafruit.com',
      'smart_home_${DateTime.now().millisecondsSinceEpoch}',
      1883,
    );
    _client.logging(on: false);
    _client.keepAlivePeriod = 20;
    _client.onConnected = _onConnected;
    _client.onDisconnected = _onDisconnected;
    _client.onSubscribed = _onSubscribed;
  }

  void _onConnected() {
    print('MQTT Connected');
  }

  void _onDisconnected() {
    print('MQTT Disconnected');
  }

  void _onSubscribed(String topic) {
    print('Subscribed to $topic');
  }

  Future<bool> connect() async {
    try {
      await _client.connect(username, apiKey);
      return _client.connectionStatus?.state == MqttConnectionState.connected;
    } catch (e) {
      return false;
    }
  }

  void subscribeToStatus() {
    _client.subscribe('$username/status', MqttQos.atMostOnce);
    _client.updates?.listen(_onMessage);
  }

  void _onMessage(List<MqttReceivedMessage<MqttMessage?>>? messages) {
    final topic = messages![0].topic;
    final payload = messages[0].payload as MqttPublishMessage;
    final message = MqttPublishPayload.bytesToStringAsString(
      payload.payload.message,
    );
    
    if (topic.contains('status')) {
      _parseStatus(message);
    }
  }

  void _parseStatus(String message) {
    // Parse JSON: {"pump":"ON","fan":"OFF"}
    final pumpOn = message.contains('"pump":"ON"');
    final fanOn = message.contains('"fan":"ON"');
    
    _statusController.add({'pump': pumpOn, 'fan': fanOn});
  }

  Future<void> controlDevice(String device, bool turnOn) async {
    if (_client.connectionStatus?.state != MqttConnectionState.connected) {
      return;
    }

    final topic = '$username/$device';
    final message = turnOn ? 'ON' : 'OFF';
    
    final builder = MqttClientPayloadBuilder();
    builder.addString(message);
    
    _client.publishMessage(topic, MqttQos.atMostOnce, builder.payload!);
  }

  void disconnect() {
    _client.disconnect();
    _statusController.close();
  }
}