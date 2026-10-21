#!/usr/bin/env python3
# Rasterize the redistributable DejaVu font; requires Pillow only when regenerating.
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
base=Path(__file__).resolve().parents[1]
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',20)
rows=[]
for c in range(32,127):
 im=Image.new('1',(13,26)); ImageDraw.Draw(im).text((0,0),chr(c),font=font,fill=1)
 rows.extend(sum((1<<x) for x in range(13) if im.getpixel((x,y))) for y in range(26))
(base/'source/font.h').write_text('/* DejaVu Sans Mono raster; see assets/FONT-LICENSE.txt. */\nstatic const unsigned short glyphs[95][26] = {\n'+''.join('{'+','.join(map(str,rows[i:i+26]))+'},\n' for i in range(0,len(rows),26))+'};\n')
# Icon is maintained from the user-provided artwork; font regeneration must not overwrite it.
