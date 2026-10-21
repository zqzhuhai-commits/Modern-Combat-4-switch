#!/usr/bin/env python3
"""Exercise production scanner against synthetic, non-game fixtures."""
import ctypes,pathlib,subprocess,tempfile,zipfile
root=pathlib.Path(__file__).resolve().parents[1]
build=root/'build';build.mkdir(exist_ok=True)
subprocess.run(['cc','-std=c11','-D_DEFAULT_SOURCE','-Wall','-Wextra','-Werror','-shared','-fPIC','-I'+str(root/'source'),'-I'+str(root.parent/'runtime/source'),str(root/'source/scan.c'),str(root.parent/'runtime/source/rt_apkfind.c'),'-o',str(build/'scan-test.so'),'-lcrypto'],check=True)
class Scan(ctypes.Structure):
 _fields_=[('apk_candidates',ctypes.c_int),('data_files',ctypes.c_int),('limited',ctypes.c_int),('errors',ctypes.c_int),('library_present',ctypes.c_int),('library_verified',ctypes.c_int),('data_bytes',ctypes.c_uint64),('apk',ctypes.c_char*768),('library',ctypes.c_char*768),('data_path',ctypes.c_char*1024),('library_sha256',ctypes.c_char*65)]
lib=ctypes.CDLL(str(build/'scan-test.so'))
lib.scan_files.argtypes=[ctypes.c_char_p,ctypes.POINTER(Scan)]
lib.save_report.argtypes=[ctypes.c_char_p,ctypes.POINTER(Scan)]
def scan(p):
 s=Scan();lib.scan_files(str(p).encode(),ctypes.byref(s));return s
with tempfile.TemporaryDirectory() as td:
 p=pathlib.Path(td);s=scan(p/'missing');assert not s.apk_candidates and not s.data_files
 (p/'bad.apk').write_bytes(b'not a zip')
 with zipfile.ZipFile(p/'wrong.apk','w') as z:z.writestr('AndroidManifest.xml',b'fixture')
 assert scan(p).apk_candidates==0
 (p/'libshadowguardian.so').write_bytes(b'not a supported library')
 s=scan(p);assert s.library_present and not s.library_verified
 with zipfile.ZipFile(p/'candidate.apk','w') as z:
  z.writestr('AndroidManifest.xml',b'fixture, not a real manifest')
  z.writestr('lib/armeabi-v7a/libshadowguardian.so',b'fixture, not a real ELF')
 d=p/'gameloft/games/GloftSGHP';d.mkdir(parents=True)
 (d/'empty').touch();(d/'asset').write_bytes(b'test');(d/'nested').mkdir();(d/'nested/file').write_bytes(b'12345')
 (d/'cycle').symlink_to(d,target_is_directory=True)
 s=scan(p);assert s.apk_candidates==1 and s.data_files==2 and s.data_bytes==9 and not s.limited
 assert lib.save_report(str(p).encode(),ctypes.byref(s))==0
 assert 'NOT validated' in (p/'frontend.log').read_text()
 assert (d/'asset').read_bytes()==b'test'
 # Directory budget/depth cannot lock up on unbounded trees.
 deep=d
 for _ in range(9):deep=deep/'more';deep.mkdir()
 assert scan(p).limited==1
 assert lib.save_report(str(p/'absent').encode(),ctypes.byref(s))==-1
print('PASS: missing/invalid/wrong/candidate APK, nested/empty files, symlink loop, limits, report, write failure')
# Optional original library: check the same production hash path used on Switch.
if __name__=='__main__':
 import sys,shutil,hashlib
 if len(sys.argv)>1:
  with tempfile.TemporaryDirectory() as td:
   p=pathlib.Path(td);shutil.copyfile(sys.argv[1],p/'libshadowguardian.so')
   s=scan(p)
   assert s.library_present and s.library_verified
   assert s.library_sha256.decode()==hashlib.sha256((p/'libshadowguardian.so').read_bytes()).hexdigest()
  print('PASS: supplied original library SHA256 recognized by production scanner')
