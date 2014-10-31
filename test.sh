#!/usr/bin/env bash
set -e

TESTFILENAME="tmp_testing_file";

echo -e "\nWriting file"
cat <<EOF > $TESTFILENAME
hello world how are you
EOF

echo -e "\nRunning dodo"
./dodo $TESTFILENAME <<EOF
e/hello/
b6
e/world/
w/marge/
q
EOF

echo -e "\nComparing output"
GOT=`cat $TESTFILENAME`
EXPECTED="hello marge how are you"

if [ ! "$GOT" = "$EXPECTED" ]; then
    echo -e "\nTest failed:"
    echo "Got '$GOT'"
    echo "Expected '$EXPECTED'"
    # do not clean up on failure to allow inspection of file
    #rm -f $TESTFILENAME
    echo "Leaving tmp file laying around as '$TESTFILENAME'"
    exit 1;
fi


echo -e "\nAll green"

echo -e "\nRemoving temporary file"
rm -f $TESTFILENAME

