BUILDDIR?=linux/gcc/aarch64

include config/base.mk
include config/extra/with-security.mk
include config/extra/with-gcc.mk
include config/extra/with-aarch64.mk
include config/extra/with-debug.mk
include config/extra/with-brutality.mk
include config/extra/with-optimization.mk
include config/extra/with-threads.mk

# CPPFLAGS+=-march=haswell -mtune=skylake
CCPFLAGS+=-mcpu=neoverse-n1
CPPFLAGS+=-DFD_HAS_INT128=1 -DFD_HAS_DOUBLE=1 -DFD_HAS_ALLOCA=1
# -DFD_HAS_X86=1 -DFD_HAS_SSE=1 -DFD_HAS_AVX=1

FD_HAS_INT128:=1
FD_HAS_DOUBLE:=1
FD_HAS_ALLOCA:=1
# FD_HAS_X86:=1
# FD_HAS_SSE:=1
# FD_HAS_AVX:=1
