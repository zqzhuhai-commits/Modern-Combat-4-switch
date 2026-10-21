#!/usr/bin/env python3
"""Create reviewable frontend binaries and source archives, without private inputs."""
from pathlib import Path
import hashlib,json,zipfile
root=Path(__file__).resolve().parents[1];build=root/'frontend/build';out=root/'release';out.mkdir(exist_ok=True)
version='0.1.1'
manifest=json.loads((build/'manifest.json').read_text())
for name,meta in manifest['files'].items():assert hashlib.sha256((build/name).read_bytes()).hexdigest()==meta['sha256']
with zipfile.ZipFile(out/f'Shadow-Guardian-Frontend-{version}.zip','w',zipfile.ZIP_DEFLATED) as z:
 for ext in ['nsp','nro']:
  src=build/f'Shadow-Guardian-Frontend-{version}.{ext}'
  dest=f'孤侠魅影-前端预览-{version}.nsp' if ext=='nsp' else 'switch/shadowguardian_nx/shadowguardian_nx.nro'
  z.write(src,dest)
 for path in [root/'README.md',root/'LICENSE',root/'THIRD_PARTY.md',*sorted((root/'docs').glob('*'))]:
  if path.is_file():z.write(path,path.relative_to(root))
 z.write(build/'manifest.json','manifest.json')
 z.write(root/'frontend/assets/icon.jpg','icon-256.jpg')
 z.write(root/'frontend/assets/icon-source.png','icon-source.png')
 z.write(root/'frontend/assets/FONT-LICENSE.txt','licenses/FONT-LICENSE.txt')
 for p in (root/'licenses').glob('*'):
  if p.is_file():z.write(p,'licenses/'+p.name)
 for p in (build/'previews').glob('*.png'):z.write(p,'interface-previews/'+p.name)
 z.write(root/'frontend/tools/prepare_data.py','prepare_data.py')
with zipfile.ZipFile(out/f'Shadow-Guardian-Source-{version}.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(root.rglob('*')):
  rel=p.relative_to(root)
  if not p.is_file() or any(x in {'.git','build','release','__pycache__'} for x in rel.parts):continue
  if p.suffix.lower() in {'.nsp','.nro','.elf','.o','.pyc','.keys','.apk','.obb'}:continue
  z.write(p,'shadowguardian-switch/'+str(rel))
for p in sorted(out.glob('*.zip')):
 with zipfile.ZipFile(p) as z:assert z.testzip() is None
 print(p.name,p.stat().st_size,hashlib.sha256(p.read_bytes()).hexdigest())
