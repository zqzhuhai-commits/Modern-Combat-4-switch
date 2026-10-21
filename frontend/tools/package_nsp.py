#!/usr/bin/env python3
"""Build an installable frontend NSP. Key contents and packer output stay private."""
import os,argparse,hashlib,json,pathlib,shutil,subprocess,tempfile
p=argparse.ArgumentParser()
p.add_argument('--keyset',type=pathlib.Path,required=True)
p.add_argument('--tool',default='hacbrewpack')
p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('build'))
a=p.parse_args();base=pathlib.Path(__file__).resolve().parents[1]/'build/package'
for name,magic in [('exefs/main',b'NSO0'),('exefs/main.npdm',b'META')]:
 if (base/name).read_bytes()[:4]!=magic:raise SystemExit('Invalid native build input: '+name)
if not a.keyset.is_file():raise SystemExit('Private local keyset not found')
a.out.mkdir(parents=True,exist_ok=True)
with tempfile.TemporaryDirectory(prefix='sgfrontend-') as tmp:
 t=pathlib.Path(tmp)
 cmd=[a.tool,'--keyset',str(a.keyset.resolve()),'--exefsdir',str(base/'exefs'),'--romfsdir',str(base/'romfs'),'--controldir',str(base/'control'),'--nologo']
 for k in ['nsp','nca','temp','backup']:cmd+=['--'+k+'dir',str(t/k)]
 r=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=dict(os.environ,HACBREWPACK_TITLE_VERSION="65536"))
 files=list((t/'nsp').glob('*.nsp'))
 if r.returncode or len(files)!=1:raise SystemExit('NSP packaging failed; tool output withheld to protect local keys')
 out=a.out/'Shadow-Guardian-Frontend-0.1.1.nsp';shutil.copyfile(files[0],out)
 print(json.dumps({'file':str(out),'size':out.stat().st_size,'sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'hardware_tested':False,'gameplay_available':False}))
