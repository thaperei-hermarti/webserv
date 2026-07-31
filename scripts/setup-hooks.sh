#!/usr/bin/env bash
# Installs the git pre-commit hook for clang-format and clang-tidy.
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel)"
HOOK_DIR="$(git rev-parse --git-dir)/hooks"
HOOK_FILE="$HOOK_DIR/pre-commit"
HOOK_SRC="$ROOT_DIR/scripts/pre-commit"

if [ ! -f "$HOOK_SRC" ]; then
	echo "[ERROR] $HOOK_SRC not found"
	exit 1
fi

if [ -f "$HOOK_FILE" ]; then
	if [ "$(cat "$HOOK_FILE")" != "$(cat "$HOOK_SRC")" ]; then
		echo "[WARNING] Existing pre-commit hook differs from $HOOK_SRC"
		read -rp "Overwrite it? [y/N] " answer
		if [ "${answer:-n}" != "y" ] && [ "${answer:-n}" != "Y" ]; then
			echo "[INFO] Aborted. Hook not installed."
			exit 0
		fi
	fi
fi

install -m 755 "$HOOK_SRC" "$HOOK_FILE"
echo "[OK] Pre-commit hook installed at $HOOK_FILE"
