#!/bin/bash

if [ "${XDEV68K_DIR}" == "" ]; then
  echo "error: XDEV68K_DIR environment variable is not defined."
  exit 1
fi

MPU_TYPE=68060
BUILD_DIR=_build060

CC=${XDEV68K_DIR}/m68k-toolchain/bin/m68k-elf-gcc
GAS2HAS="${XDEV68K_DIR}/util/x68k_gas2has.pl -cpu ${MPU_TYPE} -inc doscall.inc -inc iocscall.inc"
RUN68=${XDEV68K_DIR}/run68/run68x
HAS=${XDEV68K_DIR}/x68k_bin/HAS060.X

INCLUDE_FLAGS="-I${XDEV68K_DIR}/include/xc -I${XDEV68K_DIR}/include/xdev68k"
COMMON_FLAGS="-m${MPU_TYPE} -O3 ${INCLUDE_FLAGS} -z-stack=32768"
CFLAGS="${COMMON_FLAGS} -Wno-builtin-declaration-mismatch -fcall-used-d2 -fcall-used-a2 \
    -fexec-charset=cp932 -fverbose-asm -fno-defer-pop -DFPM_DEFAULT -D_TIME_T_DECLARED -D_CLOCK_T_DECLARED -Dwint_t=int \
		-DXDEV68K -DFPM_DEFAULT -DOPT_SPEED"

LIBS="${XDEV68K_DIR}/lib/xc/CLIB.L ${XDEV68K_DIR}/lib/xc/DOSLIB.L ${XDEV68K_DIR}/lib/xc/IOCSLIB.L ${XDEV68K_DIR}/lib/xc/FLOATFNC.L ${XDEV68K_DIR}/lib/m68k_elf/m68000/libgcc.a"

function do_compile() {
  pushd .
  cd $1
  rm -rf ${BUILD_DIR}
  mkdir -p ${BUILD_DIR}
  for c in $2; do
    echo "compiling ${c}.c in ${1}"
	  ${CC} -S ${CFLAGS} -o ${BUILD_DIR}/${c}.m68k-gas.s ${c}.c
    if [ ! -f ${BUILD_DIR}/${c}.m68k-gas.s ]; then
      return 1
    fi
	  perl ${GAS2HAS} -i ${BUILD_DIR}/${c}.m68k-gas.s -o ${BUILD_DIR}/${c}.s
	  rm -f ${BUILD_DIR}/${c}.m68k-gas.s
	  ${RUN68} ${HAS} -e -u -w0 ${INCLUDE_FLAGS} ${BUILD_DIR}/${c}.s -o ${BUILD_DIR}/${c}.o
    if [ ! -f _build/${c}.o ]; then
      return 1
    fi
  done
  popd
  return 0
}

function build_libmad() {
  do_compile . "bit decoder fixed frame huffman layer3 layer12 stream synth timer version"
}

build_libmad
