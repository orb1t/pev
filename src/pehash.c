/*
    pev - the PE file analyzer toolkit
    
    pehash.c - calculate PE file cryptographic signatures

    Copyright (C) 2012 Jan Seidl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "pehash.h"

PEV_OUTPUT_SECTOR *output = NULL;
extern struct options config;
static int ind;

void usage() 
{
    printf("Usage: %s OPTIONS FILE\n"
    "Show PE file cryptographic signatures\n"
    "\nExample: %s --hash md5 winzip.exe\n"
    "\nOptions:\n"
    " -A, --all                              full output (all available hashes) (default)\n"
    " -f, --format <text|csv|xml|html>       change output format (default text)\n"
    " -h, --hash <md5|sha1|sha256>           hashing algorithm\n"
    " -v, --version                          show version and exit\n"
    " --help                                 show this help and exit\n",
    PROGRAM, PROGRAM);

}//end :: usage

void parse_hash_algorithm(const char *optarg)
{
    if (! strcmp(optarg, "md5"))
        config.md5 = true;
    else if (! strcmp(optarg, "sha1"))
        config.sha1 = true;
    else if (! strcmp(optarg, "sha256"))
        config.sha256 = true;
    else
        EXIT_ERROR("invalid hashing algorithm option");
}//end :: parse_hash_algorithm

void parse_options(int argc, char *argv[])
{
    int c;

    /* Parameters for getopt_long() function */
    static const char short_options[] = "AHSh:dif:v";

    static const struct option long_options[] = {
        {"help",             no_argument,       NULL,  1 },
        {"all",              no_argument,       NULL, 'A'},
        {"hash",             required_argument, NULL, 'h'},
        {"format",           required_argument, NULL, 'f'},
        {"version",          no_argument,       NULL, 'v'},
        { NULL,              0,                 NULL,  0 }
    };

    // setting all fields to false
    memset(&config, false, sizeof(config));

    config.all = true;

    while ((c = getopt_long(argc, argv, short_options,
            long_options, &ind)))
    {
        if (c < 0)
            break;


        switch (c)
        {
            case 1:     // --help option
                usage();
                exit(EXIT_SUCCESS);

            case 'A':
                config.all = true; break;

            case 'v':
                printf("%s %s\n%s\n", PROGRAM, TOOLKIT, COPY);
                exit(EXIT_SUCCESS);

            case 'h':
                config.all = false;
                parse_hash_algorithm(optarg); break;
                
            case 'f':
                parse_format(optarg); break;

            default:
                fprintf(stderr, "%s: try '--help' for more information\n", PROGRAM);
                exit(EXIT_FAILURE);
        }
    }
}//end :: parse_config

int main(int argc, char *argv[])
{

    FILE *fp = NULL;
    
    if (argc < 2)
    {
        usage();
        exit(1);
    }//end :: if

    parse_options(argc, argv); // opcoes

    if ((fp = fopen(argv[argc-1], "rb")) == NULL)
        EXIT_ERROR("file not found or unreadable");

    PEV_OUTPUT_SECTOR *outSectorSignatures = add_sector("Cryptographic signatures", &output);

    if (true == config.all || true == config.md5) {
        char md5_sum[((MD5_DIGEST_LENGTH*2)+1)];
        calc_file_md5(fp, md5_sum);
    
        add_line("MD5 Signature", md5_sum, outSectorSignatures);
    }//end :: if

    if (true == config.all || true == config.sha256) {
        char sha256_sum[((SHA256_DIGEST_LENGTH*2)+1)];
        calc_file_sha256(fp, sha256_sum);
    
        add_line("SHA256 Signature", sha256_sum, outSectorSignatures);
    }//end :: if

    if (true == config.all || true == config.sha1) {
        char sha1_sum[((SHA_DIGEST_LENGTH*2)+1)];
        calc_file_sha1(fp, sha1_sum);
    
        add_line("SHA1 Signature", sha1_sum, outSectorSignatures);
    }//end :: if

    fclose (fp);

    dump_output(output);

    return 0;

}//end :: main

void calc_file_sha1(FILE *fp, char *sha1sum)
{

    if (fp == NULL) EXIT_ERROR("file not found or unreadable");
    rewind(fp);

    unsigned char hash[SHA_DIGEST_LENGTH];

    SHA_CTX mdContext;

    size_t bytes;
    unsigned char data[1024];

    SHA1_Init(&mdContext);

    while ((bytes = fread (data, 1, 1024, fp)) != 0)
        SHA1_Update (&mdContext, data, bytes);

    SHA1_Final(hash, &mdContext);

    unsigned int i;
    for(i = 0; i < SHA_DIGEST_LENGTH; i++) sprintf(&sha1sum[i*2], "%02x", hash[i]);


}//end :: calc_file_sha1


void calc_file_sha256(FILE *fp, char *sha256sum)
{

    if (fp == NULL) EXIT_ERROR("file not found or unreadable");
    rewind(fp);

    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256_CTX mdContext;

    size_t bytes;
    unsigned char data[1024];

    SHA256_Init(&mdContext);

    while ((bytes = fread (data, 1, 1024, fp)) != 0)
        SHA256_Update (&mdContext, data, bytes);

    SHA256_Final(hash, &mdContext);

    unsigned int i;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++) sprintf(&sha256sum[i*2], "%02x", hash[i]);


}//end :: calc_file_sha256

void calc_file_md5(FILE *fp, char *md5sum)
{

    if (fp == NULL) EXIT_ERROR("file not found or unreadable");
    rewind(fp);

    unsigned char hash[MD5_DIGEST_LENGTH];

    size_t bytes;

    MD5_CTX mdContext;

    unsigned char data[1024];

    MD5_Init (&mdContext);

    while ((bytes = fread (data, 1, 1024, fp)) != 0)
        MD5_Update (&mdContext, data, bytes);

    MD5_Final (hash,&mdContext);

    unsigned int i;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) sprintf(&md5sum[i*2], "%02x", hash[i]);

}//end :: calc_file_md5
