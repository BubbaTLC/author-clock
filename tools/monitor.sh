#!/usr/bin/env bash
# Run idf.py monitor with wall-clock timestamps prepended to every log line.
# Usage: ./tools/monitor.sh [extra idf.py monitor args]
#
# Each line in the serial output will be prefixed with the host time, e.g.:
#   2026-04-10 22:08:09 | I (18321) DISPLAY_MGR: Updating display: 04:00
exec idf.py monitor --timestamps "$@"