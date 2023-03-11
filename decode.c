#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

// func. to start the process of opening files and decoding
Status do_decoding(DecodeInfo *decInfo)
{
	uint decodingerrorflag = 0;
	printf("[INFO] Opening %s file\n", decInfo->stego_image_fname);
	if (!open_stego_decode(decInfo))
	{
		printf("[INFO] ## Decoding Procedure Started ##\n");
		printf("[INFO] Decoding Magic String Signature\n");
		if (!decode_magic_string(MAGIC_STRING, decInfo))
		{
			printf("[INFO] Decoding Output File Extension size\n");
			if (!decode_output_file_extn_size(decInfo))
			{
				printf("[INFO] Decoding Output File Extension\n");
				decode_output_file_extn(decInfo->extn_output_file, decInfo);

				printf("[INFO] Original output file had %s extension\n", decInfo->extn_output_file);
				printf("[INFO] So, concatenating %s to %s by default\n", decInfo->extn_output_file, decInfo->output_fname);
				strcat(decInfo->output_fname, decInfo->extn_output_file);
			}
			else
			{
				printf("[INFO] Extension missing in decoded information\n");
				// printf("[INFO] So, concatenating %s to %s by default\n", decInfo->extn_output_file, decInfo->output_fname);
				strcat(decInfo->output_fname, decInfo->extn_output_file);
			}

			printf("[INFO] Final output filename is %s\n", decInfo->output_fname);
			printf("[INFO] Decoding filesize for %s\n", decInfo->output_fname);
			if (!decode_output_file_size(decInfo))
			{
				if (!open_output_decode(decInfo))
				{
					printf("[INFO] Copying super-secret data to %s file\n", decInfo->output_fname);
					if (!decode_secret_data_from_image(decInfo))
					{
						printf("[INFO] Done\n");

						// close files after finishing execution
						fclose(decInfo->fptr_output);
						fclose(decInfo->fptr_stego_image);

						return e_success;
					}
				}
			}
		}
		else
		{
			fclose(decInfo->fptr_stego_image);
			decodingerrorflag = 1;
		}
	}
	else
		decodingerrorflag = 1;

	if (!decodingerrorflag)
		return e_success;
	else
	{
		fprintf(stderr, "[ERROR] Failed\n");
		return e_failure;
	}
}

// Function to read and validate CLA
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
	// Check extension for argv[2]
	if (strstr(argv[2], ".bmp") != NULL)
	{
		decInfo->stego_image_fname = argv[2];

		// check if output filename is passed
		if (argv[3] == NULL)
		{
			strcpy(decInfo->output_fname, "decoded_text");
			printf("[INFO] Output File not mentioned. Creating %s file as default\n", decInfo->output_fname);
		}
		else
		{
			char *ptr = strstr(argv[3], ".");
			if (ptr == NULL)
				strcpy(decInfo->output_fname, argv[3]);
			else
			{
				*ptr = '\0';
				strcpy(decInfo->output_fname, argv[3]);
			}
		}

		return e_success;
	}
	else
	{
		fprintf(stderr, "[ERROR] Use .bmp file\n");
		return e_failure;
	}
}

// Function to open stego input image file
Status open_stego_decode(DecodeInfo *decInfo)
{
	// Stego Image file
	decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

	// Do Error handling
	if (decInfo->fptr_stego_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "[ERROR] File %s does not exist\n", decInfo->stego_image_fname);
		return e_failure;
	}

	// No failure return e_success
	printf("[INFO] Done\n");
	return e_success;
}

// Function to decode Magic string from i/p file and check if it matches with original
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
	// Skipping header of 54 bytes
	fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

	int len = strlen(magic_string);
	for (int i = 0; i < len; i++)
	{
		if (extract_string_from_image(decInfo->fptr_stego_image) == magic_string[i])
			continue;
		else
		{
			fprintf(stderr, "[ERROR] Magic string doesn't match, This image might not be encoded.\n");
			return e_failure;
		}
	}
	printf("[INFO] Done, Magic string matched\n");
	return e_success;
}

// Function to decode secret file's extn from i/p image
Status decode_output_file_extn(char *extn_output_file, DecodeInfo *decInfo)
{
	for (int i = 0; i < decInfo->ext_size; i++)
	{
		extn_output_file[i] = extract_string_from_image(decInfo->fptr_stego_image);
	}

	printf("[INFO] Done\n");
	return e_success;
}

// Function to extract charactes from i/p image
char extract_string_from_image(FILE *fptr_stego_image)
{
	char buffer[DEC_MAX_IMAGE_BUF_SIZE]; // 8 bytes

	fread(buffer, 8, 1, fptr_stego_image);

	return extract_lsb_from_bytes(buffer);
}

// Function to extract bits from LSB positions of bufffer
char extract_lsb_from_bytes(char *buffer)
{
	char temp_buffer = 0;

	for (int i = 7; i >= 0; i--)
		temp_buffer = (temp_buffer << 1) | (buffer[i] & 1);

	return temp_buffer;
}

// Function to extract secret file extn size(integer value)
Status decode_output_file_extn_size(DecodeInfo *decInfo)
{
	decInfo->ext_size = extract_int_from_image(decInfo->fptr_stego_image);
	printf("[INFO] Done\n");

	if (!decInfo->ext_size)
		return e_failure;
	else
		return e_success;
}

// Function to extract integer value from i/p image using a buffer of 32 bytes
uint extract_int_from_image(FILE *fptr_stego_image)
{
	char buffer[DEC_MAX_FILE_SIZE_BUFFER]; // 32 bytes
	int temp_buffer = 0;
	fread(buffer, 32, 1, fptr_stego_image);
	if (!extract_lsb_from_int(&temp_buffer, buffer))
		return temp_buffer;
}

// Function to extract LSB bits from i/p image and store in temp variable
Status extract_lsb_from_int(int *temp_buffer, char *buffer)
{
	for (int i = 31; i >= 0; i--)
		*temp_buffer = ((*temp_buffer) << 1) | (buffer[i] & 1);
	return e_success;
}

// Function to extract integer value(no. of characters) from i/p image
Status decode_output_file_size(DecodeInfo *decInfo)
{
	decInfo->size_output_file = extract_int_from_image(decInfo->fptr_stego_image);
	printf("[INFO] Done - This secret file has %d characters\n", decInfo->size_output_file);
	return e_success;
}

// Function to open output file
Status open_output_decode(DecodeInfo *decInfo)
{
	// Output file
	printf("[INFO] Opening %s\n", decInfo->output_fname);
	decInfo->fptr_output = fopen(decInfo->output_fname, "w");

	// Do Error handling
	if (decInfo->fptr_output == NULL)
	{
		perror("fopen");
		fprintf(stderr, "[ERROR] Unable to open %s file\n", decInfo->output_fname);
		return e_failure;
	}

	// No failure return e_success
	printf("[INFO] Done\n");
	return e_success;
}

// Function to extract secret data from i/p stego image and store inside output file
Status decode_secret_data_from_image(DecodeInfo *decInfo)
{
	for (int i = 0; i < decInfo->size_output_file; i++)
	{
		fputc(extract_string_from_image(decInfo->fptr_stego_image), decInfo->fptr_output);
	}
	return e_success;
}
