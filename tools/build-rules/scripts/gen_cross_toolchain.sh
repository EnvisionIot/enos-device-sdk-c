#! /bin/bash

procing() {
    trap 'exit 0;' 6
    while :; do
        for j in '-' '\\' '|' '/'; do
            tput sc
            echo -ne  "[$j]"
            sleep 0.1
            tput rc
        done
    done
}

waiting() {
    local pid="$1"

    echo -ne "\033[1;32m"
    echo -n "$2 .................... "
    procing &

    local tmppid="$!"
    wait $pid
    tput rc

    echo "done"
    echo -ne "\033[0m"
    echo ""
    kill -6 $tmppid >/dev/null 1>&2
}

LOCAL_CC=${TOOLCHAIN_DLDIR}/${RELPATH}/${CC}
LOCAL_AR=${TOOLCHAIN_DLDIR}/${RELPATH}/${AR}

# echo "[RELPATH]: ${RELPATH}"
# echo "[GITPATH]: ${GITPATH}"

which ${CC} > /dev/null 2>&1

if [ $? != 0 ] && [ ! -f ${LOCAL_CC} ]; then

    echo "Unable to find available toolchain of [${CC}] from local ENV or Internet. Abort!" | grep --color ".*"
    echo ""
    exit 1

fi
