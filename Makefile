# dodo - dodo dodo dodo dodo dodo dodo dodo
# See LICENSE file for copyright and license details.

.POSIX:

include config.mk

SRC = dodo.c
OBJ = ${SRC:.c=.o}
ASAN = -fsanitize=address,undefined -fno-omit-frame-pointer

all: options dodo

options:
	@echo dodo build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -g -c ${CFLAGS} ${ASAN} $<

${OBJ}: config.mk

dodo: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${LDFLAGS} ${ASAN} ${OBJ}

clean:
	@echo cleaning
	@rm -f dodo ${OBJ} dodo-${VERSION}.tar.gz
	@echo removing gcov files
	@find . -iname '*.gcda' -delete
	@find . -iname '*.gcov' -delete
	@find . -iname '*.gcno' -delete

dist: clean
	@echo creating dist tarball 'dodo-${VERSION}.tar.gz'
	@mkdir -p dodo-${VERSION}
	@cp -R config.mk dodo.1 LICENSE \
		Makefile README.md test.sh ${SRC} \
		dodo-${VERSION}
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

debug:
	@echo Building dodo debug build
	@echo CC -o $@
	@${CC} -g -c ${DEBUG_CFLAGS} ${SRC}
	@${CC} -o dodo ${DEBUG_LDFLAGS} ${OBJ}

test: debug
	@echo Running t/basic.sh
	@./t/basic.sh
	@echo Running more exhaustive t/exhaustive.sh
	@./t/exhaustive.sh
	@echo Running interactive t/interactive.sh
	@./t/interactive.sh
	@echo ""
	@echo "all tests passed"

.PHONY: all options clean dist install uninstall test debug
