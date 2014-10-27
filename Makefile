# dodo - dodo dodo dodo dodo dodo dodo dodo
# See LICENSE file for copyright and license details.

.POSIX:

include config.mk

SRC = dodo.c
OBJ = ${SRC:.c=.o}

all: options dodo 

options:
	@echo dodo build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -g -c ${CFLAGS} $<

${OBJ}: config.mk

dodo: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${LDFLAGS} ${OBJ}

clean:
	@echo cleaning
	@rm -f dodo ${OBJ} dodo-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p dodo-${VERSION}
	@cp -R LICENSE Makefile config.mk \
		README TODO dodo.1 codes.h ${SRC} dodo-${VERSION}
	@tar -cf dodo-${VERSION}.tar dodo-${VERSION}
	@gzip dodo-${VERSION}.tar
	@rm -rf dodo-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dodo ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dodo
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < dodo.1 > ${DESTDIR}${MANPREFIX}/man1/dodo.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/dodo.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/dodo
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/dodo.1

test: dodo
	@echo Running test.sh
	@./test.sh

.PHONY: all options clean dist install uninstall test
