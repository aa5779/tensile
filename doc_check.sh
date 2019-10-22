#!/bin/sh

TEXIS=
for f in "$@"; do
    case "$f" in
        *.texi)
            TEXIS="$TEXIS $f"
            ;;
    esac
done

`dirname $0`/docsymbols.sh "$@" |
    {
        RC=0
        while read -r SYMBOL; do
            ${GREP:-grep} -q -E "^ *@def(unx?|macx?|varx?|(tp|type(fun|var))x? +(\\{[^}]*\\}|[^[:space:]{}]+)) +$SYMBOL( |$)" \
                          $TEXIS ||
                {
                    echo "Symbol $SYMBOL is not documented" >&2
                    RC=1
                }
        done
        exit $RC
    }
