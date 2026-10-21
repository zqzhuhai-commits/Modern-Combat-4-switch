#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${DEVKITPRO:?Set DEVKITPRO to your installed devkitPro prefix}"
export PATH="$DEVKITPRO/devkitA64/bin:$DEVKITPRO/tools/bin:$PATH"
mkdir -p build/package/{exefs,control,romfs}
flags=(-std=c11 -D_DEFAULT_SOURCE -D__SWITCH__ -O2 -g -Wall -Wextra -Werror -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -ffunction-sections -fdata-sections -Isource -I../runtime/source -I"$DEVKITPRO/libnx/include")
objects=()
for src in source/main.c source/ui.c source/scan.c ../runtime/source/rt_apkfind.c; do
 obj="build/$(basename "${src%.c}").o"
 aarch64-none-elf-gcc "${flags[@]}" -c "$src" -o "$obj"
 objects+=("$obj")
done
aarch64-none-elf-gcc -specs="$DEVKITPRO/libnx/switch.specs" -march=armv8-a+crc+crypto -mtp=soft -fPIE -Wl,--gc-sections -Wl,-Map,build/frontend.map "${objects[@]}" -L"$DEVKITPRO/libnx/lib" -lnx -o build/frontend.elf
elf2nso build/frontend.elf build/package/exefs/main
npdmtool npdm.json build/package/exefs/main.npdm
nacptool --create '孤侠魅影' 'Shadow Guardian NX project' '0.1.1' build/package/control/control.nacp --titleid=0100F53447550000
python3 tools/set_titles.py
cp assets/icon.jpg build/package/control/icon_AmericanEnglish.dat
elf2nro build/frontend.elf build/Shadow-Guardian-Frontend-0.1.1.nro --icon=assets/icon.jpg --nacp=build/package/control/control.nacp
cp STATUS.txt build/package/romfs/STATUS.txt
printf 'Native frontend built. Package with tools/package_nsp.py using a private local keyset.\n'
