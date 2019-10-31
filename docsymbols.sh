#!/bin/sh

for f; do
    case "$f" in
        *.o|*.a)
            ${NM:-nm} -P -g "$f" | \
                ${SED:-sed}  -n \
                             's/^\([[:alpha:]_][[:alnum:]_]*\) [BCDGRSTVW] .*$/\1/p'
            ;;
        *.h)
            ${SED:-sed} -n \
                        '/@ifset INTERNALS/,/@end ifset/!{
                                 /static inline/,/{/s/^\([[:alpha:]_][[:alnum:]_]*\)(.*$/\1/p 
                                 s/^typedef \([[:alpha:]_][[:alnum:]_]* \)*\([[:alpha:]_][[:alnum:]_]*\) *[;{]$/\2/p
                                 s/^enum \([[:alpha:]_][[:alnum:]_]*\).*$/\1/p
                                 /TNH_[[:alnum:]_]*_H/!s/^#define \([[:alpha:]_][[:alnum:]_]*\).*$/\1/p 
                        }' "$f"
            ;;
        *.info)
            # Just to support mixed dependency lists
            ;;
        *)
            echo "Unsupported file type: $f" >&2
            exit 1
            ;;
    esac
done
