#!/usr/bin/env python3
"""Inspect native binaries and, optionally, decrypt NSP sections with a private local keyset."""
import argparse,hashlib,json,pathlib,struct,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--keyset');p.add_argument('--hactool',default='hactool');a=p.parse_args()
base=pathlib.Path(__file__).resolve().parents[1]/'build'
def u32(d,o):return struct.unpack_from('<I',d,o)[0]
def pfs(d):
 assert d[:4]==b'PFS0';count,ns=u32(d,4),u32(d,8);assert 0<count<100
 names=d[16+count*24:16+count*24+ns];start=16+24*count+ns;out={}
 for i in range(count):
  off,size,idx=struct.unpack_from('<QQI',d,16+i*24)
  name=names[idx:].split(b'\0',1)[0].decode();assert '/' not in name and '..' not in name
  assert start+off+size<=len(d);out[name]=d[start+off:start+off+size]
 return out
elf=(base/'frontend.elf').read_bytes();assert elf[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',elf,18)[0]==183
nro=(base/'Shadow-Guardian-Frontend-0.1.1.nro').read_bytes();assert nro[16:20]==b'NRO0'
aset=u32(nro,24);assert nro[aset:aset+4]==b'ASET'
iconoff,iconsize,nacpoff,nacpsize=struct.unpack_from('<QQQQ',nro,aset+8)
assert nro[aset+iconoff:aset+iconoff+iconsize]==(base.parent/'assets/icon.jpg').read_bytes()
nacp=nro[aset+nacpoff:aset+nacpoff+nacpsize];assert len(nacp)==0x4000 and nacp.startswith('孤侠魅影\0'.encode('utf-8'))
npdm=(base/'package/exefs/main.npdm').read_bytes();assert npdm[:4]==b'META' and npdm[12]&1
aci=npdm.find(b'ACI0');assert aci>=0 and struct.unpack_from('<Q',npdm,aci+16)[0]==0x0100F53447550000
nsp=base/'Shadow-Guardian-Frontend-0.1.1.nsp';files=pfs(nsp.read_bytes());assert len(files)==3 and all(n.endswith('.nca') for n in files)
verified_sections=False
if a.keyset:
 with tempfile.TemporaryDirectory(prefix='sgverify-') as tmp:
  t=pathlib.Path(tmp);got_program=got_control=got_meta=False
  for i,(name,data) in enumerate(files.items()):
   path=t/name;path.write_bytes(data);out=t/str(i)
   for sub in ['exefs','romfs','section0']:(out/sub).mkdir(parents=True,exist_ok=True)
   r=subprocess.run([a.hactool,'-k',a.keyset,'--disablekeywarns','--suppresskeys','-x',str(path),'--exefsdir='+str(out/'exefs'),'--romfsdir='+str(out/'romfs'),'--section0dir='+str(out/'section0')],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
   assert r.returncode==0,'NCA extraction failed; tool output kept private'
   if (out/'exefs/main').exists():
    assert (out/'exefs/main').read_bytes()==(base/'package/exefs/main').read_bytes()
    patched=(out/'exefs/main.npdm').read_bytes();j=patched.find(b'ACI0')
    assert struct.unpack_from('<Q',patched,j+16)[0]==0x0100F53447550000;got_program=True
    assert (out/'romfs/STATUS.txt').read_bytes()==(base.parent/'STATUS.txt').read_bytes()
   if (out/'romfs/control.nacp').exists():
    cn=(out/'romfs/control.nacp').read_bytes();assert cn.startswith('孤侠魅影\0'.encode('utf-8'))
    for lang in range(16):assert cn[lang*0x300:].startswith('孤侠魅影\0'.encode('utf-8'))
    assert (out/'romfs/icon_AmericanEnglish.dat').read_bytes()==(base.parent/'assets/icon.jpg').read_bytes()
    got_control=True
   if list((out/'section0').glob('*.cnmt')):got_meta=True
  assert got_program and got_control and got_meta;verified_sections=True
manifest={'version':'0.1.1','display_title':'孤侠魅影','title_id':'0100F53447550000','architecture':'AArch64','gameplay_available':False,'hardware_tested':False,'nsp_sections_extracted_and_matched':verified_sections,'files':{}}
for name in ['Shadow-Guardian-Frontend-0.1.1.nsp','Shadow-Guardian-Frontend-0.1.1.nro']:
 data=(base/name).read_bytes();manifest['files'][name]={'size':len(data),'sha256':hashlib.sha256(data).hexdigest()}
(base/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest,indent=2))
