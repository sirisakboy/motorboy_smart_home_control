import 'package:flutter/material.dart';
import '../services/simple_relay_service.dart';
import '../services/network_scanner.dart';

class SmartPumpFanDashboard extends StatefulWidget {
  const SmartPumpFanDashboard({super.key});

  @override
  State<SmartPumpFanDashboard> createState() => _SmartPumpFanDashboardState();
}

class _SmartPumpFanDashboardState extends State<SmartPumpFanDashboard> {
  late SimpleRelayService _apiService;
  
  bool _isOn = false;
  bool _isLoading = false;
  bool _isScanning = false;
  bool _isConnected = false;
  String? _errorMessage;
  List<String> _foundDevices = [];
  
  int? _onHour;
  int? _offHour;

  @override
  void initState() {
    super.initState();
    _scanNetwork();
  }

  Future<void> _scanNetwork() async {
    setState(() => _isScanning = true);
    try {
      final devices = await NetworkScanner.scanEsp01();
      setState(() => _foundDevices = devices);
    } catch (e) {
      setState(() => _foundDevices = []);
    }
    setState(() => _isScanning = false);
  }

  void _connectToDevice(String? ip) {
    if (ip == null || ip.isEmpty) return;
    
    setState(() {
      _isLoading = true;
      _errorMessage = null;
    });

    _apiService = SimpleRelayService(baseUrl: 'http://$ip');
    
    _apiService.ping().then((connected) async {
      if (connected) {
        final status = await _apiService.getRelayStatus();
        setState(() {
          _isConnected = true;
          _isOn = status ?? false;
          _isLoading = false;
        });
      } else {
        setState(() {
          _isConnected = false;
          _isLoading = false;
          _errorMessage = 'ESP01 ไม่ตอบสนอง';
        });
      }
    });
  }

  void _toggleRelay() async {
    setState(() => _isLoading = true);
    final success = await _apiService.setRelay(!_isOn);
    if (success) {
      setState(() => _isOn = !_isOn);
    }
    setState(() => _isLoading = false);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Row(
          children: [
            Image.asset('assets/logo_no_background.png', height: 32),
            const SizedBox(width: 10),
            const Text('MotorBoy Dashboard'),
          ],
        ),
        actions: [
          if (_isConnected)
            IconButton(
              icon: const Icon(Icons.refresh),
              onPressed: () => setState(() => _isConnected = false),
            ),
        ],
      ),
      body: _isLoading
          ? const Center(child: CircularProgressIndicator())
          : _isConnected
              ? _buildControlPanel()
              : _buildDeviceSelector(),
    );
  }

  Widget _buildDeviceSelector() {
    return Column(
      children: [
        if (_isScanning) const LinearProgressIndicator(),
        Expanded(
          child: _foundDevices.isEmpty
              ? Center(child: Text('ไม่พบอุปกรณ์', style: TextStyle(color: Colors.white70)))
              : GridView.builder(
                  padding: const EdgeInsets.all(16),
                  gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
                    crossAxisCount: 2,
                    childAspectRatio: 1,
                    crossAxisSpacing: 16,
                    mainAxisSpacing: 16,
                  ),
                  itemCount: _foundDevices.length,
                  itemBuilder: (context, index) {
                    final ip = _foundDevices[index];
                    return InkWell(
                      onTap: () => _connectToDevice(ip),
                      child: Container(
                        decoration: BoxDecoration(
                          color: const Color(0xFF1E1E1E),
                          borderRadius: BorderRadius.circular(20),
                        ),
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            const Icon(Icons.router, size: 48, color: Colors.blue),
                            const SizedBox(height: 10),
                            Text(ip, style: const TextStyle(color: Colors.white)),
                          ],
                        ),
                      ),
                    );
                  },
                ),
        ),
      ],
    );
  }

  Widget _buildControlPanel() {
    return Center(
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Icon(Icons.electric_bolt, size: 120, color: _isOn ? Colors.blue : Colors.grey[800]),
          const SizedBox(height: 20),
          Text(_isOn ? 'SYSTEM ACTIVE' : 'SYSTEM STANDBY',
            style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold, color: _isOn ? Colors.blue : Colors.white70)),
          const SizedBox(height: 20),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton.icon(
                onPressed: () async {
                  // TODO: Call AiService for insights
                },
                icon: const Icon(Icons.psychology),
                label: const Text('AI Insight'),
              ),
              const SizedBox(width: 10),
              ElevatedButton.icon(
                onPressed: () async {
                  // TODO: Call AiService for intelligent control
                },
                icon: const Icon(Icons.smart_toy),
                label: const Text('AI Control'),
              ),
            ],
          ),
          const SizedBox(height: 40),
          IconButton(
            icon: Icon(_isOn ? Icons.toggle_on : Icons.toggle_off, size: 150, color: _isOn ? Colors.red : Colors.grey),
            onPressed: _toggleRelay,
          ),
        ],
      ),
    );
  }
}