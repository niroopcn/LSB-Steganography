#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    int errorflag = 0;					//declare error flag to check CLA format erorr
    if (argc >= 3)
    {
	uint ret = check_operation_type(argv);		//check and store operation type in 'ret' variable

	switch (ret)					//match type with switch case
	{
	    case 0:					//case for encoding
		{
		    if (argc >= 4)
		    {
			EncodeInfo encInfo;							//declare encInfo variable for structure
			memset(&encInfo, 0, sizeof(encInfo));
			printf("[INFO] Encoding Selected\n[INFO] Validating Arguments...\n");
			if (!read_and_validate_encode_args(argv, &encInfo))			//func. call to read and validate CLA
			{
			    printf("[INFO] Validation Passed\n");

			    if (!do_encoding(&encInfo))						//func. call to encode data
				printf("[INFO] ## Encoding Done Successfully ##\n");
			    else
			    {
				fprintf(stderr, "[ERROR] ## Encoding Failed ##\n");
				errorflag = 1;
			    }
			}
			else
			    errorflag = 1;							//set error flag high for read and validate failure
		    }
		    else
		    {
			errorflag = 1;
			fprintf(stderr, "[ERROR] Please provide at least 4 arguments\n");
		    }
		}
		break;
	    case 1:					//case for decoding
		{
		    DecodeInfo decInfo;								//declare decInfo structure variable
			memset(&decInfo, 0, sizeof(decInfo));
		    printf("[INFO] Decoding Selected\n[INFO] Validating arguments\n");
		    if (!read_and_validate_decode_args(argv, &decInfo))				//func. call to read and validate CLA
		    {
			printf("[INFO] Validation Passed\n");
			if (!do_decoding(&decInfo))					//func. call to decode data
			{
			    printf("[INFO] ## Decoding Done Successfully ##\n");
			    printf("[INFO] Displaying secret message...\n");

			    char command[50] = {'c','a','t',' '};
			    strcat(command, decInfo.output_fname);	//concatenate output filename to command character array
			    system(command);				//passing command "cat o/p_filename" to terminal to display file contents 
			}
			else
			{
			    fprintf(stderr, "[ERROR] ## Decoding Failed ##\n");
			    errorflag = 1;
			}
		    }
		    else
			errorflag = 1;								//set errorflag high for read and validate failure
		}
		break;
	    default:						//case for unsupported format
		{
		    fprintf(stderr, "[ERROR] Unsupported Format.\n");
		    errorflag = 1;
		}
	}
    }
    else
	errorflag = 1;

    if (errorflag)						//if flag is high, print error message
	fprintf(stderr, "Usage: Encoding: ./a.out -e <.bmp_file> <.txt_file> [output file]\nUsage: Decoding: ./a.out -d <.bmp_file> [output file]\n");

    return 0;
}

OperationType check_operation_type(char *argv[])		//func. to check 1st argument from CLA
{
    if (!strcmp(argv[1], "-e"))					//if "-e" is passed, return e_encode(0)
	return e_encode;
    else if (!strcmp(argv[1], "-d"))				//if "-d" is passed, return e_decode(1)
	return e_decode;
    else							//else return e_unsupported(2)
	return e_unsupported;
}
