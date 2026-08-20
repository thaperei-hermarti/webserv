#!/bin/sh

set -eu

mode=${1:-all}

pkg_cflags=""
pkg_libs=""

if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists gtest_main 2>/dev/null; then
	pkg_cflags=$(pkg-config --cflags gtest_main)
	pkg_libs=$(pkg-config --libs gtest_main)
elif command -v gtest-config >/dev/null 2>&1; then
	pkg_cflags=$(gtest-config --cxxflags 2>/dev/null || true)
	pkg_libs=$(gtest-config --libs 2>/dev/null || true)
elif command -v brew >/dev/null 2>&1; then
	prefix=$(brew --prefix googletest 2>/dev/null || true)
	if [ -n "$prefix" ] && [ -d "$prefix" ]; then
		pkg_cflags="-I$prefix/include"
		pkg_libs="-L$prefix/lib -lgtest_main -lgtest"
	fi
fi

if [ -z "$pkg_cflags" ] && [ -z "$pkg_libs" ] \
	&& [ -f "$HOME/.local/include/gtest/gtest.h" ] \
	&& [ -f "$HOME/.local/lib/libgtest.a" ]; then
	pkg_cflags="-I$HOME/.local/include"
	pkg_libs="-L$HOME/.local/lib -lgtest_main -lgtest -lpthread"
fi

if [ -z "$pkg_cflags" ] && [ -z "$pkg_libs" ]; then
	pkg_cflags=${GTEST_CFLAGS:--DGTEST_HAS_PTHREAD=1}
	pkg_libs=${GTEST_LIBS:--lgtest_main -lgtest}
fi

case "$mode" in
	cflags)
		printf '%s\n' "$pkg_cflags"
		;;
	libs)
		printf '%s\n' "$pkg_libs"
		;;
	all)
		printf 'CFLAGS=%s\n' "$pkg_cflags"
		printf 'LIBS=%s\n' "$pkg_libs"
		;;
	*)
		echo "usage: $0 [cflags|libs|all]" >&2
		exit 1
		;;
esac