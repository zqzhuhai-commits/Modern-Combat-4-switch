#!/bin/bash
# MC4 hosted build: reuse the MC3 session toolchain (no Docker).
set -e
MC3=/home/user/Doubao/chats/38448573415630594/mc3-switch
ROOT="$MC3/toolchain"
export DEVKITPRO="$ROOT/dkp-root/opt/devkitpro"
export DEVKITARM="$DEVKITPRO/devkitARM"
[ -d "$DEVKITPRO/libnx32" ] || ln -s "$ROOT/libnx32/prefix" "$DEVKITPRO/libnx32"
export PATH="$DEVKITARM/bin:$PATH"
cd "$M"
make -f Game.mk -j"${JOBS:-4}" "$@"
echo "==> built: $M/mc4_game.nsp ($M/Modern-Combat-4.nsp)"
