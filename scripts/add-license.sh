#!/bin/bash
#######################################################################
# Copyright (c) 2017 Artem V. Andreev
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#  
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#  
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#######################################################################
TOPDIR="$(dirname $0)"

test -f "${TOPDIR}/.boilerplate" || { echo "No boilerplate config file" >&2; exit 1; }

ADDHEADER="headache -c ${TOPDIR}/.boilerplate -h /dev/stdin"
MIT_LICENSE='Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
 
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.'
GPL3_LICENSE='This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>'
LGPL3_LICENSE="${GPL3_LICENSE//GNU General/GNU Lesser General}"

LICENSE=""
OWNER="Artem V. Andreev"
set -e

for arg; do
    case "$arg" in
        --mit)
            LICENSE="$MIT_LICENSE" 
            ;;
        --lgpl)
            LICENSE="$LGPL3_LICENSE"
            ;;
        --gpl)
            LICENSE="$GPL3_LICENSE"
            ;;
        --owner=*)
            OWNER="${arg#--owner=}"
            ;;
        --*)
            echo "Unknown option $arg" >&2
            exit 1
            ;;
        *)
            test -n "$OWNER" || { echo "No owner specified" >&2; exit 1; }
            test -n "$LICENSE" || { echo "No license specified" >&2; exit 1; }
            $ADDHEADER "$arg" <<EOF
Copyright (c) `date +%Y` $OWNER

$LICENSE
EOF
            ;;
    esac
done
