#!/bin/bash

# figure out where we are
basedir=`dirname "$(readlink -f $0)"`

# figure out which action to perform
action="$1"

case "$action" in
install)
  ;;
uninstall)
  ;;
*)
  echo "Unrecognized action: $action"
  echo "Usage: $0 [install|uninstall] [--without-dxgi] [--symlink]"
  exit 1
esac

# process arguments
shift

with_dxgi=1
file_cmd="cp"

while [ $# -gt 0 ]; do
  case "$1" in
  "--without-dxgi")
    with_dxgi=0
    ;;
  "--symlink")
    file_cmd="ln -s"
    ;;
  esac
  shift
done

# check wine prefix before invoking wine, so that we
# don't accidentally create one if the user screws up
if [ -n "$WINEPREFIX" ] && ! [ -f "$WINEPREFIX/system.reg" ]; then
  echo "$WINEPREFIX:"' Not a valid wine prefix.' >&2
  exit 1
fi

# find wine executable
export WINEDEBUG=-all

if [ -z "$wine" ]; then
  wine="wine"
fi

wine64="${wine}64"

# resolve 32-bit and 64-bit system32 path
winever=`$wine --version | grep wine`
if [ -z "$winever" ]; then
    echo "$wine:"' Not a wine executable. Check your $wine.' >&2
    exit 1
fi

win32_sys_path=$($wine winepath -u 'C:\windows\system32' 2> /dev/null)
win64_sys_path=$($wine64 winepath -u 'C:\windows\system32' 2> /dev/null)

if [ -z "$win32_sys_path" ] && [ -z "$win64_sys_path" ]; then
  echo 'Failed to resolve C:\windows\system32.' >&2
  exit 1
fi

# create native dll override
overrideDll() {
  $wine reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v $1 /d native /f >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "Failed to add override for $1"
    exit 1
  fi
}

# remove dll override
restoreDll() {
  $wine reg delete 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v $1 /f > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo "Failed to remove override for $1"
  fi
}

# copy or link dxvk dll, back up original file
installFile() {
  dstfile="${1}/${3}.dll"
  srcfile="${basedir}/${2}/${3}.dll"

  if [ -f "${srcfile}.so" ]; then
    srcfile="${srcfile}.so"
  fi
  
  if ! [ -f "${srcfile}" ]; then
    echo "${srcfile}: File not found" >&2
    exit 1
  fi
  
  if [ -n "$1" ]; then
    if [ -f "${dstfile}" ]; then
      if ! [ -f "${dstfile}.old" ]; then
        mv "${dstfile}" "${dstfile}.old"
      else
        rm "${dstfile}"
      fi
      $file_cmd "${srcfile}" "${dstfile}"
    else
      echo "${dstfile}: File not found in wine prefix" >&2
      exit 1
    fi
  fi
}

# remove dxvk dll, restore original file
uninstallFile() {
  dstfile="${1}/${2}.dll"
  
  if [ -f "${dstfile}.old" ]; then
    rm "${dstfile}"
    mv "${dstfile}.old" "${dstfile}"
  fi
}

install() {
  installFile "$win32_sys_path" "x32" "$1"
  installFile "$win64_sys_path" "x64" "$1"
  overrideDll "$1"
}

uninstall() {
  uninstallFile "$win32_sys_path" "$1"
  uninstallFile "$win64_sys_path" "$1"
  restoreDll "$1"
}

# skip dxgi during install if not explicitly
# enabled, but always try to uninstall it
if [ $with_dxgi -ne 0 ] || [ "$action" == "uninstall" ]; then
  $action dxgi
fi

$action d3d10
$action d3d10_1
$action d3d10core
$action d3d11
