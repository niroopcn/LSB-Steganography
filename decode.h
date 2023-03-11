#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include<string.h>
#include "types.h"
#include "common.h"

//defining MACROS
#define DEC_MAX_SECRET_BUF_SIZE 1
#define DEC_MAX_IMAGE_BUF_SIZE (DEC_MAX_SECRET_BUF_SIZE * 8)
#define DEC_MAX_FILE_SUFFIX 5
#define DEC_MAX_FILE_SIZE_BUFFER (DEC_MAX_IMAGE_BUF_SIZE * 4)

/*
 * Structure to store information required for
 * decoding source image to output file
 * Info about size and extn and other intermediate data is
 * also stored
 */
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* output File Info */
    char output_fname[30];
    FILE *fptr_output;
    int ext_size;
    char extn_output_file[DEC_MAX_FILE_SUFFIX];
    int size_output_file;

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Get File pointers for i/p file and open stego image file*/
Status open_stego_decode(DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Decode the Magic String and check if it matches*/
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode function to decode strings from image */
char extract_string_from_image(FILE *fptr_stego_image);

/* Decode a byte from LSB of image data array */
char extract_lsb_from_bytes(char *buffer);

/* Decode secret file extenstion size*/
Status decode_output_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file size and extn size integer data form image*/
uint extract_int_from_image(FILE *fptr_stego_image);

/* Decode integer value from LSB positons of image data array*/
Status extract_lsb_from_int(int *temp_buffer, char *buffer);

/* Decode extn string from i/p image */
Status decode_output_file_extn(char *extn_output_file, DecodeInfo *decInfo);

/* Decode integer value of no. of characters in secret file*/
Status decode_output_file_size(DecodeInfo *decInfo);

/* Open output file to store secret data into*/
Status open_output_decode(DecodeInfo *decInfo);

/* Decode the secret message string from i/p image*/
Status decode_secret_data_from_image(DecodeInfo *decInfo);

#endif
