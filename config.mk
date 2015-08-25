VERSION = 0.1

PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

INCS = 
LIBS = 

# NB: including  -fprofile-arcs -ftest-coverage for gcov
# travis wasn't happy with -Wmaybe-uninitialized  so removed for now
# -Wextra was removed due to unused params
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wstrict-prototypes -Wshadow -Wdeclaration-after-statement -Wunused-function -fprofile-arcs -ftest-coverage ${INCS}

# NB: including  -fprofile-arcs for gcov
LDFLAGS = -fprofile-arcs ${LIBS}


CC = cc
