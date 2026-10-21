#!/usr/bin/env python3
"""Verify boot-test NSP contents and its exact AArch32 executable segments."""
import argparse,hashlib,json,pathlib,struct,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--keyset',required=True);p.add_argument('--hactool',default='hactool');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[2];build=root/'build/game';package=build/'孤侠魅影-运行层启动测试-0.2.1.nsp'
def pfs(data):
 assert data[:4]==b'PFS0';count,sz=struct.unpack_from('<II',data,4);assert 0<count<100
 names=16+24*count;start=names+sz;out={}
 for i in range(count):
  off,n,k=struct.unpack_from('<QQI',data,16+24*i);name=data[names+k:names+sz].split(b'\0',1)[0].decode()
  assert '/' not in name and '..' not in name and start+off+n<=len(data)
  out[name]=data[start+off:start+off+n]
 return out
nso=(build/'exefs/main').read_bytes();assert nso[:4]==b'NSO0' and struct.unpack_from('<I',nso,12)[0]==0x38
elf=(root/'shadowguardian_game.elf').read_bytes();assert elf[:6]==b'\x7fELF\x01\x01' and struct.unpack_from('<H',elf,18)[0]==40
po=struct.unpack_from('<I',elf,28)[0];pe,pn=struct.unpack_from('<HH',elf,42);loads=[]
for i in range(pn):
 ph=struct.unpack_from('<8I',elf,po+i*pe)
 if ph[0]==1:loads.append(ph)
assert len(loads)==3
for i,ph in enumerate(loads):
 off,addr,size,bss=struct.unpack_from('<4I',nso,16+16*i)
 data=nso[off:off+size];assert addr==ph[2] and size==ph[4] and data==elf[ph[1]:ph[1]+ph[4]]
 assert hashlib.sha256(data).digest()==nso[0xa0+32*i:0xc0+32*i]
 if i==2:assert bss==ph[5]-ph[4]
files=pfs(package.read_bytes());assert len(files)==3 and all(n.endswith('.nca') for n in files)
checks={"nso_segments_match_elf":True}
with tempfile.TemporaryDirectory(prefix='sg-test-verify-') as temp:
 t=pathlib.Path(temp);cnmt=None;contained=[]
 for i,(name,data) in enumerate(files.items()):
  assert hashlib.sha256(data).hexdigest()[:32]==name.split('.')[0]
  path=t/name;path.write_bytes(data);out=t/str(i)
  for sub in ['exefs','romfs','section0']:(out/sub).mkdir(parents=True,exist_ok=True)
  r=subprocess.run([a.hactool,'-k',a.keyset,'--disablekeywarns','--suppresskeys','-x',str(path),'--exefsdir='+str(out/'exefs'),'--romfsdir='+str(out/'romfs'),'--section0dir='+str(out/'section0')],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  assert r.returncode==0
  if (out/'exefs/main').exists():
   assert (out/'exefs/main').read_bytes()==nso
   npdm=(out/'exefs/main.npdm').read_bytes();assert npdm[:4]==b'META' and not npdm[12]&1
   aci=npdm.find(b'ACI0');assert struct.unpack_from('<Q',npdm,aci+16)[0]==0x0100F53447550000
   assert list(p.name for p in (out/'romfs').iterdir())==['STATUS.txt']
   checks['program_matches']=checks['npdm_arm32']=True
  if (out/'romfs/control.nacp').exists():
   c=(out/'romfs/control.nacp').read_bytes()
   for j in range(16):assert c[j*0x300:].startswith('孤侠魅影\0'.encode())
   assert c[0x3060:].startswith(b'0.2.1\0')
   assert (out/'romfs/icon_AmericanEnglish.dat').read_bytes()==(root/'frontend/assets/icon.jpg').read_bytes()
   checks['control_name_version_icon']=True
  for v in (out/'section0').glob('*.cnmt'):cnmt=v.read_bytes()
 assert cnmt and struct.unpack_from('<QI',cnmt,0)==(0x0100F53447550000,131073)
 # CNMT extended header precedes the content records.
 ext,count=struct.unpack_from('<HH',cnmt,0xe)
 for i in range(count):
  entry=0x20+ext+i*0x38;hash_=cnmt[entry:entry+32];id_=cnmt[entry+32:entry+48].hex()
  found=files.get(id_+'.nca');assert found and hashlib.sha256(found).digest()==hash_
 checks['cnmt_content_hashes']=True
assert len(checks)==5
manifest={'version':'0.2.1','title':'孤侠魅影','title_id':'0100F53447550000','architecture':'AArch32','executes_original_initialization':True,'hardware_gameplay_verified':False,'checks':checks,'file':package.name,'size':package.stat().st_size,'sha256':hashlib.sha256(package.read_bytes()).hexdigest()}
(build/'runtime-manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n');print(json.dumps(manifest,indent=2,ensure_ascii=False))
