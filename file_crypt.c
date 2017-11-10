/*
	File encryption and decryption

	Author: Rytis Karpuška
			rytis@elektromotus.lt

*/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "aes.h"

#define KEY_LEN		16

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
	fprintf(stderr, "  ./file_crypt encrypt [-k key] [-i initiation vector] <input file> <output file>\n");
	fprintf(stderr, "Decryption:\n");
	fprintf(stderr, "  ./file_crypt decrypt [-k key] [-i initiation vector] <input_file> <output file>\n");
	return;
}


int parse_args(int argc, char *argv[], struct params_t *params)
{
	//Check argument count
	if(argc != 4 && argc != 8){
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
	params->input_filename = argv[argc - 2];
	params->output_filename = argv[argc - 1];
	params->key = NULL;
	params->iv  = NULL;

	//check for password and initiation vector on command line
	int c;
	while((c = getopt(argc, argv, "k:i:")) >= 0){
		switch(c){
		case 'k':
			params->key = malloc(strlen(optarg) + 1);
			strcpy(params->key, optarg);
			break;
		case 'i':
			params->iv = malloc(strlen(optarg) + 1);
			strcpy(params->iv, optarg);
			break;
		default:
			print_usage();
			return -EFAULT;
		}
	}

	return 0;
}


int main(int argc, char *argv[])
{
	struct params_t params;
	FILE *in_f = NULL, *out_f = NULL;
	uint8_t *in_data = NULL, *out_data = NULL;
	int status = 0;

	//Reset parameters
	memset(&params, 0, sizeof(params));

	//Parse arguments
	if((status = parse_args(argc, argv, &params)))
		goto CLEANUP;

	//try to open input file
	in_f = fopen(params.input_filename, "rb");
	if(in_f == NULL){
		fprintf(stderr, "Could not open input file\n");
		status = -EPERM;
		goto CLEANUP;
	}

	//Try to open output file
	out_f = fopen(params.output_filename, "wb");
	if(out_f == NULL){
		fprintf(stderr, "Could not open output file\n");
		status = -EPERM;
		goto CLEANUP;
	}

	//Read Password from input
	if(params.key == NULL){
		char *key = getpass("Please enter key: ");
		params.key = malloc(strlen(key) + 1);
		strcpy(params.key, key);
	}

	//check length
	if(strlen(params.key) != KEY_LEN){
		fprintf(stderr, "Key length must be %d bytes in length\n", KEY_LEN);
		status = -EINVAL;
		goto CLEANUP;
	}

	//Read initialisation vector
	if(params.iv == NULL){
		char *iv = getpass("Please enter initiation vector: ");
		params.iv = malloc(strlen(iv) + 1);
		strcpy(params.iv, iv);
	}

	//check length
	if(strlen(params.iv) != KEY_LEN){
		fprintf(stderr, "Initiation vector must be %d bytes in length\n", KEY_LEN);
		status = -EINVAL;
		goto CLEANUP;
	}

	//figure out file size
	fseek(in_f, 0, SEEK_END);
	int f_size = ftell(in_f);
	fseek(in_f, 0, SEEK_SET);

	//Calculate aligned size
	int aligned_f_size = (f_size % KEY_LEN) != 0 ? f_size + KEY_LEN - f_size % KEY_LEN : f_size;

	//allocate memory buffer for input file
	in_data = malloc(aligned_f_size);
	out_data = malloc(aligned_f_size);

	//read entire file to memory
	f_size = fread(in_data, 1, aligned_f_size, in_f);
	memset(in_data + f_size, 0, aligned_f_size - f_size);

	//Do actual encryption or decryption
	if(params.decryption_requested){
		//Decrypt file
		fprintf(stderr, "Decrypting...\n");
		AES128_CBC_decrypt_buffer(out_data, in_data, aligned_f_size, (uint8_t *)params.key, (uint8_t *)params.iv);
	} else {
		fprintf(stderr, "Encrypting...\n");
		AES128_CBC_encrypt_buffer(out_data, in_data, aligned_f_size, (uint8_t *)params.key, (uint8_t *)params.iv);
	}
	fprintf(stderr, "Done.\n");

	//Write output
	fwrite(out_data, aligned_f_size, 1, out_f);

CLEANUP:

	//Free memory
	if(in_data)
		free(in_data);
	if(out_data)
		free(out_data);
	if(params.key)
		free(params.key);
	if(params.iv)
		free(params.iv);

	//close files
	if(in_f)
		fclose(in_f);
	if(out_f)
		fclose(out_f);

	return status;
}





