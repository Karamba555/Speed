#!/bin/bash -e

source=$1

if [ -z "$source" ] ; then
  echo "Usage: $0 <source> [ -b <board ] [--boost]" 1>&2
  exit 1
fi

shift

if [ "$1" == "-dn" ] ; then
  displayName=true
  shift
else
  displayName=false
fi

prefix=$TOPDIR/staging_dir/toolchain-aarch64_cortex-a53_gcc-12.3.0_musl/bin/aarch64-openwrt-linux-

include=$STAGING_DIR/usr/include/
openwrt_lib=$STAGING_DIR/usr/lib/
# CFLAGS="-I $include -I $staging_include"
CFLAGS="-I $include"
# LIB="-L $staging_lib -L $openwrt_lib -Wl,-rpath-link=$openwrt_lib -W"
LIB="-L $openwrt_lib -Wl,-rpath-link=$openwrt_lib -W"
pkg_config=$STAGING_DIR/host/bin/pkg-config.real
cmake=$TOPDIR/staging_dir/host/bin/cmake
  
export CC="${prefix}gcc $CFLAGS $LIB"
export CXX="${prefix}g++ $CFLAGS $LIB"
export LD="${prefix}ld $LIB"

cmake_vars="-DPKG_CONFIG_EXECUTABLE=$pkg_config"

if $displayName; then
  cmake_vars="$cmake_vars -DAWC_DISPLAY_NAME=$awc_display_name"
fi
  
openssl=${openwrt_lib}libcrypto.so  
cmake_vars="$cmake_vars -DOPENSSL_CRYPTO_LIBRARY:FILEPATH=$openssl"


# Newer cmake
#$cmake $boost -S $source -B . $@
# Older cmake
$cmake $cmake_vars $source $@

