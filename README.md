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
I was unable to do so using my normal toolset (ed, sed or vim) and I realised that writing a tool
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
until it's stdin input is finished (it sees eof).

in dodo all changes are flushed immediately; there are no concepts of 'saving', 'undo' or 'backups'.

dodo is really a very thin wrapper around `fread` and `fwrite`.

example dodo usage:

    ./dodo filename <<EOF
        p          # print 100 bytes
        p5         # print 5 bytes ('hello')
        e/hello/   # expect string 'hello'
        b6         # goto byte 6 in file
        e/world/   # expect string 'world'
        w/marge/   # write string 'marge' (writes over 'world')
        b38
        e/sl\/ash/ # expect string 'sl/ash'
        w/slashy/  # write string 'sl/ash' with 'slashy'
        q          # quit
    EOF

each of the commands is explained below in more detail.


Commands
--------

dodo currently supports the following commands and syntax:

**print**

    p
    pnumber

print specified number of bytes, if number is not specified will default to 100

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


Syntax
------

**comments:**
dodo also supports comments

    # this is a comment and spans until \n


**whitespace:**

    in dodo whitespace is non-significant except in the case of a newline ('\n') terminating a comment

**escape character:**
a backslash can be used as an escape character, useful mainly when the expected string or replacement string has to include a string delimiter (forward slash)

    e/foo\/bar/
    w/baz\\qux/

will replace `foo/bar` with `baz\qux`

Contributions
-------------
Chris Hall (original author)
David Phillips


