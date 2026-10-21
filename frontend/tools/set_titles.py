#!/usr/bin/env python3
"""Populate every NACP language slot with the requested Chinese game name."""
from pathlib import Path
p=Path(__file__).resolve().parents[1]/'build/package/control/control.nacp'
d=bytearray(p.read_bytes());assert len(d)==0x4000
entry=d[:0x300]
assert entry.startswith('孤侠魅影\0'.encode())
for i in range(16):d[0x300*i:0x300*(i+1)]=entry
p.write_bytes(d)
