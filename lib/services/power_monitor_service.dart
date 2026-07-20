import '../models/device.dart';

class PowerMonitorService {
  // Map device types to average consumption in Watts
  static const Map<String, double> _powerProfiles = {
    'pump': 150.0,
    'fan': 50.0,
    'light': 10.0,
  };

  static double getPowerRating(String deviceType) {
    return _powerProfiles[deviceType] ?? 0.0;
  }

  // Calculate current power consumption in Watts
  static double calculateCurrentUsage(List<SmartDevice> devices) {
    return devices
        .where((device) => device.isOn)
        .fold(0.0, (sum, device) => sum + device.powerRating);
  }

  // Estimate energy usage in kWh over a period (hours)
  static double calculateEnergyUsageKWh(double totalWatts, double hours) {
    return (totalWatts * hours) / 1000.0;
  }
}
