#!/usr/bin/env bash
# Installs missing development dependencies: clang-format, clang-tidy,
# GoogleTest and valgrind.
#
# Strategy, first one that works:
#   1. brew        - user-level install, no sudo required (42 school machines)
#   2. apt + sudo  - system install (Debian/Ubuntu)
#   3. pip --user  - user-level install of clang-format/clang-tidy, GoogleTest
#                    built from source into ~/.local
set -euo pipefail

ROOT_DIR="$(git rev-parse --show-toplevel)"
cd "$ROOT_DIR"

MISSING=()
NEED_GTEST=0
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)

tool_missing()
{
	local tool="$1"
	local bin
	if command -v "$tool" >/dev/null 2>&1; then
		return 1
	fi
	for bin in "$HOME/.local/bin/$tool" "$HOME/bin/$tool"; do
		if [ -x "$bin" ]; then
			echo "[INFO] $tool found at $bin (not in PATH)"
			return 1
		fi
	done
	echo "[INFO] $tool not found, scheduling installation"
	MISSING+=("$tool")
	return 0
}

tool_missing clang-format || true
tool_missing clang-tidy || true
tool_missing valgrind || true

if ! sh scripts/check-deps.sh >/dev/null 2>&1; then
	echo "[INFO] GoogleTest not available, scheduling installation"
	NEED_GTEST=1
fi

if [ "${#MISSING[@]}" -eq 0 ] && [ "$NEED_GTEST" -eq 0 ]; then
	echo "[OK] All dependencies are already installed"
	exit 0
fi

find_brew()
{
	if command -v brew >/dev/null 2>&1; then
		return 0
	fi
	local cand dir
	for cand in "$HOME/.brew/bin/brew" /opt/homebrew/bin/brew /usr/local/bin/brew \
		/goinfre/brew/bin/brew /goinfre/homebrew/bin/brew /sgoinfre/goinfre/brew/bin/brew; do
		if [ -x "$cand" ]; then
			dir=$(dirname "$cand")
			echo "[INFO] Found brew at $cand, adding it to PATH"
			export PATH="$dir:$PATH"
			return 0
		fi
	done
	return 1
}

can_sudo()
{
	[ "$(id -u)" -eq 0 ] && return 0
	command -v sudo >/dev/null 2>&1 || return 1
	sudo -v >/dev/null 2>&1
}

apt_cmd()
{
	if [ "$(id -u)" -eq 0 ]; then
		apt-get "$@"
	else
		sudo apt-get "$@"
	fi
}

build_apt_gtest()
{
	local src=/usr/src/googletest
	local lib
	if [ ! -d "$src" ]; then
		return 0
	fi
	for lib in /usr/lib/libgtest*.a /usr/lib/*/libgtest*.a /usr/local/lib/libgtest*.a; do
		[ -e "$lib" ] && return 0
	done
	echo "[INFO] libgtest-dev ships sources only, building static libraries..."
	if ! command -v cmake >/dev/null 2>&1; then
		echo "[INFO] Installing cmake to build GoogleTest"
		apt_cmd install -y cmake
	fi
	local build_dir
	build_dir=$(mktemp -d)
	trap 'rm -rf "$build_dir"' EXIT
	cmake -S "$src" -B "$build_dir" -DBUILD_GMOCK=OFF -DCMAKE_INSTALL_PREFIX=/usr/local
	if [ "$(id -u)" -eq 0 ]; then
		cmake --build "$build_dir" --target install -- -j"$JOBS"
	else
		sudo cmake --build "$build_dir" --target install -- -j"$JOBS"
	fi
}

install_apt()
{
	local pkgs=()
	local t
	for t in "${MISSING[@]}"; do
		pkgs+=("$t")
	done
	[ "$NEED_GTEST" -eq 1 ] && pkgs+=(libgtest-dev)
	echo "[INFO] Installing with apt: ${pkgs[*]}"
	apt_cmd update
	apt_cmd install -y "${pkgs[@]}"
	if [ "$NEED_GTEST" -eq 1 ]; then
		build_apt_gtest
	fi
}

install_brew()
{
	local pkgs=()
	local t
	for t in "${MISSING[@]}"; do
		if [ "$t" = "valgrind" ] && [ "$(uname -s)" = "Darwin" ]; then
			echo "[WARNING] valgrind is not supported on macOS; skipping"
			continue
		fi
		pkgs+=("$t")
	done
	[ "$NEED_GTEST" -eq 1 ] && pkgs+=(googletest)
	echo "[INFO] Installing with brew (no sudo required): ${pkgs[*]}"
	brew install "${pkgs[@]}"
}

build_gtest_local()
{
	local prefix="$HOME/.local"
	local build_dir
	if [ -f "$prefix/lib/libgtest.a" ] && [ -f "$prefix/include/gtest/gtest.h" ]; then
		return 0
	fi
	if ! command -v cmake >/dev/null 2>&1; then
		echo "[ERROR] cmake not found; cannot build GoogleTest."
		exit 1
	fi
	if ! command -v git >/dev/null 2>&1; then
		echo "[ERROR] git not found; cannot fetch GoogleTest."
		exit 1
	fi
	build_dir=$(mktemp -d)
	trap 'rm -rf "$build_dir"' EXIT
	echo "[INFO] Building GoogleTest from source into $prefix"
	git clone --depth 1 https://github.com/google/googletest.git "$build_dir/googletest"
	cmake -S "$build_dir/googletest" -B "$build_dir/build" -DBUILD_GMOCK=OFF -DCMAKE_INSTALL_PREFIX="$prefix"
	cmake --build "$build_dir/build" --target install -- -j"$JOBS"
}

install_pip()
{
	local pip
	if command -v pip3 >/dev/null 2>&1; then
		pip=pip3
	elif command -v pip >/dev/null 2>&1; then
		pip=pip
	fi
	local pkgs=()
	local t
	local installed=0
	for t in "${MISSING[@]}"; do
		case "$t" in
			clang-format|clang-tidy)
				pkgs+=("$t")
				;;
			*)
				echo "[WARNING] $t cannot be installed via pip; skipping"
				;;
		esac
	done
	if [ -n "${pkgs[*]}" ]; then
		echo "[INFO] Installing with pip (user-level): ${pkgs[*]}"
		set +e
		"$pip" install --user "${pkgs[@]}"
		set -e
		local fail=0
		local t2
		for t2 in "${pkgs[@]}"; do
			if ! command -v "$t2" >/dev/null 2>&1 && [ ! -x "$HOME/.local/bin/$t2" ]; then
				echo "[ERROR] pip install failed: $t2 is not available."
				fail=1
			fi
		done
		if [ "$fail" -eq 1 ]; then
			exit 1
		fi
		installed=1
	fi
	if [ "$NEED_GTEST" -eq 1 ]; then
		build_gtest_local
		installed=1
	fi
	if [ "$installed" -eq 0 ]; then
		echo "[ERROR] None of the missing tools can be installed via pip."
		exit 1
	fi
	echo "[INFO] Ensure $HOME/.local/bin is in your PATH"
	if [ "$NEED_GTEST" -eq 1 ]; then
		echo "[INFO] Add these to your shell profile so the Makefile finds GoogleTest:"
		echo "export GTEST_CFLAGS=-I$HOME/.local/include"
		echo "export GTEST_LIBS=-L$HOME/.local/lib -lgtest_main -lgtest"
	fi
}

if find_brew; then
	install_brew
elif command -v apt-get >/dev/null 2>&1 && can_sudo; then
	install_apt
elif command -v pip3 >/dev/null 2>&1 || command -v pip >/dev/null 2>&1; then
	install_pip
else
	echo "[ERROR] No supported install method found (brew, apt+sudo or pip)."
	echo "[INFO] Install manually: clang-format, clang-tidy, valgrind and GoogleTest."
	exit 1
fi

echo "[OK] All dependencies installed"
