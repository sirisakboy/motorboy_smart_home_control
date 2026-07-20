import 'package:flutter/material.dart';
import 'dart:ui';

class DeviceCard extends StatelessWidget {
  final String deviceType;
  final bool isOn;
  final int delayMinutes;
  final VoidCallback onToggle;
  final VoidCallback onDelayTap;

  const DeviceCard({
    super.key,
    required this.deviceType,
    required this.isOn,
    required this.delayMinutes,
    required this.onToggle,
    required this.onDelayTap,
  });

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final isActive = isOn || delayMinutes > 0;
    
    // Gradient colors based on brand
    final gradient = LinearGradient(
      colors: isActive
          ? [const Color(0xFF0000FF), const Color(0xFF4B0082)]
          : [Colors.grey.shade800, Colors.grey.shade900],
      begin: Alignment.topLeft,
      end: Alignment.bottomRight,
    );

    return Container(
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(24),
        gradient: LinearGradient(
          colors: [
            Colors.white.withOpacity(0.05),
            Colors.white.withOpacity(0.01),
          ],
        ),
        border: Border.all(
          color: isActive ? Colors.white.withOpacity(0.2) : Colors.transparent,
          width: 1.5,
        ),
      ),
      child: ClipRRect(
        borderRadius: BorderRadius.circular(24),
        child: BackdropFilter(
          filter: ImageFilter.blur(sigmaX: 10, sigmaY: 10),
          child: Padding(
            padding: const EdgeInsets.all(20),
            child: Column(
              children: [
                Container(
                  padding: const EdgeInsets.all(12),
                  decoration: BoxDecoration(
                    shape: BoxShape.circle,
                    gradient: gradient,
                  ),
                  child: Icon(
                    deviceType == 'pump' ? Icons.water_damage : Icons.air,
                    size: 32,
                    color: Colors.white,
                  ),
                ),
                const SizedBox(height: 16),
                Text(
                  deviceType == 'pump' ? 'ปั้มน้ำ' : 'ลมกัน',
                  style: const TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
                const SizedBox(height: 12),
                Switch(
                  value: isOn,
                  onChanged: (_) => onToggle(),
                  activeColor: theme.colorScheme.primary,
                ...
                                if (delayMinutes > 0) ...[
                                  const SizedBox(height: 8),
                                  Text(
                                    'อัตโนมัติ: $delayMinutes นาที',
                                    style: TextStyle(
                                      fontSize: 12,
                                      color: theme.colorScheme.secondary,
                                    ),
                                  ),
                                ],
                                const SizedBox(height: 8),
                                Text(
                                  'การใช้ไฟ: ${isOn ? 'ใช่' : 'ไม่ใช่'}',
                                  style: const TextStyle(fontSize: 12, color: Colors.white70),
                                ),
                              ],
                            ),
                          ),
                        ),
                      ),
                    );
                  }
                }