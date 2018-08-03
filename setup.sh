#!/bin/sh

for arg; do
    case "$arg" in
        --cc=*)
            CC="${arg#--cc=}"
            ;;
        --variant=*)
            PLATFORM_VARIANT="${arg#--variant=}"
            test -f "setup/${PLATFORM_VARIANT}.mk" || {
                echo "Variant $PLATFORM_VARIANT is unknown"
                exit 1
            }
            ;;
        *)
            echo "Unknown arg $arg" >&2
            exit 1
            ;;
    esac
done

test -n "$CC" || CC=c99

NORMALIZE='/^#/d; s/[^[:alnum:]_.+:-]//g; /^$/d'

PLATFORM_ARCH="$(${CC} -E setup/test_arch.h | sed -e "$NORMALIZE")"  || PLATFORM_OS=unknown
PLATFORM_OS="$(${CC} -E setup/test_os.h | sed -e "$NORMALIZE")" || PLATFORM_OS=unknown
PLATFORM_CC_ID="$(${CC} -E setup/test_cc.h | sed -e "$NORMALIZE")" || PLATFORM_CC_ID=unknown

case "$PLATFORM_CC_ID" in
    gcc*)
        PLATFORM_CC_FAMILY=gcc
        PLATFORM_CC_VERSION=${PLATFORM_CC_ID#gcc}
        ;;
    clang*)
        PLATFORM_CC_FAMILY=clang
        PLATFORM_CC_VERSION=${PLATFORM_CC_ID#clang}
        ;;
    *)
        PLATFORM_CC_FAMILY=other
        PLATFORM_CC_VERSION=unknown
        ;;
esac

PLATFORM_PLATFORM="${PLATFORM_OS}_${PLATFORM_ARCH}_${PLATFORM_CC_ID}${PLATFORM_VARIANT:+_}${PLATFORM_VARIANT}"

BUILDDIR="build/${PLATFORM_PLATFORM}"

mkdir -p "$BUILDDIR" || exit 1

if ! ${CC} -Wall -W -Werror -o "$BUILDDIR/testrun" setup/testrun.c || \
        ! test -x "$BUILDDIR/testrun"; then
    echo "${CC} cannot make executables" >&2
    exit 1
fi
if test -n "$CROSSBUILD" || ! "$BUILDDIR/testrun" 2>&1 | diff -q - setup/testrun.expected >/dev/null; then
    echo "Cross-build detected" >&2
    CROSSBUILD=1
fi
rm -f "$BUILDDIR/testrun"
   

echo "Target platform is $PLATFORM_PLATFORM"
cat  >$BUILDDIR/Makefile <<EOF
TOPDIR = ../..
CC = $CC
PLATFORM_ARCH = $PLATFORM_ARCH
PLATFORM_OS = $PLATFORM_OS
PLATFORM_VARIANT = $PLATFORM_VARIANT
PLATFORM_CC_FAMILY = $PLATFORM_CC_FAMILY
PLATFORM_CC_VERSION = $PLATFORM_CC_VERSION
CROSSBUILD = $CROSSBUILD

include ../../Makefile.generic

EOF
exit 0
