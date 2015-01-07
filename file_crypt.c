/*
	File encryption and decryption for Elektromotus bootloader

	Author: Rytis Karpuška
			rytis@elektromotus.lt

*/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "aes.h"

struct params_t {
	int decryption_requested;
	char *input_filename;
	char *output_filename;
	char *key;
	char *iv;
};

void print_usage()
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "Encryption:\n");
	fprintf(stderr, "  ./file_crypt encrypt <input file> <output file>\n");
	fprintf(stderr, "Decryption:\n");
	fprintf(stderr, "  ./file_crypt decrypt <input_file> <output file>\n");
	return;
}


int parse_args(int argc, char *argv[], struct params_t *params)
{
	//Check argument count
	if(argc != 4){
		print_usage();
		return -EFAULT;
	}

	//enumerate command
	if(strcmp(argv[1], "encrypt") == 0){
		params->decryption_requested = 0;
	} else if(strcmp(argv[1], "decrypt") == 0){
		params->decryption_requested = 1;
	} else {
		print_usage();
		return -EFAULT;
	}

	//enumerate filenames and keys
	params->input_filename = argv[2];
	params->output_filename = argv[3];

	return 0;
}


int main(int argc, char *argv[])
{
	struct params_t params;

	//Parse arguments
	if(parse_args(argc, argv, &params))
		exit(0);

	//try to open input file
	FILE *in_f = fopen(params.input_filename, "r");
	if(in_f == NULL){
		fprintf(stderr, "Could not open input file\n");
		exit(-EPERM);
	}

	//Try to open output file
	FILE *out_f = fopen(params.output_filename, "w");
	if(out_f == NULL){
		fprintf(stderr, "Could not open output file\n");
		fclose(in_f);
		exit(-EPERM);
	}

	//Read Password from input
	char *key = getpass("Please enter key: ");

	//Ask for repetition on encryption
	if(!params.decryption_requested){
		char *key2 = getpass("Please repeate key: ");

		//check if passwords are the same
		if(strcmp(key, key2) != 0){
			fprintf(stderr, "Entered keys does not match\n");
			exit(-EPERM);
		}
	}

	//check length
	if(strlen(key) != 16){
		fprintf(stderr, "Key length must be 16 bytes in length\n");
		exit(-EFAULT);
	}

	//Read initialisation vector
	char *iv = getpass("Please enter initialisation vector: ");

	if(!params.decryption_requested){
		char *iv2 = getpass("Please repeate initialisation vector: ");

		if(strcmp(iv, iv2) != 0){
			fprintf(stderr, "Entered initialisation vectors does not match\n");
			exit(-EPERM);
		}
	}

	//check length
	if(strlen(iv) != 16){
		fprintf(stderr, "Initiation vector must be 16 bytes in length\n");
		exit(-EPERM);
	}

	//figure out file size
	fseek(in_f, 0, SEEK_END);
	int f_size = ftell(in_f);
	fseek(in_f, 0, SEEK_SET);

	//Calculate 16byte aligned size
	int aligned_f_size = (f_size % 16) != 0 ? f_size + 16 - f_size % 16 : f_size;

	//allocate memory buffer for input file
	uint8_t *in_data = malloc(aligned_f_size);
	uint8_t *out_data = malloc(aligned_f_size);


	//read entire file to memory
	fread(in_data, f_size, 1, in_f);
	memset(in_data + f_size, 0, aligned_f_size - f_size);

	//Do actual encryption or decryption
	if(params.decryption_requested){
		//Decrypt file
		fprintf(stderr, "Decrypting...\n");
		AES128_CBC_decrypt_buffer(out_data, in_data, aligned_f_size, key, iv);
	} else {
		fprintf(stderr, "Encrypting...\n");
		AES128_CBC_encrypt_buffer(out_data, in_data, aligned_f_size, key, iv);
	}
	fprintf(stderr, "Done.\n");

	//Write output
	fwrite(out_data, aligned_f_size, 1, out_f);

	//Free memory
	free(in_data);
	free(out_data);

	//close files
	fclose(in_f);
	fclose(out_f);

	return 0;
}





