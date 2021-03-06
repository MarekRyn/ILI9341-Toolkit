# ILI9341-Toolkit
Drivers and tools for developing applications on STM32 platform with ILI9341 TFT.


	Drivers and tools are at early stage of development. Any improvements from community are welcome.

<b>Content:</b>

* STM32F1	- drivers for ILI9341 optimized for parralel 16-bit communication
* TFT Tools	- python scripts for image and font compression


<b>Drivers:</b>

- optimized for 16bit color (565 format) and 16-bit data bus
- included basic graphic functions and decompressors for images and fonts
- text displayed with antialiasing


<b>Font Compressor:</b>

- importing any .ttf font into STM32 c project
- as default all characters from ASCII 32 to 127 are encoded
- it is possible to define subset of characters that schould be encoded and
  consequently saving uC's memory
- compressor saves antialiasing information for smooth display on TFT screen


<u>EXAMPLE 1:</u> Compressing font verdana with size 12 and exporting to .c file

	fcmpr = FontCompressor("verdana.ttf", 12)
	fcmpr.export2c("verdana.c", "verdana12")


<u>EXAMPLE 2:</u> Compressing font verdana with size 14 - only selected characters

	fcmpr = FontCompressor("verdana.ttf", 12, "1234567890ABCDEF")
	fcmpr.export2c("verdana.c", "verdana12")



<b>Image Compressor:</b>

- designed for images with max 255 colors (mainly GUI graphics, not photos)
- recommended resolution 240x320
- fast image decompression on uC
- for animations it is possible to compare two images and encode only
  differences between them



<u>EXAMPLE 1:</u> Compressing image00.png (240x320) and exporting to .c file

	icmpr = ImgCompressor("image00.png")
	icmpr.export2c("image00.c", "image00")

<u>EXAMPLE 2:</u> Compressing image01.png (240x320) and exporting to .c file.
	   In addition image is compared to image00.png and only differences are encoded.		   
	   Latter to properly display this image it is require to display image00 first, and
	   than image01 as only parts of picture that changed are updated.

		   
	icmpr = ImgCompressor("image01.png", "image00.png")
	icmpr.export2c("image01.c", "image01")


