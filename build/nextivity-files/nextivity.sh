#!/bin/bash -e

cmake=$TOPDIR/build/nextivity-files/cmake.sh

export awc_display_name="Speedway"

set_lua_version() {
  rootfs=$TOPDIR/build_dir/target-aarch64_cortex-a53_musl/root-mediatek
  lua_version=usr/lib/lua/luci/version.lua

  if [ -n "$rootfs" ] && [ -e $rootfs/$lua_version ] ; then
	# Replace the value of luciname
	sed -i 's/^luciname\s*=\s*".*"/luciname    = "SHIELD MegaFi 2"/' $rootfs/$lua_version

        version=$(cat  $TOPDIR/Version)
	# Replace the value of luciversion
	sed -i "s/^luciversion\s*=\s*\"unknown\"/luciversion = \"$version\"/" $rootfs/$lua_version
  else
    echo "*** No Lua version file to process in OpenWrt development code at $rootfs *** "
  fi
}

build_cmake_cgi() {
  target=$1
  src=$2

  shift 2
  binaries=$@

  if [ -z "${binaries}" ] ; then
    binaries=${target}
  fi

  src=$TOPDIR/build/nextivity-files/${src}
  dst=$TOPDIR/build_dir/target-aarch64_cortex-a53_musl/${target}

  mkdir -p "${dst}"
  cd "${dst}"
  $cmake "${src}" -dn
  make

  # cgi_bin=$STAGING_DIR/root-mediatek/www/cgi-bin
  cgi_bin=$TOPDIR/build_dir/target-aarch64_cortex-a53_musl/root-mediatek/www/cgi-bin
  mkdir -p "${cgi_bin}/actions"

  for cgi in modem-status power-cycle reboot; do 
    cp -av "${dst}"/${cgi} "${cgi_bin}"/actions
  done

  cp -av "${dst}"/factory.cgi "${cgi_bin}"/
  cp -av "${dst}"/cgi-userdetails "${cgi_bin}"/
  
  cd "${TOPDIR}"
}

build_cmake_target() {
  target=$1
  src=$2

  shift 2
  binaries=$@

  if [ -z "${binaries}" ] ; then
    binaries=${target}
  fi

  src=$TOPDIR/build/nextivity-files/${src}
  dst=$TOPDIR/build_dir/target-aarch64_cortex-a53_musl/${target}

  mkdir -p "${dst}"
  cd "${dst}"
  $cmake "${src}" -dn
  make

  cp -av "${dst}"/"${target}" "${TOPDIR}"/build_dir/target-aarch64_cortex-a53_musl/root-mediatek/usr/bin
  cd "${TOPDIR}"
}

build_cmake_cgi cgi-handlers cgi-handlers

echo "Adjusting CGI links"
#Replacing links with actual file
for cgi in download backup exec upload; do
  bin=${cgi_bin}/cgi-${cgi}
  if [ -f "${bin}" ]; then
    rm "${bin}"
  fi
  cp -av "$TOPDIR"/build_dir/target-aarch64_cortex-a53_musl/root-mediatek/usr/libexec/cgi-io "${bin}"
done

build_cmake_target password-megafi password-megafi
build_cmake_target speedway-gpsd speedway-gpsd

echo "Copying LuCi static files"
cp -r "$TOPDIR"/build/nextivity-files/www "$TOPDIR"/build_dir/target-aarch64_cortex-a53_musl/root-mediatek

if [ "$www" = "ro" ]; then
  echo "Set read-only permission for www folder"
  sudo chmod -R 555 "$TOPDIR"/build_dir/target-aarch64_cortex-a53_musl/root-mediatek/www
fi

#Change build version for LuCi UI
fw_version=$(cat "$TOPDIR"/Version)
echo "Firmware Version: ${fw_version}" > "$TOPDIR"/files/etc/nextivity_build_info
echo "Target board: SHIELD" >> "$TOPDIR"/files/etc/nextivity_build_info
cp -r "$TOPDIR"/files/etc/nextivity_build_info "$TOPDIR"/build_dir/target-aarch64_cortex-a53_musl/root-mediatek/etc/nextivity_build_info

set_lua_version
