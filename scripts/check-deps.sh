#!/bin/sh

set -eu

CXX=${CXX:-c++}
TEST_CFLAGS=${TEST_CFLAGS:--Wall -Wextra -Werror -std=c++17}
INCLUDES=${INCLUDES:--I./inc}
GTEST_CFLAGS=$(sh scripts/gtest-flags.sh cflags)
GTEST_LIBS=$(sh scripts/gtest-flags.sh libs)

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT INT TERM

cat > "$tmpdir/check_gtest.cpp" <<'EOF'
#include <gtest/gtest.h>

int main()
{
	return 0;
}
EOF

if ! $CXX $TEST_CFLAGS $GTEST_CFLAGS $INCLUDES "$tmpdir/check_gtest.cpp" $GTEST_LIBS -o "$tmpdir/check_gtest" >/dev/null 2>&1; then
	echo "[ERROR] GoogleTest is not available."
	echo "[INFO] Install it with your platform package manager:"
	echo "[INFO]   Linux: sudo apt install libgtest-dev or your distro equivalent"
	echo "[INFO]   macOS: brew install googletest"
	exit 1
fi

echo "[OK] GoogleTest dependency check passed"