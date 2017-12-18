# qt-ditherer

## WhatIsIt
This qt-based app work with 32-bit RGB pics, makes'em 8-bit Indexed, downscales (hardcoded, but easy-rewriting) their color table and does dithering (Floyd-Steinberg algorythm).
Also, its functionality allows to do some more fun stuff, listed in updates

### Before
![inputScreen](inputScreen.jpg)

### After
![outputScreen](outputScreen.jpg)

## Update №1
Added upscaling image by bicubic interpolating

## Update №2
Added diff noises, blending, binarization, some simple matrix filters, 2-d Fourier transform
