VERSION = 0.1

PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

INCS = 
LIBS = 

CFLAGS = -std=c99 -pedantic -Werror -Wall -Wstrict-prototypes -Wshadow -Wdeclaration-after-statement -Wunused-function ${INCS}
# NB: including  -fprofile-arcs -ftest-coverage for gcov
# travis wasn't happy with -Wmaybe-uninitialized  so removed for now
# -Wextra was removed due to unused params
DEBUG_CFLAGS = -fprofile-arcs -ftest-coverage ${CFLAGS}

LDFLAGS = ${LIBS}
# NB: including  -fprofile-arcs for gcov
DEBUG_LDFLAGS = -fprofile-arcs ${LDFLAGS}

CC = cc
