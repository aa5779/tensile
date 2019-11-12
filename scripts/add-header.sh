#!/bin/bash

H="${1//./_}"

cat >"$1" <<EOF
/** @file
 *  $3
 * @author Artem V. Andreev
 * @copyright
 * @parblock
 * &copy; `date +%Y` Artem V. Andreev
 *
 * SPDX-License-Identifier: ${2:-MIT}
 * @endparblock
 */

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
