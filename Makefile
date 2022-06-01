# $Id: Makefile 5 2011-01-31 03:48:23Z henry_groover $
# Makefile for licut

OUTPUT:=./output
HOST_TARGET:=$(shell uname -m)-$(shell uname -s | tr 'L' 'l')
# Override TARGET for cross-compile
TARGET:=${HOST_TARGET}
TGT:=
ifeq (${TARGET},arm-linux)
TGT:=arm-linux-
endif
PACKAGE:=${OUTPUT}/licut-${TARGET}.tgz
SOURCES:=$(wildcard src/*.cpp)
BINDIR:=${TARGET}/bin
OBJDIR:=${TARGET}/obj
LIBDIR:=
OBJS:=$(patsubst %.cpp,${OBJDIR}/%.o,${SOURCES})
LIBS:=${GFLAGS_LIB}
LIB_PATHS:=$(addprefix ${LIBDIR}/,${LIBS})
LDFLAGS += -lgflags ${LIB_PATHS}
CFLAGS += -lgflags


LICUT:=${TARGET}/bin/licut

all: ${PACKAGE}

${PACKAGE}: ${LICUT} ${OUTPUT} ${OUTPUT}/${TARGET}
	cp ${LICUT} ${OUTPUT}/${TARGET};
	cp *.md ${OUTPUT}/${TARGET}
	# svn export --force ../doc ${OUTPUT}/${TARGET}/doc;
	tar czf ${OUTPUT}/$(@F) -C ${OUTPUT}/${TARGET} licut README.md Using_Inkscape.md
	@ls -l $@

${OUTPUT} ${OUTPUT}/${TARGET} ${LIBDIR}:
	mkdir -p $@

clean:
	rm -rf ${LICUT} ${OBJS} ${PACKAGE} ${TARGET} output

.PHONY: all clean install uninstall

${LICUT}: ${OBJS} ${LIB_PATHS}
	@mkdir -p $(dir $@)
	${TGT}${CXX} -o $@ ${OBJS} ${LDFLAGS}
	cp $@ $@.debug
	${TGT}strip $@

${OBJDIR}/%.o: %.cpp
	@mkdir -p $(dir $@)
	${TGT}${CXX} ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

install: ${LICUT}
	cp ${LICUT} /usr/local/bin

uninstall:
	rm -f /usr/local/bin/licut
