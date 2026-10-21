import { readFileSync, writeFileSync } from 'fs';
import { buildNsp } from '/home/user/nsp-tools/hb/node_modules/@tootallnate/hacbrewpack/dist/index.js';
const rd = (p) => new Blob([readFileSync(p)]);
const inDir = '/home/user/nsp-tools/in';
const exefs = new Map([
  ['main', rd(process.argv[2])],        // mc4_game.nso
  ['main.npdm', rd(process.argv[3])],   // mc4_game.npdm
]);
const control = new Map([
  ['control.nacp', rd('/tmp/mc4_control.nacp')],
  ['icon_AmericanEnglish.dat', rd(inDir + '/icon_AmericanEnglish.dat')],
]);
const logo = new Map([
  ['NintendoLogo.png', rd(inDir + '/NintendoLogo.png')],
  ['StartupMovie.gif', rd(inDir + '/StartupMovie.gif')],
]);
const keys = '/home/user/.switch/prod.keys';
const out = await buildNsp({
  keys, keyGeneration: 1, exefs, control, romfs: {}, logo,
  titleId: '0100F53447550002', plaintext: true,
  titleName: '现代战争4', titlePublisher: 'Independent Homebrew',
});
const buf = Buffer.from(await out.nsp.arrayBuffer());
writeFileSync(process.argv[4], buf);
console.log('NSP bytes:', buf.length, 'titleId:', out.titleId);
