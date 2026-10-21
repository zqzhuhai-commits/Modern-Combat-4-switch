#!/bin/bash
# MC4 Switch port build: sync the project into the devkitPro chroot, build
# the AArch32 runtime there, and copy the artifacts back.
set -euo pipefail
SRC=/home/user/Doubao/chats/38447866592439042/mc4-switch
CHROOT=/opt/dkp-root
JOBS="${JOBS:-4}"

sudo rm -rf "$CHROOT/root/mc4-build"
sudo mkdir -p "$CHROOT/root/mc4-build"
# Copy source, runtime, tools, portlibs; exclude build output and git.
(cd "$SRC" && tar --exclude='build' --exclude='.git' -cf - .) | sudo tar -xf - -C "$CHROOT/root/mc4-build"

sudo chroot "$CHROOT" /bin/bash -c '
set -euo pipefail
export DEVKITPRO=/opt/devkitpro
export PATH="$DEVKITPRO/devkitARM/bin:$DEVKITPRO/tools/bin:$PATH"
cd /root/mc4-build
make -f Game.mk -j'"$JOBS"' "$@"
' _ "$@"

# Copy artifacts back
sudo chown -R user:user "$CHROOT/root/mc4-build" 2>/dev/null || true
cp -r "$CHROOT/root/mc4-build/build" "$SRC/" 2>/dev/null || true
cp "$CHROOT/root/mc4-build"/mc4_game.elf "$SRC/" 2>/dev/null || true
cp "$CHROOT/root/mc4-build"/mc4_game.nsp "$SRC/" 2>/dev/null || true
echo "=== MC4 build artifacts ==="
ls -la "$SRC"/mc4_game.* "$SRC"/build/game/*.nsp 2>/dev/null || true
