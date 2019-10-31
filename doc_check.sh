#!/bin/sh

INFOS=
for f in "$@"; do
    case "$f" in
        *.info)
            INFOS="$INFOS $f"
            ;;
    esac
done

`dirname $0`/docsymbols.sh "$@" |
    {
        RC=0
        while read -r SYMBOL; do
            ${GREP:-grep} -q -E "^ -- ((Macro|Type|Enum|Struct|Union): $SYMBOL( |\$)|(Variable|Function): ([[:alnum:]_*]+ )*$SYMBOL *(\\(|\\[|\$))" \
                          $INFOS ||
                {
                    echo "Symbol $SYMBOL is not documented" >&2
                    RC=1
                }
        done
        exit $RC
    }
