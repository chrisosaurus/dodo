#!/usr/bin/env sh

for infile in t/tests/exhaustive/*.in; do
    base=`echo $infile | sed 's/\.in$//g'`
    testfile="$base.tmp"

    echo "testing $base"
    cp $infile $testfile

    ./dodo $testfile < "$base.dodo"
    ret=$?
    if [ $ret -ne 0 ]; then
        echo "dodo failed: test failed for $base"
        exit 1
    fi

    diff $testfile "$base.out"
    ret=$?
    if [ $ret -ne 0 ]; then
        echo "output was not expected: test failed for $base"
        exit 1
    fi

    rm $testfile
done

echo "exhaustive testing completed successfully"

