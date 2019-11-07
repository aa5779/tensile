#!/bin/bash

H="${1//./_}"

cat >"$1" <<EOF
/*
 * Copyright (c) `date +%Y` Artem V. Andreev
 *
 * SPDX-License-Identifier: ${2:+MIT}
 */

/** @file
 *  $3
 */
#ifndef TNH_${H^^*}
#define TNH_${H^^*} 1

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_${H^^*} */
EOF
