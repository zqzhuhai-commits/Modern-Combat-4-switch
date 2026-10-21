#!/usr/bin/env python3
"""Copy only original game library/resources from the user's PortMaster ZIP.
No runtime Linux binaries, saves, logs, keys, or system libraries are copied.
"""
import argparse,hashlib,pathlib,shutil,zipfile
p=argparse.ArgumentParser();p.add_argument('archive',type=pathlib.Path);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--dry-run',action='store_true');a=p.parse_args()
known='ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9'
with zipfile.ZipFile(a.archive) as z:
 names=z.namelist();libs=[n for n in names if n.endswith('/lib/armeabi-v7a/libshadowguardian.so')]
 if len(libs)!=1:raise SystemExit('Expected exactly one original ARMv7 library')
 libname=libs[0];prefix=libname[:-len('lib/armeabi-v7a/libshadowguardian.so')]
 if hashlib.sha256(z.read(libname)).hexdigest()!=known:raise SystemExit('Unrecognized library SHA256; analysis required before preparing it')
 resource_prefix=prefix+'Gameloft/Games/GloftSGHP/'
 todo=[(libname,pathlib.PurePosixPath('libshadowguardian.so'))]
 for name in names:
  if not name.startswith(resource_prefix) or name.endswith('/'):continue
  relative=pathlib.PurePosixPath(name[len(resource_prefix):])
  if relative.is_absolute() or '..' in relative.parts or '\\' in name:raise SystemExit('Unsafe archive resource path')
  todo.append((name,pathlib.PurePosixPath('gameloft/games/GloftSGHP')/relative))
 if len(todo)<2:raise SystemExit('Original game resource folder is missing')
 if not a.dry_run:
  # Never overwrite an existing installation or save area.
  if a.output.exists() and any(a.output.iterdir()):raise SystemExit('Choose an empty output directory, then merge only after backing up your existing files')
  for name,relative in todo:
   dest=a.output.joinpath(*relative.parts);dest.parent.mkdir(parents=True,exist_ok=True)
   with z.open(name) as src,dest.open('wb') as dst:shutil.copyfileobj(src,dst)
 print(('DRY RUN: ' if a.dry_run else 'Prepared: ')+str(len(todo)-1)+' resource files + verified original library')
 print('Gameplay runtime remains pending; these are preparation files only.')
