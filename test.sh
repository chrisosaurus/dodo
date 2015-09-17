#!/usr/bin/env bash

# check for valgrind
hash valgrind || exit


set -eu

TESTFILENAME=$(mktemp) || exit
VALGRINDOPTS="--track-origins=yes --error-exitcode=1 --leak-check=full --show-reachable=yes"

make_test_file() {
    echo -e "\nWriting file"
    cat <<EOF > "$TESTFILENAME"
hello world how are you mutter mutter sl/ash
EOF
}

warn() {
    if [[ -e $TESTFILENAME ]] ; then
        printf 'Leaving tmp file laying around as '\''%s'\''\n' "$TESTFILENAME"
        exit 1
    fi
}
do_test() {
    [ -z "$1" ] && exit
    echo -e "\nRunning $1"
    valgrind $VALGRINDOPTS $1 "$TESTFILENAME" <<EOF
        p          # print 100 bytes
        p5         # print 5 bytes ('hello')
        e/hello/   # expect string 'hello'
        b6         # goto byte 6 in file
        e/world/   # expect string 'world'
        w/marge/   # write string 'marge' (writes over 'world')
        b38
        e/sl\/ash/ # expect string 'sl/ash'
        w/slashy/  # write over string 'sl/ash' with 'slashy'
        q          # quit
EOF


    echo -e "\nComparing output"
    GOT=`cat "$TESTFILENAME"`
    EXPECTED="hello marge how are you mutter mutter slashy"

    if [ ! "$GOT" = "$EXPECTED" ]; then
        echo -e "\nTest failed:"
        echo "Got '$GOT'"
        echo "Expected '$EXPECTED'"
        # do not clean up on failure to allow inspection of file
        #rm -f -- "$TESTFILENAME"
        exit 1
    fi
}

trap warn EXIT

for dodo in "./dodo" "./dodo -i"; do
    make_test_file
    do_test "$dodo"
done

echo -e "\nAll green"

echo -e "\nRemoving temporary file"
rm -f -- "$TESTFILENAME"

