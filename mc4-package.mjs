import { readFileSync, writeFileSync } from 'fs';
import { createCipheriv } from 'crypto';
import { buildNsp } from '/home/user/nsp-tools/hb/node_modules/@tootallnate/hacbrewpack/dist/index.js';

const rd = (p) => new Blob([readFileSync(p)]);
const inDir = '/home/user/nsp-tools/in';

/* ---- Standard AES-128-XTS (IEEE P1619 / Nintendo) ----
 * The bundled @tootallnate/aes-xts misses the GF(2^128) x^j per-block
 * tweak step, so its NCA headers cannot be decrypted by DBI/Tinfoil.
 * Provide a correct implementation here instead.
 */
function xtsMul2(x) {
  // x: Uint8Array(16), multiply by x in GF(2^128) (big-endian poly, x^128+x^7+x^2+x+1)
  const out = new Uint8Array(16);
  let carry = 0;
  for (let i = 15; i >= 0; i--) {
    const b = x[i];
    out[i] = ((b << 1) | carry) & 0xff;
    carry = (b & 0x80) ? 1 : 0;
  }
  if (carry) out[15] ^= 0x87;
  return out;
}
function xtsEncrypt(key, data, sectorSize, startSector) {
  // key: 32 bytes (key1 + key2), data: plaintext, sectorSize: 0x200, startSector: tweak base
  const k1 = key.subarray(0, 16), k2 = key.subarray(16, 32);
  const e2 = createCipheriv('aes-128-ecb', k2, null);
  const e1 = createCipheriv('aes-128-ecb', k1, null);
  const out = new Uint8Array(data.length);
  let sector = startSector;
  for (let pos = 0; pos < data.length; pos += sectorSize) {
    const chunk = data.subarray(pos, pos + sectorSize);
    const nBlocks = Math.ceil(chunk.length / 16);
    // T = E(K2, tweak), tweak = big-endian 128-bit sector number
    const tweakBytes = Buffer.alloc(16);
    tweakBytes.writeBigUInt64BE(BigInt(sector), 8);
    let T = new Uint8Array(e2.update(tweakBytes).subarray(0, 16));
    const block = Buffer.alloc(16);
    for (let j = 0; j < nBlocks; j++) {
      const src = chunk.subarray(j * 16, j * 16 + 16);
      for (let b = 0; b < 16; b++) block[b] = src[b] ^ T[b];
      const enc = new Uint8Array(e1.update(block));
      for (let b = 0; b < 16; b++) out[pos + j * 16 + b] = enc[b] ^ T[b];
      T = xtsMul2(T);
    }
    sector++;
  }
  return out;
}

const exefs = new Map([
  ['main', rd(process.argv[2])],
  ['main.npdm', rd(process.argv[3])],
]);
const control = new Map([
  ['control.nacp', rd('/home/user/Doubao/chats/38447866592439042/mc4-switch/control.nacp')],
  ['icon_AmericanEnglish.dat', rd(inDir + '/icon_AmericanEnglish.dat')],
]);
const logo = new Map([
  ['NintendoLogo.png', rd(inDir + '/NintendoLogo.png')],
  ['StartupMovie.gif', rd(inDir + '/StartupMovie.gif')],
]);
const keys = readFileSync('/home/user/Doubao/chats/38446514291538690/keys/prod.keys', 'utf8');
const out = await buildNsp({
  keys, exefs, control, romfs: {}, logo,
  titleId: '0100F53447550002', plaintext: true,
  titleName: '现代战争4', titlePublisher: 'Independent Homebrew',
});
const buf = Buffer.from(await out.nsp.arrayBuffer());
writeFileSync(process.argv[4], buf);
console.log('NSP bytes:', buf.length, 'titleId:', out.titleId);
