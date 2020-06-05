# Created on: Jun 01, 2020
# Author: Marek Ryn


# Imports
from PIL import Image
from collections import OrderedDict


# Classes
class ImgCompressor:

    def __init__(self, imgfile, cmpfile=None):
        # Loading image data
        try:
            self.image = Image.open(imgfile)
        except:
            print('ERROR 1: Unable to open image file, or unknown format.')
            return
        self.width = self.image.width
        self.height = self.image.height

        if (self.width > 4186) or (self.height > 4186):
            print('ERROR 2: File exceeds maximum resolution (4186x4186).')
            return

        if cmpfile:
            try:
                self.cmpimage = Image.open(cmpfile)
            except:
                print('ERROR 1: Unable to open image file, or unknown format.')
                return

            if (self.width != self.cmpimage.width) or (self.height != self.cmpimage.height):
                print('ERROR 3: Main file and comparative file have different dimensions.')
                return

        self.cimage1 = [[-1]*self.width for _ in range(self.height)]
        self.cimage2 = [[-1]*self.width for _ in range(self.height)]
        self.blocks = []
        self.outbin = bytearray()

        tpalette = {}

        # Generating color palette
        print("Generating color palette...")
        for y in range(self.height):
            for x in range(self.width):
                c = self.rgb2rgb565(*self.image.getpixel((x, y)))

                if c in tpalette:
                    tpalette[c] += 1
                else:
                    tpalette[c] = 1

        if len(tpalette) > 255:
            print('ERROR 4: Image contains more than 255 colors.')
            return

        # Sorting palette by frequency
        print("Sorting palette...")
        self.cpalette = OrderedDict(sorted(tpalette.items(), key=lambda t: t[1], reverse=True))

        # Generating color map for given image
        print("Generating color map...")

        for cc in self.cpalette.keys():
            tpalette[cc] = list(self.cpalette.keys()).index(cc)

        for y in range(self.height):
            for x in range(self.width):
                c = self.rgb2rgb565(*self.image.getpixel((x, y)))
                if cmpfile:
                    c0 = self.rgb2rgb565(*self.cmpimage.getpixel((x,y)))
                    if c == c0:
                        self.cimage2[y][x] = -2
                self.cimage1[y][x] = tpalette[c]

        # Compressing - Stage 1: building block list
        print("Compressing - Stage 1: building block list...")
        for c in range(len(self.cpalette)):
            for y in range(self.height):
                for x in range(self.width):

                    # Checking if this part of picture should be processed
                    if self.cimage1[y][x] != c:
                        continue
                    if self.cimage2[y][x] >= c:
                        continue
                    if self.cimage2[y][x] == -2:
                        continue

                    w = 0
                    h = 0
                    flag1 = True
                    flag2 = True

                    # Looking for largest block
                    while flag1 or flag2:

                        # Stretching block horizontally
                        if flag1:
                            for j in range(y, y+h):
                                if self.cimage1[j][x + w] < c:
                                    flag1 = False
                                if self.cimage2[j][x + w] == c:
                                    flag1 = False
                                if self.cimage2[j][x + w] == -2:
                                    flag1 = False
                            if flag1:
                                w += 1
                                if x + w == self.width:
                                    flag1 = False

                        # Stretching block vertically
                        if flag2:
                            for i in range(x, x+w):
                                if self.cimage1[y + h][i] < c:
                                    flag2 = False
                                if self.cimage2[y + h][i] == c:
                                    flag2 = False
                                if self.cimage2[y + h][i] == -2:
                                    flag2 = False
                            if flag2:
                                h += 1
                                if y + h == self.height:
                                    flag2 = False

                    # Marking processed parts of image
                    for j in range(h):
                        for i in range(w):
                            if self.cimage1[y+j][x+i] == c:
                                self.cimage2[y+j][x+i] = c

                    # Adding block to list
                    self.blocks.append([c, x, y, w, h])

        # Compressing - Stage 2: Encoding binary output

        print("Compressing - Stage 2: Encoding...")
        # Saving color palette
        self.outbin.append(len(self.cpalette))
        for i in range(len(self.cpalette)):
            c = list(self.cpalette.items())[i][0]
            self.outbin.append(c & 0x00FF)
            self.outbin.append(c >> 8)

        # Encoding and saving blocks
        c = 0
        sx = 0
        sy = 0

        for block in self.blocks:
            # print(block)
            if block[0] > c:        # Next color
                for i in range(block[0] - c):
                    self.outbin.append(0x00)
                c = block[0]
                sx = 0
                sy = 0

            # Updating image active section /MAX IMAGE RESOLUTION: 4186px x 4186px !!!!/
            if (block[1] < sx * 256) or (block[1] > sx * 256 + 255) or (block[2] < sy * 256) or (block[2] > sy * 256 + 255):
                t = ((block[1] // 256) << 3) | (block[2] // 256)
                self.outbin.append(t)

            if block[4] == 1:       # H-Line

                b = block[3]
                t = 0x40
                if b < 63:
                    t += b
                    self.outbin.append(t)
                else:
                    t += 63
                    self.outbin.append(t)
                    for i in range((b - 63) // 256):
                        self.outbin.append(0xFF)
                    self.outbin.append((b-63) % 256)

            if (block[3] == 1) and (block[4] > 1):      # V-Line
                b = block[4]
                t = 0x80
                if b < 63:
                    t += b
                    self.outbin.append(t)
                else:
                    t += 63
                    self.outbin.append(t)
                    for i in range((b - 63) // 256):
                        self.outbin.append(0xFF)
                    self.outbin.append((b-63) % 256)

            if (block[3] > 1) and (block[4] > 1):        # Block
                t = 0xC0
                if block[3] > 255:
                    t += 0x02
                if block[4] > 255:
                    t += 0x01
                self.outbin.append(t)
                self.outbin.append(block[3] & 0x00FF)
                if block[3] > 255:
                    self.outbin.append(block[3] >> 8)
                self.outbin.append(block[4] & 0x00FF)
                if block[4] > 255:
                    self.outbin.append(block[4] >> 8)

            # Writing Block X,Y position for H-Line, V-Line, Block
            self.outbin.append(block[1] & 0xFF)
            self.outbin.append(block[2] & 0xFF)

        print()
        print("Total Before: ", self.width * self.height, "bytes (for 8-bit color)")
        print("Total After:  ", len(self.outbin), "bytes")
        print("Compression Ratio: ", round(10000 * (1 - len(self.outbin) / (self.width * self.height))) / 100, "%")

    def export2bin(self, fname):
        # Saving encoded binary file
        with open(fname, "wb") as f:
            f.write(self.outbin)

    def export2c(self, fname, varname):
        # Saving as c/cpp text file
        with open(fname, "a+") as f:
            f.write("//------------------------------------------------------------------------------\n")
            f.write("// File generated by TFT Tools - Image Compressor \n")
            f.write("// Written by Marek Ryn \n")
            f.write("//------------------------------------------------------------------------------\n")
            f.write("\n")
            f.write("const uint8_t "+varname+" ["+str(len(self.outbin))+"] = {")
            for i in range(len(self.outbin)):
                if i % 16 == 0:
                    f.write("\n")
                f.write("0x{:02x}".format(self.outbin[i]))
                if i < len(self.outbin) - 1:
                    f.write(", ")
            f.write("};\n\n")

    @staticmethod
    def rgb2rgb565(r,  g,  b, x=0):
        r = (r & 0xF8) << 8
        g = (g & 0xFC) << 3
        b = (b >> 3)
        return r | g | b

