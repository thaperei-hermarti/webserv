#!/usr/bin/env bash
# Installs the git hooks (pre-commit, commit-msg) from scripts/.
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel)"
HOOK_DIR="$(git rev-parse --git-dir)/hooks"

install_hook() {
	local name="$1"
	local src="$ROOT_DIR/scripts/$name"
	local dst="$HOOK_DIR/$name"

	if [ ! -f "$src" ]; then
		echo "[ERROR] $src not found"
		exit 1
	fi

	if [ -f "$dst" ]; then
		if [ "$(cat "$dst")" != "$(cat "$src")" ]; then
			echo "[WARNING] Existing $name hook differs from $src"
			read -rp "Overwrite it? [y/N] " answer
			if [ "${answer:-n}" != "y" ] && [ "${answer:-n}" != "Y" ]; then
				echo "[INFO] Aborted. $name hook not installed."
				return 0
			fi
		fi
	fi

	install -m 755 "$src" "$dst"
	echo "[OK] $name hook installed at $dst"
}

install_hook pre-commit
install_hook commit-msg