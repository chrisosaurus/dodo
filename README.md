dodo
====
dodo - scriptable in place file editor

WARNING
-------
dodo is a VERY early on work in progress, not yet recommended for actual use.


Description
-----------
dodo is a non-interactive scriptable in place file editor.


Purpose
-------
dodo was born from the need to efficiently edit very large files (16GB plain sql dumps),
I was unable to so using my normal toolset (ed, sed or vim) and I realised that writing a tool
specifically for this use case would be very simple.


Building
--------

    cd dodo
    make
    make test


Usage
-----
dodo is run and supplied with a single argument representing the filename to work on.

dodo then reads it's commands from stdin,
note that dodo is non-interactive so will not start work
until it's stdin input is finished (it sees EOF).

note that in dodo all changes are flushed immediately; there are no concepts of 'saving', 'undo' or 'backups'.

example dodo usage:

    ./dodo $TESTFILENAME <<EOF
    e/hello/
    b6
    e/world/
    w/marge/
    q
    EOF

Explanation
-----------
here is the above example with comments included:

    echo -e "\nRunning dodo"
    ./dodo $TESTFILENAME <<EOF
    e/hello/   # expect string 'hello'
    b6         # goto byte 6 in file
    e/world/   # expect string 'world'
    w/marge/   # write string 'marge' (writes over 'world')
    q          # quit
    EOF

each of the commands in explained below in more detail.


dodo currently supports the following commands:


expect:

    e/string/

check for 'string' at current cursor position, exit with error if not found


byte:

    bnumber

move cursor to absolute byte 'number' within file


write:

    w/string/

write string to current cursor position, this will overwrite any characters in the way


quit:

    q

exit dodo


dodo also supports comments

    # this is a comment and spans until \n



