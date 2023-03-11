#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

// Function for initiating the process of opening files, checking capacity and encoding data from src image to stego outpue image
Status do_encoding(EncodeInfo *encInfo)
{
	uint encodingerrorflag = 0;
	printf("[INFO] Opening required files\n");
	if (!open_files(encInfo))
	{
		printf("[INFO] Checking Image Metadata\n");
		if (!check_capacity(encInfo))
		{
			printf("[INFO] ## Encoding Procedure Started ##\n");
			printf("[INFO] Copying Image Header\n");
			if (!copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image))
			{
				printf("[INFO] Encoding Magic String Signature\n");
				if (!encode_magic_string(MAGIC_STRING, encInfo))
				{
					printf("[INFO] Encoding %s File Extension size\n", encInfo->secret_fname);
					if (!encode_secret_file_extn_size(encInfo))
					{
						printf("[INFO] Encoding %s File Extension\n", encInfo->secret_fname);
						if (!encode_secret_file_extn(encInfo->extn_secret_file, encInfo))
						{
							printf("[INFO] Encoding %s File Size\n", encInfo->secret_fname);
							if (!encode_secret_file_size(encInfo->size_secret_file, encInfo))
							{
								printf("[INFO] Encoding %s File Data\n", encInfo->secret_fname);
								if (!encode_secret_file_data(encInfo))
								{
									printf("[INFO] Copying Left Over Data\n");
									if (!copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image))
									{
										printf("[INFO] Done\n");
										// close files after finishing execution
										fclose(encInfo->fptr_src_image);
										fclose(encInfo->fptr_secret);
										fclose(encInfo->fptr_stego_image);
									}
								}
							}
						}
					}
				}
			}
		}
		else
			encodingerrorflag = 1;
	}
	else
		encodingerrorflag = 1;

	if (!encodingerrorflag)
		return e_success;
	else
		return e_failure;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
	// Src Image file
	printf("[INFO] Opening %s\n", encInfo->src_image_fname);
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");

	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "[ERROR] File %s does not exist\n", encInfo->src_image_fname);

		return e_failure;
	}

	// Secret file
	printf("[INFO] Opening %s\n", encInfo->secret_fname);
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "[ERROR] File %s does not exist\n", encInfo->secret_fname);
		fclose(encInfo->fptr_src_image);
		return e_failure;
	}

	// check secret file size
	fseek(encInfo->fptr_secret, 0, SEEK_END);
	encInfo->size_secret_file = ftell(encInfo->fptr_secret);
	rewind(encInfo->fptr_secret);

	// Stego Image file
	printf("[INFO] Opening %s\n", encInfo->stego_image_fname);
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
	// Do Error handling
	if (encInfo->fptr_stego_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

		return e_failure;
	}

	// No failure return e_success
	printf("[INFO] Done\n");
	return e_success;
}

// Read and validate CLA for encoding process
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
	// Check extension for argv[2]
	if (strstr(argv[2], ".bmp") != NULL)
	{
		encInfo->src_image_fname = argv[2];

		// store name of secret file
		encInfo->secret_fname = argv[3];
		char *extfound = strstr(encInfo->secret_fname, ".");

		if (extfound != NULL)
		{
			uint secretlen = strlen(extfound) - 1;
			if (secretlen < 4)
				strcpy(encInfo->extn_secret_file, extfound);
			else
			{
				fprintf(stderr, "[ERROR] Extension of secret file exceeded 4 character limit\n");
				return e_failure;
			}
		}
		else
			bzero(encInfo->extn_secret_file, 4); // if no extn is present, make string elements ZERO

		// Check if output filename is passed
		if (argv[4] == NULL) // If output filename is not passed
		{
			encInfo->stego_image_fname = "stego_image_default.bmp";
			printf("[INFO] Output File not mentioned. Creating %s as default\n", encInfo->stego_image_fname);
		}
		else // If output filename is passed
		{
			char *ptr1 = strstr(argv[4], ".bmp");
			if (ptr1 == NULL)
			{
				printf("[INFO] Extension of output file does not match .bmp, so concatenating .bmp by default\n");
				strcat(argv[4], ".bmp");
				encInfo->stego_image_fname = argv[4];
			}
			else
				encInfo->stego_image_fname = argv[4];
		}

		return e_success;
	}
	else // If source file is not passed with extn, show error
	{
		fprintf(stderr, "[ERROR] Use .bmp file for source image\n");
		return e_failure;
	}
}

// Function to check if source image has enough bytes to store secret data
Status check_capacity(EncodeInfo *encInfo)
{
	// store image capacity a.k.a resolution * 3
	encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
	printf("[INFO] Checking for image capacity of %s: %u Bytes\n", encInfo->src_image_fname, encInfo->image_capacity);

	// store bits per pixel in structure variable
	fseek(encInfo->fptr_src_image, 28, SEEK_SET);
	fread(&(encInfo->bits_per_pixel), sizeof(short int), 1, encInfo->fptr_src_image);
	rewind(encInfo->fptr_src_image);
	printf("[INFO] This image is of %u bit depth (bits per pixel)\n", encInfo->bits_per_pixel);

	// size of .bmp file
	uint sizeofbmp = get_file_size(encInfo->fptr_src_image);
	printf("[INFO] Checking for %s size: %u Bytes\n", encInfo->src_image_fname, sizeofbmp);
	rewind(encInfo->fptr_src_image);
	uint ext_len = strlen(encInfo->extn_secret_file);

	uint sizeoftxt = get_file_size(encInfo->fptr_secret);
	printf("[INFO] Checking for %s size: %u Bytes\n", encInfo->secret_fname, sizeoftxt);
	rewind(encInfo->fptr_secret);
	if (!sizeoftxt)
	{
		fprintf(stderr, "[ERROR] File Empty\n");
		return e_failure;
	}
	else
		printf("[INFO] Done. Not Empty\n");

	printf("[INFO] Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
	if (sizeofbmp > (uint)54 + (8 + strlen(MAGIC_STRING) + ext_len + sizeoftxt) * 8)
	{
		printf("[INFO] Done. Found OK\n");
		return e_success;
	}
	else
	{
		fprintf(stderr, "[ERROR] Cannot encode %s in %s, File capacity exceeded\n", encInfo->secret_fname, encInfo->src_image_fname);
		return e_failure;
	}
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height;
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);

	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image);
	printf("[INFO] Image Width is %up\n", width);

	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image);
	printf("[INFO] Image Height is %up\n", height);

	rewind(fptr_image);

	// Return image capacity
	return width * height * 3;
}

// Function to calculate total file size
uint get_file_size(FILE *fptr)
{
	fseek(fptr, 0, SEEK_END);
	return ftell(fptr);
}

// Function to copy 54 bytes of header information from source to desttination file
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	rewind(fptr_src_image);
	char buffer[54];
	fread(buffer, 54, 1, fptr_src_image);
	fwrite(buffer, 54, 1, fptr_dest_image);
	printf("[INFO] Done\n");
	return e_success;
}

// Function to encode magic string to stego output image
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
	if (!encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image))
	{
		printf("[INFO] Done\n");
		return e_success;
	}
}

// Function to encode integer value of secret file extn size to stego output image
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
	int extsize = strlen(encInfo->extn_secret_file);
	if (!extsize)
		printf("[INFO] No extension provided. Encoding (int) 0\n");
	else
		printf("[INFO] Extension length is (int) %d, Encoding...\n", extsize);
	if (!encode_size(extsize, encInfo->fptr_src_image, encInfo->fptr_stego_image))
	{
		printf("[INFO] Done\n");
		return e_success;
	}
}

/*Function to encode strings to stego output image,
  this function reads 8 bytes of data from source image and stores inside buffer,
  then sends this buffer to be modified/encoded and writes the buffer to stego output image*/
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	char buffer[MAX_IMAGE_BUF_SIZE]; // 8 bytes
	for (uint i = 0; i < size; i++)
	{
		fread(buffer, 8, 1, fptr_src_image);
		if (!encode_byte_to_lsb(data[i], buffer))
			fwrite(buffer, 8, 1, fptr_stego_image);
	}
	return e_success;
}

// Function to modify LSB positions in each byte of buffer
Status encode_byte_to_lsb(char data, char *image_buffer)
{
	for (int i = 0; i < 8; i++)
	{
		if ((data >> i) & 1)
			image_buffer[i] |= 1;
		else
			image_buffer[i] &= ~1;
	}
	return e_success;
}

// Function to encode extn string to stego output image
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
	uint len = strlen(file_extn);
	if (!len)
		printf("[INFO] No extension provided so skipping encoding extension string\n");
	else if (!encode_data_to_image(file_extn, len, encInfo->fptr_src_image, encInfo->fptr_stego_image))
		printf("[INFO] Extension string is %s, Encoding...\n", file_extn);

	printf("[INFO] Done\n");
	return e_success;
}

// Function to encode file size/no. of characters as an integer value to stego output image
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
	if (!encode_size((int)file_size, encInfo->fptr_src_image, encInfo->fptr_stego_image))
	{
		printf("[INFO] Done\n");
		return e_success;
	}
}

/*Function to encode an integer value to stego output image
  this function reads 32 bytes of data from src image which is stored in buffer,
  and then is modified by changing LSB positions, the modified buffer is finally copied to stego output image*/
Status encode_size(int file_size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	char buffer[MAX_FILE_SIZE_BUFFER]; // 32 bytes
	fread(buffer, 32, 1, fptr_src_image);

	for (int i = 0; i < 32; i++)
	{
		if ((file_size >> i) & 1)
			buffer[i] = buffer[i] | 1;
		else
			buffer[i] = buffer[i] & ~1;
	}
	fwrite(buffer, 32, 1, fptr_stego_image);
	return e_success;
}

// Function to encode data(characters from secret file) onto the stego output image file
Status encode_secret_file_data(EncodeInfo *encInfo)
{
	rewind(encInfo->fptr_secret);
	while ((encInfo->secret_data[0] = fgetc(encInfo->fptr_secret)))
	{
		if (feof(encInfo->fptr_secret))
			break;

		encode_data_to_image(encInfo->secret_data, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image);
	}

	printf("[INFO] Done\n");
	return e_success;
}

// Function to copy remaining image information(RGB data) from source image to stego output image
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
	int ch;
	while ((ch = fgetc(fptr_src)) != EOF)
		fputc(ch, fptr_dest);
	return e_success;
}
