#!/bin/sh

set -eu

CXX=${CXX:-c++}
TEST_CFLAGS=${TEST_CFLAGS:--Wall -Wextra -Werror -std=c++17}
INCLUDES=${INCLUDES:--I./inc}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT INT TERM

cat > "$tmpdir/check_gtest.cpp" <<'EOF'
#include <gtest/gtest.h>

int main()
{
	return 0;
}
EOF

try()
{
	# $1: cflags, $2: libs
	if $CXX $TEST_CFLAGS $1 $INCLUDES "$tmpdir/check_gtest.cpp" $2 \
		-o "$tmpdir/check_gtest" 2>"$tmpdir/check_gtest.err"; then
		return 0
	fi
	return 1
}

cleared=0
cflags=$(sh scripts/gtest-flags.sh cflags)
libs=$(sh scripts/gtest-flags.sh libs)

if try "$cflags" "$libs"; then
	cleared=1
elif [ -f "$HOME/.local/include/gtest/gtest.h" ] && [ -f "$HOME/.local/lib/libgtest.a" ] \
	&& try "-I$HOME/.local/include" "-L$HOME/.local/lib -lgtest_main -lgtest -lpthread"; then
	cleared=1
elif try "-DGTEST_HAS_PTHREAD=1" "-lgtest_main -lgtest"; then
	cleared=1
fi

if [ "$cleared" -eq 1 ]; then
	echo "[OK] GoogleTest dependency check passed"
	exit 0
fi

echo "[ERROR] GoogleTest is not available or cannot be linked."
echo "[INFO] Install it with: make setup"
echo "[INFO]   or: sudo apt install libgtest-dev / brew install googletest"
echo "[INFO] Last compiler output:"
sed 's/^/  /' "$tmpdir/check_gtest.err" 2>/dev/null || true
exit 1