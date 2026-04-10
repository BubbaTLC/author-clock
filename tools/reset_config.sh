#!/usr/bin/env bash
# Author Clock Configuration Guide
# Multiple ways to change WiFi credentials and timezone settings

echo "=== Author Clock Configuration Options ==="
echo ""
echo "🌐 EASY METHOD: Web Interface (Recommended)"
echo "The device always runs a configuration server:"
echo ""
echo "1. Connect to 'AuthorClock' WiFi network"
echo "2. Open browser to: http://192.168.4.69"  
echo "3. Enter new settings:"
echo "   • WiFi credentials"
echo "   • Location and timezone (Saskatoon = -6)"
echo "4. Submit form (settings saved immediately)"
echo "5. Reset device (press RST button) to use new WiFi"
echo ""
echo "🔧 HARDWARE METHOD: BOOT Button Reset"
echo "If web interface is not accessible:"
echo ""
echo "1. Hold down the BOOT button on your ESP32"
echo "2. Press and release the EN/RST button (while holding BOOT)"
echo "3. Keep holding BOOT for 3+ seconds after reset"  
echo "4. Release BOOT button"
echo "5. Device will clear all settings and restart into setup mode"
echo ""
echo "📊 MONITOR METHOD: Serial Debugging"
echo "To watch what's happening:"
echo ""
echo "Press Enter to start serial monitor, or Ctrl+C to exit..."
read

echo "Starting serial monitor with timestamps..."
echo "Look for 'Configuration server active' message."
echo "Press Ctrl+] to exit monitor."
echo ""

# Try to find the USB serial port
PORT=$(ls /dev/tty.usbserial-* 2>/dev/null | head -1)
if [ -z "$PORT" ]; then
    PORT=$(ls /dev/ttyUSB* 2>/dev/null | head -1)
fi

if [ -z "$PORT" ]; then
    echo "No USB serial port found. Please specify manually:"
    echo "./tools/monitor.sh -p /dev/ttyXXX"
    exit 1
fi

echo "Using port: $PORT"
./tools/monitor.sh -p "$PORT"