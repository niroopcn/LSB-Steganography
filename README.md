# LSB-Steganography

Usage: Encoding: ./a.out -e <.bmp_file> <.txt_file> [output file]
Usage: Decoding: ./a.out -d <.bmp_file> [output file]

Approach: 
Digital image steganography is used to hide confidential data within an image. Least significant bit (LSB) is a well-known steganography approach.
LSB stegenography method is used to encode a message such a way that the quality of the image doesn't change much.

We take 1 byte of image data and encode/modify only the LSB bit.
Accordingly, to encode 1 byte of text data, we would need 8 bytes of image data.

We use BMP files because typically, these image files are uncompressed, and therefore large and lossless.
Their advantage is their simple structure and wide acceptance in Windows programs.

While encoding, we would need to skip the first 54 bytes of header data in .bmp file so that the metadata wouldn't change and modify image properties accidentaly.
Here we start encoding from the 55th byte.
