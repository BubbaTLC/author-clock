#!/usr/bin/env bash
# Debug helper for Author Clock web interface issues
# Usage: ./tools/debug_web.sh

echo "=== Author Clock Web Interface Diagnostics ==="
echo ""

echo "🔍 Step 1: Check if you're connected to the right network"
echo "Expected network: 'AuthorClock' (open WiFi)"
echo "Your network info:"
if command -v networksetup >/dev/null 2>&1; then
    # macOS
    networksetup -getairportnetwork en0 2>/dev/null || echo "Could not get WiFi info"
    echo ""
    echo "Your IP address:"
    ifconfig en0 | grep "inet " | awk '{print $2}' 2>/dev/null || echo "Could not get IP"
elif command -v nmcli >/dev/null 2>&1; then
    # Linux with NetworkManager
    nmcli dev status | grep wifi
    echo ""
    echo "Your IP address:"
    ip route get 1.1.1.1 | grep -oP 'src \K\S+' 2>/dev/null || echo "Could not get IP"
else
    echo "Manual check: Verify you're connected to 'AuthorClock' WiFi"
fi

echo ""
echo "🌐 Step 2: Test connectivity to ESP32"
echo "Trying to ping ESP32 AP gateway..."
if ping -c 3 192.168.4.1 >/dev/null 2>&1; then
    echo "✅ ESP32 AP gateway (192.168.4.1) is reachable"
else
    echo "❌ Cannot reach ESP32 AP gateway (192.168.4.1)"
    echo "   This suggests you're not connected to the ESP32's WiFi network"
fi

echo ""
echo "🔧 Step 3: Test web server"
echo "Trying to connect to web interface..."
if command -v curl >/dev/null 2>&1; then
    echo "Testing HTTP connection to http://192.168.4.69 ..."
    if curl -s --connect-timeout 5 http://192.168.4.69 >/dev/null 2>&1; then
        echo "✅ Web server is responding"
        echo ""
        echo "Try these URLs in your browser:"
        echo "  • http://192.168.4.69"
        echo "  • http://192.168.4.1" 
        echo "  • http://authorclock.local (if mDNS works)"
    else
        echo "❌ Web server is not responding"
        echo ""
        echo "Possible causes:"
        echo "  • ESP32 web server not started"
        echo "  • Wrong IP address" 
        echo "  • Firewall blocking connection"
        echo "  • ESP32 in wrong WiFi mode"
    fi
else
    echo "curl not available - try connecting manually to:"
    echo "  • http://192.168.4.69"
    echo "  • http://192.168.4.1"
fi

echo ""
echo "📋 Step 4: Next steps"
echo ""
echo "If web interface still doesn't work:"
echo "1. Check ESP32 serial output:"
echo "   ./tools/monitor.sh"
echo ""
echo "2. Look for these log messages:"
echo "   • 'Configuration server started'"
echo "   • 'Failed to start config web server'"
echo "   • WiFi mode changes"
echo ""
echo "3. Try alternative approaches:"
echo "   • Use http://192.168.4.1 instead of .69"
echo "   • Check if captive portal popup appears"
echo "   • Try from different device/browser"