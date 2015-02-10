dodo
====
dodo - scriptable in place file editor

WARNING
-------
dodo is a VERY early on work in progress, **not yet recommended for actual use**.


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

    ./dodo filename <<EOF
        e/hello/   # expect string 'hello'
        b6         # goto byte 6 in file
        e/world/   # expect string 'world'
        w/marge/   # write string 'marge' (writes over 'world')
        q          # quit
    EOF

each of the commands is explained below in more detail.


Commands
--------

dodo currently supports the following commands and syntax:


**expect:**

    e/string/

check for 'string' at current cursor position, exit with error if not found.

expect does not move the cursor.


**byte:**

    bnumber

move cursor to absolute byte 'number' within file


**write:**

    w/string/

write 'string' to current cursor position, this will overwrite any characters in the way

write moves the cursor by the number of bytes written


**quit:**

    q

exit dodo, quit is not actually needed as EOF will trigger an implicit quit.


**comments:**
dodo also supports comments

    # this is a comment and spans until \n


**whitespace:**

    in dodo whitespace is non-significant except in the case of a newline ('\n') terminating a comment



