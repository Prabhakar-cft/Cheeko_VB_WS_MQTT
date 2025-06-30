# VB6824 Voice Communication Project

A modular ESP32-based voice communication system using VB6824 audio hardware, Opus codec, WebSocket for real-time audio streaming, and MQTT for device management.

## 🎯 Features

- **Real-time Voice Communication** - Low-latency audio streaming via WebSocket
- **Opus Audio Codec** - High-quality, low-bandwidth audio compression
- **WiFi Manager** - Dynamic WiFi configuration with captive portal
- **MQTT Integration** - Device management and authentication
- **RGB LED Status System** - Comprehensive visual feedback with animations
- **Modular Architecture** - Clean, maintainable code structure
- **Button Control** - Simple push-to-talk functionality

## 🏗️ Architecture

### Modular Design
The project follows a clean modular architecture with separate components for each functionality:

```
main/
├── main.c                    # Application entry point
├── session_manager.c/h       # UUID generation and session management
├── wifi_manager.c/h          # WiFi initialization and event handling
├── websocket_client.c/h      # WebSocket client management
├── audio_session.c/h         # Audio recording and playback
├── button_handler.c/h        # Button input handling
├── led.c/h                   # RGB LED control with animations
├── mqtt_handler.c/h          # MQTT client management
└── opus_wapper.c/h           # Opus codec wrapper
```

## 🔧 Hardware Requirements

- **ESP32-C2** microcontroller
- **VB6824** audio codec module
- **RGB LED** (GPIO 0, 1, 2)
- **Push Button** (GPIO 3)
- **Battery Monitor** (GPIO 4) - Optional

## 📋 Prerequisites

- ESP-IDF v5.4.1 or later
- Python 3.7+
- Git

## 🚀 Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd vb_demo_mq
   ```

2. **Install ESP-IDF**
   ```bash
   # Follow ESP-IDF installation guide
   # https://docs.espressif.com/projects/esp-idf/en/latest/esp32c2/get-started/
   ```

3. **Configure the project**
   ```bash
   idf.py menuconfig
   ```

4. **Build and flash**
   ```bash
   idf.py build
   idf.py flash
   ```

## ⚙️ Configuration

### WiFi Configuration
Update WiFi credentials in `main/wifi_manager.h`:
```c
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"
```

### WebSocket Server
Configure WebSocket server in `main/websocket_client.h`:
```c
#define WS_URI "ws://your_server_ip:port"
```

### Audio Settings
Audio configuration in `main/websocket_client.h`:
```c
#define SAMPLE_RATE 16000
#define CHANNELS 1
#define FRAME_DURATION 60  // ms
```

## 🎨 LED Status System

The RGB LED provides comprehensive visual feedback:

| State | LED Color | Animation | Description |
|-------|-----------|-----------|-------------|
| `LED_SYS_IDLE` | Off | None | System idle |
| `LED_SYS_WIFI_CONNECTING` | Yellow | Blink (100ms) | WiFi connecting |
| `LED_SYS_WIFI_CONNECTED` | Green | Solid | WiFi connected |
| `LED_SYS_WS_CONNECTING` | Cyan | Blink (300ms) | WebSocket connecting |
| `LED_SYS_WS_CONNECTED` | Green | Solid | WebSocket connected |
| `LED_SYS_AUTH_WAITING` | Yellow | Pulse (1000ms) | Waiting for auth |
| `LED_SYS_READY` | Green | Solid | Ready to record |
| `LED_SYS_BUTTON_PRESSED` | Magenta | Solid | Button pressed |
| `LED_SYS_RECORDING` | Magenta | Pulse (800ms) | Recording audio |
| `LED_SYS_PLAYING` | Blue | Pulse (600ms) | Playing audio |
| `LED_SYS_ERROR` | Red | Blink (200ms) | Error state |

## 🔄 Usage

1. **Power on** the device
2. **Wait for WiFi connection** (Yellow blinking LED)
3. **Wait for WebSocket connection** (Cyan blinking LED)
4. **Wait for authentication** (Yellow pulsing LED)
5. **Ready to use** (Green solid LED)
6. **Press and hold button** to start recording (Magenta LED)
7. **Release button** to stop recording

## 📡 Communication Protocol

### WebSocket Messages

**Start Recording:**
```json
{
  "type": "start",
  "session_id": "uuid-string",
  "sample_rate": 16000,
  "channels": 1,
  "frame_duration": 60.0,
  "auth_token": "mqtt-auth-token"
}
```

**Stop Recording:**
```json
{
  "type": "stop",
  "session_id": "uuid-string"
}
```

**Audio Frames:**
- Binary data with 4-byte length header (big-endian)
- Opus-encoded audio data

## 🔧 Development

### Adding New Features

1. **Create new module** in `main/` directory
2. **Add to CMakeLists.txt** source files
3. **Update main.c** to initialize new module
4. **Add LED states** if needed

### LED Animation System

The LED system supports multiple animation types:
- `LED_ANIM_BLINK` - Simple on/off blink
- `LED_ANIM_PULSE` - Smooth pulse effect
- `LED_ANIM_ROTATE` - Rotate through colors
- `LED_ANIM_BREATH` - Breathing effect
- `LED_ANIM_WAVE` - Wave pattern
- `LED_ANIM_LOADING` - Loading animation

### Thread Safety

All LED operations are thread-safe using mutex protection.

## 🐛 Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Check WiFi credentials in `wifi_manager.h`
   - Verify network availability

2. **WebSocket Connection Failed**
   - Check server URL in `websocket_client.h`
   - Verify server is running and accessible

3. **Audio Issues**
   - Check VB6824 connections
   - Verify GPIO pins (10, 18)

4. **LED Not Working**
   - Check RGB LED connections (GPIO 0, 1, 2)
   - Verify power supply

### Debug Logs

Enable debug logging:
```bash
idf.py monitor
```

## 📝 License

[Add your license information here]

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📞 Support

For issues and questions:
- Create an issue in the repository
- Check the troubleshooting section
- Review the ESP-IDF documentation

## 🔄 Version History

- **v1.0.0** - Initial modular release with LED animation system
- **v0.9.0** - WebSocket and MQTT integration
- **v0.8.0** - WiFi manager implementation
- **v0.7.0** - Basic audio functionality

---

**Note:** This project is designed for ESP32-C2 with VB6824 audio hardware. Ensure compatibility before deployment. 