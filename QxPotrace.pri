INCLUDEPATH += $$PWD/include

SOURCES += \
    $$PWD/src/qxpotrace.cpp \
    $$PWD/src/potrace-1.10/trans.c \
    $$PWD/src/potrace-1.10/trace.c \
    $$PWD/src/potrace-1.10/progress_bar.c \
    $$PWD/src/potrace-1.10/potracelib.c \
    $$PWD/src/potrace-1.10/decompose.c \
    $$PWD/src/potrace-1.10/curve.c \
    $$PWD/src/potrace-1.10/bbox.c

HEADERS  += \
    $$PWD/include/QxPotrace \
    $$PWD/src/qxpotrace.h \
    $$PWD/src/potrace-1.10/trans.h \
    $$PWD/src/potrace-1.10/trace.h \
    $$PWD/src/potrace-1.10/progress_bar.h \
    $$PWD/src/potrace-1.10/progress.h \
    $$PWD/src/potrace-1.10/potracelib.h \
    $$PWD/src/potrace-1.10/main.h \
    $$PWD/src/potrace-1.10/lists.h \
    $$PWD/src/potrace-1.10/decompose.h \
    $$PWD/src/potrace-1.10/curve.h \
    $$PWD/src/potrace-1.10/bitmap.h \
    $$PWD/src/potrace-1.10/bbox.h \
    $$PWD/src/potrace-1.10/auxiliary.h


DEFINES += POTRACE=\"\\\"potrace\\\"\"
DEFINES += MKBITMAP=\"\\\"mkbitmap\\\"\"
DEFINES += VERSION=\"\\\"1.10\\\"\"
DEFINES += PACKAGE=\"\\\"potrace\\\"\"
