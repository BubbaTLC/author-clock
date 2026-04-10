#!/usr/bin/env bash
set -e
if git diff --cached --name-only | grep -q "^sdkconfig$"; then
    echo "ERROR: sdkconfig is staged for commit."
    echo "       sdkconfig contains build-time settings and device-specific config"
    echo "       that should not be committed to version control."
    echo "       Use sdkconfig.defaults for shared settings instead."
    echo "       Remove it: git reset HEAD sdkconfig"
    exit 1
fi
