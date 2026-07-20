import 'package:flutter/material.dart';
import 'screens/smart_pump_fan_dashboard.dart';

void main() {
  runApp(const SmartHomePumpFanApp());
}

class SmartHomePumpFanApp extends StatelessWidget {
  const SmartHomePumpFanApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'MotorBoy Smart Home',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        brightness: Brightness.dark,
        scaffoldBackgroundColor: const Color(0xFF121212),
        colorScheme: const ColorScheme.dark(
          primary: Color(0xFF0000FF), // Deep Blue
          secondary: Color(0xFFFF0000), // Vibrant Red
          surface: Color(0xFF1E1E1E),
        ),
        appBarTheme: const AppBarTheme(
          backgroundColor: Color(0xFF000000),
          foregroundColor: Colors.white,
          elevation: 0,
        ),
        useMaterial3: true,
      ),
      home: const SmartPumpFanDashboard(),
    );
  }
}