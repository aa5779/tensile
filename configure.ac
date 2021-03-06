AX_IS_RELEASE([never])
AC_INIT([tensile], [0.1])
AC_PREREQ([2.69])
AM_INIT_AUTOMAKE([1.16.1])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AX_CHECK_ENABLE_DEBUG([yes], [], [TN_DEBUG_DISABLED])
AS_IF([test "x$ax_enable_debug" = xno],
      [AX_APPEND_COMPILE_FLAGS([-O3], [CFLAGS])])

AC_PROG_CC_C99
AC_PROG_RANLIB
AC_PROG_AWK
AC_PATH_PROGS([M4], [gm4 m4])

AC_CHECK_TOOL([NM], [nm])

AX_PROG_FLEX
AX_PROG_BISON

DX_PS_FEATURE([OFF])
DX_DOT_FEATURE([ON])
DX_INIT_DOXYGEN([tensile], [$(top_srcdir)/Doxyfile], [doc])

AX_CODE_COVERAGE()
AX_COMPILER_FLAGS()

AC_CHECK_LIB([unistring], [u8_check])

PKG_CHECK_MODULES([TDB], [tdb])
PKG_CHECK_MODULES([TALLOC], [talloc])

AX_GCC_FUNC_ATTRIBUTE([malloc])
AX_GCC_FUNC_ATTRIBUTE([nonnull])
AX_GCC_FUNC_ATTRIBUTE([returns_nonnull])
AX_GCC_FUNC_ATTRIBUTE([warn_unused_result])
AX_GCC_FUNC_ATTRIBUTE([sentinel])
AX_GCC_FUNC_ATTRIBUTE([alloc_size])
AX_GCC_FUNC_ATTRIBUTE([format])
AX_GCC_FUNC_ATTRIBUTE([format_arg])
AX_GCC_FUNC_ATTRIBUTE([pure])
AX_GCC_FUNC_ATTRIBUTE([const])
AX_GCC_FUNC_ATTRIBUTE([weak])
AX_GCC_FUNC_ATTRIBUTE([visibility])
AX_GCC_FUNC_ATTRIBUTE([deprecated])
AX_GCC_FUNC_ATTRIBUTE([unused])
AX_GCC_FUNC_ATTRIBUTE([used])
AX_GCC_FUNC_ATTRIBUTE([constructor])
AX_GCC_FUNC_ATTRIBUTE([destructor])
AX_GCC_FUNC_ATTRIBUTE([dllexport])
AX_GCC_FUNC_ATTRIBUTE([dllimport])
AX_GCC_FUNC_ATTRIBUTE([noreturn])

AX_GCC_VAR_ATTRIBUTE([aligned])

AX_GCC_BUILTIN([__builtin_expect])

AC_C_RESTRICT

AC_CONFIG_FILES([Makefile compiler/Makefile \
                 lib/Makefile lib/tests/Makefile])
AC_CONFIG_HEADERS([tn_config.h])
AC_OUTPUT
