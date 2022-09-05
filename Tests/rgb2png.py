from PIL import Image
import struct
from pathlib import Path

def BGR555_to_RGB(val):
    return ((val & 0x001F) * 0x08, ((val & 0x03E0) >> 5) * 0x08, ((val & 0x7C00) >> 10) * 0x08)

for x in Path("frames").glob("*.bin"):
    foo = open(x.as_posix(), "rb")
    img = Image.new( 'RGB', (256, 192), "black")
    pixels = img.load()

    try:
        for i in range(img.size[1]):
            for j in range(img.size[0]):
                pixels[j,i] = BGR555_to_RGB(struct.unpack("<H", foo.read(2))[0]) 
        img.save(x.as_posix() + ".png")
    except:
        img.save(x.as_posix() + ".png")
