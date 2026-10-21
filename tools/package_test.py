#!/usr/bin/env python3
"""Package the standalone AArch32 boot test; never include the original game library."""
import argparse,hashlib,json,os,pathlib,shutil,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--keyset',type=pathlib.Path,required=True);p.add_argument('--tool',default='hacbrewpack');p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('build/game'))
a=p.parse_args();root=pathlib.Path(__file__).resolve().parents[2];build=root/'build/game';dkp=pathlib.Path(os.environ['DEVKITPRO'])
assert (build/'exefs/main').read_bytes()[:4]==b'NSO0'
npdm=(build/'exefs/main.npdm').read_bytes();assert npdm[:4]==b'META' and not npdm[12]&1
with tempfile.TemporaryDirectory(prefix='sg-runtime-package-') as temp:
 t=pathlib.Path(temp)
 for name in ['control','romfs']: (t/name).mkdir()
 subprocess.run([str(dkp/'tools/bin/nacptool'),'--create','孤侠魅影','Shadow Guardian NX project','0.2.1',str(t/'control/control.nacp'),'--titleid=0100F53447550000'],check=True)
 cp=t/'control/control.nacp';d=bytearray(cp.read_bytes());entry=d[:0x300]
 for i in range(16):d[i*0x300:(i+1)*0x300]=entry
 cp.write_bytes(d)
 shutil.copyfile(root/'frontend/assets/icon.jpg',t/'control/icon_AmericanEnglish.dat')
 (t/'romfs/STATUS.txt').write_text('Shadow Guardian runtime 0.2.1\nExperimental original-game boot test.\nExecutes original constructors/JNI/render loop after A.\n720p output and asset/JNI fixes; new build NOT hardware-verified.\nOriginal game library and resources must remain on SD.\n')
 cmd=[a.tool,'--keyset',str(a.keyset.resolve()),'--exefsdir',str(build/'exefs'),'--romfsdir',str(t/'romfs'),'--controldir',str(t/'control'),'--nologo']
 for k in ['nsp','nca','temp','backup']:cmd+=['--'+k+'dir',str(t/k)]
 r=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=dict(os.environ,HACBREWPACK_TITLE_VERSION='131073'))
 files=list((t/'nsp').glob('*.nsp'))
 if r.returncode or len(files)!=1:raise SystemExit('NSP packaging failed; tool output withheld to protect local keys')
 a.out.mkdir(parents=True,exist_ok=True);dest=a.out/'孤侠魅影-运行层启动测试-0.2.1.nsp';shutil.copyfile(files[0],dest)
 print(json.dumps({'path':str(dest),'size':dest.stat().st_size,'sha256':hashlib.sha256(dest.read_bytes()).hexdigest(),'architecture':'AArch32','hardware_tested':False,'gameplay_verified':False},ensure_ascii=False))
