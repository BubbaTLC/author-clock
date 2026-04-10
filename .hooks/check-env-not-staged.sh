#!/usr/bin/env bash
set -e
if git diff --cached --name-only | grep -q "^\.env$"; then
    echo "ERROR: .env is staged for commit and contains secrets (API keys)."
    echo "       Remove it: git reset HEAD .env"
    echo "       Then add .env to .gitignore if not already there."
    exit 1
fi
