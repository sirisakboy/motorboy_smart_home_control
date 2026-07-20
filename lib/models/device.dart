// Device Model
class SmartDevice {
  final String id;
  final String name;
  final String type; // 'pump' or 'fan'
  final String ipAddress;
  bool isOn;
  int delayMinutes;
  final double powerRating; // Watts

  SmartDevice({
    required this.id,
    required this.name,
    required this.type,
    required this.ipAddress,
    this.isOn = false,
    this.delayMinutes = 0,
    this.powerRating = 0.0,
  });

  factory SmartDevice.fromJson(Map<String, dynamic> json) {
    return SmartDevice(
      id: json['id'] ?? '',
      name: json['name'] ?? '',
      type: json['type'] ?? '',
      ipAddress: json['ipAddress'] ?? '',
      isOn: json['isOn'] ?? false,
      delayMinutes: json['delayMinutes'] ?? 0,
      powerRating: (json['powerRating'] ?? 0.0).toDouble(),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      'type': type,
      'ipAddress': ipAddress,
      'isOn': isOn,
      'delayMinutes': delayMinutes,
      'powerRating': powerRating,
    };
  }
}
...
// Device Status Response
class DeviceStatus {
  final bool isOn;
  final int delayRemaining;

  DeviceStatus({required this.isOn, required this.delayRemaining});

  factory DeviceStatus.fromJson(Map<String, dynamic> json) {
    return DeviceStatus(
      isOn: json['state'] == 'on',
      delayRemaining: json['delay_remaining'] ?? 0,
    );
  }
}