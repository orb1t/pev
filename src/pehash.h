/*
    pev - the PE file analyzer toolkit
    
    pehash.h - definitions for pehash.c

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

#ifndef PEHASH_H
#define PEHASH_H

#include "common.h"
#include <openssl/md5.h>
#include <openssl/sha.h>

#define PROGRAM "pehash"

// Prototypes
void usage();
int main(int argc, char *argv[]);
void parse_hash_algorithm(const char *optarg);

// Algorithms
void calc_md5(FILE *fp, char *md5sum);
void calc_sha256(FILE *fp, char *sha265sum);

// Structs
struct options {
    bool all;
    bool md5;
    bool sha1;
    bool sha256;
};

struct options config;

#endif

