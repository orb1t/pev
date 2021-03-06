/*
	pev - the PE file analyzer toolkit
	
	readpe.c - show PE file headers

	Copyright (C) 2012 Fernando Mercês

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

#include "readpe.h"

extern struct options config;
static int ind;

PEV_OUTPUT_SECTOR *output = NULL;

void usage()
{
	printf("Usage: %s OPTIONS FILE\n"
	"Show PE file headers\n"
	"\nExample: %s --header optional winzip.exe\n"
	"\nOptions:\n"
	" -A, --all                              full output (default)\n"
	" -H, --all-headers                      print all PE headers\n"
	" -S, --all-sections                     print all PE sections headers\n"
	" -f, --format <text|csv|xml|html>       change output format (default text)\n"
	" -d, --dirs                             show data directories\n"
	" -h, --header <dos|coff|optional>       show sepecific header\n"
	" -i, --imports                          show imported functions\n"
	" -v, --version                          show version and exit\n"
	" --help                                 show this help and exit\n",
	PROGRAM, PROGRAM);
}

void parse_headers(const char *optarg)
{
	if (! strcmp(optarg, "dos"))
		config.dos = true;
	else if (! strcmp(optarg, "coff"))
		config.coff = true;
	else if (! strcmp(optarg, "optional"))
		config.opt = true;
	else
		EXIT_ERROR("invalid header option");
}

void parse_options(int argc, char *argv[])
{
	int c;

	/* Parameters for getopt_long() function */
	static const char short_options[] = "AHSh:dif:v";

	static const struct option long_options[] = {
		{"help",             no_argument,       NULL,  1 },
		{"all",              no_argument,       NULL, 'A'},
		{"all-headers",      no_argument,       NULL, 'H'},
		{"all-sections",     no_argument,       NULL, 'S'},
		{"header",           required_argument, NULL, 'h'},
		{"imports",          no_argument,       NULL, 'i'},
		{"dirs",             no_argument,       NULL, 'd'},
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
			case 1:		// --help option
				usage();
				exit(EXIT_SUCCESS);

			case 'A':
				config.all = true; break;

			case 'H':
				config.all = false;
				config.all_headers = true; break;

			case 'd':
				config.all = false;
				config.dirs = true; break;

			case 'S':
				config.all = false;
				config.all_sections = true; break;

			case 'v':
				printf("%s %s\n%s\n", PROGRAM, TOOLKIT, COPY);
				exit(EXIT_SUCCESS);

			case 'h':
				config.all = false;
				parse_headers(optarg); break;
				
			case 'i':
				config.all = false;
				config.imports = true; break;

			case 'f':
				parse_format(optarg); break;

			default:
				fprintf(stderr, "%s: try '--help' for more information\n", PROGRAM);
				exit(EXIT_FAILURE);
		}
	}
}

char *dec2bin(unsigned int dec, char *bin, unsigned int bits)
{
	unsigned int i;
	
	for(i=0; i < bits; i++)
		bin[bits - i - 1] = (dec & (0x1 << i)) ? '1' : '0';

	bin[bits] = '\0';

	return bin;
}

void print_sections(PE_FILE *pe)
{
	char s[MAX_MSG];
	unsigned int i, j;

	char *flags[] = {
   "contains executable code",
   "contains initialized data",
   "contains uninitialized data",
   "contains data referenced through the GP",
   "contains extended relocations",
   "can be discarded as needed",
   "cannot be cached",
   "is not pageable",
   "can be shared in memory",
   "is executable",
   "is readable",
   "is writable" };

   // valid flags only for executables referenced in pecoffv8
   unsigned int valid_flags[] =
   { 0x20, 0x40, 0x80, 0x8000, 0x1000000, 0x2000000, 0x4000000,
     0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000 };

        PEV_OUTPUT_SECTOR *outSectorSections = add_sector("Sections", &output);

	if (pe->num_sections > 96)
		return;

	for (i=0; i < pe->num_sections; i++)
	{

        PEV_OUTPUT_SECTOR *outSubSectSection = add_child_sector("Section", &outSectorSections);

		snprintf(s, MAX_MSG, "%s", pe->sections_ptr[i]->Name);
		add_line("Name", s, outSubSectSection);

		snprintf(s, MAX_MSG, "%#x", pe->sections_ptr[i]->VirtualAddress);
		add_line("Virtual Address", s, outSubSectSection);

		snprintf(s, MAX_MSG, "%#x", pe->sections_ptr[i]->Misc.PhysicalAddress);
		add_line("Physical Address", s, outSubSectSection);

		snprintf(s, MAX_MSG, "%#x (%d bytes)", pe->sections_ptr[i]->SizeOfRawData,
		pe->sections_ptr[i]->SizeOfRawData);
		add_line("Size", s, outSubSectSection);

		snprintf(s, MAX_MSG, "%#x", pe->sections_ptr[i]->PointerToRawData);
		add_line("Pointer To Data", s, outSubSectSection);

		snprintf(s, MAX_MSG, "%d", pe->sections_ptr[i]->NumberOfRelocations);
		add_line("Relocations", s, outSubSectSection);

		snprintf(s, MAX_MSG, "%#x", pe->sections_ptr[i]->Characteristics);
		add_line("Characteristics", s, outSubSectSection);

		for (j=0; j < sizeof(valid_flags) / sizeof(unsigned int); j++)
		{
			if (pe->sections_ptr[i]->Characteristics & valid_flags[j])
			{
					snprintf(s, MAX_MSG, "%s", flags[j]);
					//add_line(NULL, s, outSectorSections);
			}
		}
	}
}

void print_directories(PE_FILE *pe)
{
	char s[MAX_MSG];
	unsigned int i;

	static const char *directory_names[] =
	{
		"Export Table", // 0
		"Import Table",
		"Resource Table",
		"Exception Table",
		"Certificate Table",
		"Base Relocation Table",
		"Debug",
		"Architecture",
		"Global Ptr",
		"Thread Local Storage (TLS)", // 9
		"Load Config Table",
		"Bound Import",
		"Import Address Table (IAT)",
		"Delay Import Descriptor",
		"CLR Runtime Header", "" // 14
	};

	PEV_OUTPUT_SECTOR *outSectorDataDirectories = add_sector("Data directories", &output);

	if (! pe->directories_ptr)
		return;

	for (i=0; i < pe->num_directories && i < 16; i++)
	{
		if (pe->directories_ptr[i]->Size)
		{
			snprintf(s, MAX_MSG, "%#x (%d bytes)",
					pe->directories_ptr[i]->VirtualAddress,
					pe->directories_ptr[i]->Size);
			add_line((char *) directory_names[i], s, outSectorDataDirectories);
		}
	}
}

void print_optional_header(PE_FILE *pe)
{
	char s[MAX_MSG];
	int subsystem;

	static const char *subs_desc[] = {
	"Unknown subsystem",
	"System native",
	"Windows GUI",
	"Windows CLI",
	"Posix CLI",
	"Windows CE GUI",
	"EFI application",
	"EFI driver with boot",
	"EFI run-time driver",
	"EFI ROM",
	"XBOX"};

	if (!pe->optional_ptr)
		return;

	PEV_OUTPUT_SECTOR *outSectorOptionalHeader = add_sector("Optional/Image header", &output);

	if (pe->optional_ptr->_32)
	{
		snprintf(s, MAX_MSG, "%#x (%s)", pe->optional_ptr->_32->Magic, "PE32");
		add_line("Magic number", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MajorLinkerVersion);
		add_line("Linker major version", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MinorLinkerVersion);
		add_line("Linker minor version", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfCode);
		add_line("Size of .text section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfInitializedData);
		add_line("Size of .data section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfUninitializedData);
		add_line("Size of .bss section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->AddressOfEntryPoint);
		add_line("Entrypoint", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->BaseOfCode);
		add_line("Address of .text section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->BaseOfData);
		add_line("Address of .data section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->ImageBase);
		add_line("ImageBase", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SectionAlignment);
		add_line("Alignment of sections", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->FileAlignment);
		add_line("Alignment factor", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MajorOperatingSystemVersion);
		add_line("Major version of required OS", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MinorOperatingSystemVersion);
		add_line("Minor version of required OS", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MajorImageVersion);
		add_line("Major version of image", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MinorImageVersion);
		add_line("Minor version of image", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MajorSubsystemVersion);
		add_line("Major version of subsystem", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_32->MinorSubsystemVersion);
		add_line("Minor version of subsystem", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfImage);
		add_line("Size of image", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfHeaders);
		add_line("Size of headers", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->CheckSum);
		add_line("Checksum", s, outSectorOptionalHeader);

		subsystem = pe->optional_ptr->_32->Subsystem;
		snprintf(s, MAX_MSG, "%#x (%s)", subsystem, subsystem <= 10 ? subs_desc[subsystem] : "Unknown");
		add_line("Subsystem required", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->DllCharacteristics);
		add_line("DLL characteristics", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfStackReserve);
		add_line("Size of stack to reserve", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfStackCommit);
		add_line("Size of stack to commit", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfHeapReserve);
		add_line("Size of heap space to reserve", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_32->SizeOfHeapCommit);
		add_line("Size of heap space to commit", s, outSectorOptionalHeader);
	}
	else if (pe->optional_ptr->_64)
	{
		snprintf(s, MAX_MSG, "%#x (%s)", pe->optional_ptr->_64->Magic, "PE32+");
		add_line("Magic number", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MajorLinkerVersion);
		add_line("Linker major version", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MinorLinkerVersion);
		add_line("Linker minor version", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->SizeOfCode);
		add_line("Size of .text section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->SizeOfInitializedData);
		add_line("Size of .data section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->SizeOfUninitializedData);
		add_line("Size of .bss section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->AddressOfEntryPoint);
		add_line("Entrypoint", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->BaseOfCode);
		add_line("Address of .text section", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#"PRIx64, pe->optional_ptr->_64->ImageBase);
		add_line("ImageBase", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->SectionAlignment);
		add_line("Alignment of sections", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->FileAlignment);
		add_line("Alignment factor", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MajorOperatingSystemVersion);
		add_line("Major version of required OS", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MinorOperatingSystemVersion);
		add_line("Minor version of required OS", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MajorImageVersion);
		add_line("Major version of image", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MinorImageVersion);
		add_line("Minor version of image", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MajorSubsystemVersion);
		add_line("Major version of subsystem", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%d", pe->optional_ptr->_64->MinorSubsystemVersion);
		add_line("Minor version of subsystem", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->SizeOfImage);
		add_line("Size of image", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->SizeOfHeaders);
		add_line("Size of headers", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->CheckSum);
		add_line("Checksum", s, outSectorOptionalHeader);

		subsystem = pe->optional_ptr->_64->Subsystem;
		snprintf(s, MAX_MSG, "%#x (%s)", subsystem, subsystem <= 10 ? subs_desc[subsystem] : "Unknown");
		add_line("Subsystem required", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#x", pe->optional_ptr->_64->DllCharacteristics);
		add_line("DLL characteristics", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#"PRIx64, pe->optional_ptr->_64->SizeOfStackReserve);
		add_line("Size of stack to reserve", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#"PRIx64, pe->optional_ptr->_64->SizeOfStackCommit);
		add_line("Size of stack to commit", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#"PRIx64, pe->optional_ptr->_64->SizeOfHeapReserve);
		add_line("Size of heap space to reserve", s, outSectorOptionalHeader);

		snprintf(s, MAX_MSG, "%#"PRIx64, pe->optional_ptr->_64->SizeOfHeapCommit);
		add_line("Size of heap space to commit", s, outSectorOptionalHeader);
	}
}

void print_coff_header(IMAGE_COFF_HEADER *header)
{
	char s[MAX_MSG];
	char time[40];
	register unsigned int i, j;
	char *machine = "Unknown machine type";

	static const char *flags[] =
	{
		"base relocations stripped",
		"executable image",
		"line numbers removed (deprecated)",
		"local symbols removed (deprecated)",
		"aggressively trim (deprecated for Windows 2000 and later)",
		"can handle more than 2 GB addresses", "",
		"little-endian (deprecated)",
		"32-bit machine",
		"debugging information removed",
		"copy to swap if it's on removable media",
		"copy to swap if it's on network media",
		"system file",
		"DLL image",
		"uniprocessor machine",
		"big-endian (deprecated)"
	};

	static const MACHINE_ENTRY arch[] =
	{
		{"Any machine type", 0x0},
		{"Matsushita AM33", 0x1d3},
		{"x86-64 (64-bits)", 0x8664},
		{"ARM little endian", 0x1c0},
		{"ARMv7 (or higher) Thumb mode only", 0x1c4},
		{"EFI byte code", 0xebc},
		{"Intel 386 and compatible (32-bits)", 0x14c},
		{"Intel Itanium", 0x200},
		{"Mitsubishi M32R little endian", 0x9041},
		{"MIPS16", 0x266},
		{"MIPS with FPU", 0x366},
		{"MIPS16 with FPU", 0x466},
		{"Power PC little endian", 0x1f0},
		{"Power PC with floating point support", 0x1f1},
		{"MIPS little endian", 0x166},
		{"Hitachi SH3", 0x1a2},
		{"Hitachi SH3 DSP", 0x1a3},
		{"Hitachi SH4", 0x1a6},
		{"Hitachi SH5",  0x1a8},
		{"ARM or Thumb (\"interworking\")", 0x1c2},
		{"MIPS little-endian WCE v2", 0x169}
	};

	PEV_OUTPUT_SECTOR *outSectorCoff = add_sector("COFF/File header", &output);

	for(i=0; i<(sizeof(arch)/sizeof(MACHINE_ENTRY)); i++)
	{
		if(header->Machine == arch[i].code)
			machine = (char*)arch[i].name;
	}

	snprintf(s, MAX_MSG, "%#x %s", header->Machine, machine);
	add_line("Machine", s, outSectorCoff);

	snprintf(s, MAX_MSG, "%d", header->NumberOfSections);
	add_line("Number of sections", s, outSectorCoff);

	strftime(time, 40, "%a - %d %b %Y %H:%M:%S UTC", gmtime((time_t *) & header->TimeDateStamp));
	snprintf(s, MAX_MSG, "%d (%s)", header->TimeDateStamp, time);
	add_line("Date/time stamp", s, outSectorCoff);

	snprintf(s, MAX_MSG, "%#x", header->PointerToSymbolTable);
	add_line("Symbol Table offset", s, outSectorCoff);

	snprintf(s, MAX_MSG, "%d", header->NumberOfSymbols);
	add_line("Number of symbols", s, outSectorCoff);

	snprintf(s, MAX_MSG, "%#x", header->SizeOfOptionalHeader);
	add_line("Size of optional header", s, outSectorCoff);

	snprintf(s, MAX_MSG, "%#x", header->Characteristics);
	add_line("Characteristics", s, outSectorCoff);

	for (i=1, j=0; i<0x8000; i<<=1, j++)
	{
		//if (header->Characteristics & i)
			//add_line(NULL, (char*) flags[j], outSectorCoff);
	}
}

void print_dos_header(IMAGE_DOS_HEADER *header)
{
	char s[MAX_MSG];

	PEV_OUTPUT_SECTOR *outSectorDosHeader = add_sector("DOS Header", &output);

	snprintf(s, MAX_MSG, "%#x (MZ)", header->e_magic);
	add_line("Magic number", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%d", header->e_cblp);
	add_line("Bytes in last page", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%d", header->e_cp);
	add_line("Pages in file", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%d", header->e_crlc);
	add_line("Relocations", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%d", header->e_cparhdr);
	add_line("Size of header in paragraphs", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%d", header->e_minalloc);
	add_line("Minimum extra paragraphs", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%d", header->e_maxalloc);
	add_line("Maximum extra paragraphs", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_ss);
	add_line("Initial (relative) SS value", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_sp);
	add_line("Initial SP value", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_ip);
	add_line("Initial IP value", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_cs);
	add_line("Initial (relative) CS value", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_lfarlc);
	add_line("Address of relocation table", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_ovno);
	add_line("Overlay number", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_oemid);
	add_line("OEM identifier", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_oeminfo);
	add_line("OEM information", s, outSectorDosHeader);

	snprintf(s, MAX_MSG, "%#x", header->e_lfanew);
	add_line("PE header offset", s, outSectorDosHeader);
}

void print_imported_functions(PE_FILE *pe, long offset)
{
	QWORD fptr = 0; // pointer to functions
	long aux2, aux = ftell(pe->handle);
	WORD hint = 0; // function number
	char c;
	char fname[MAX_FUNCTION_NAME];
	char hintstr[16];
	unsigned int i;
	
	if (fseek(pe->handle, offset, SEEK_SET))
		return;

	memset(&fname, 0, sizeof(fname));
	memset(&hintstr, 0, sizeof(hintstr));

    //PEV_OUTPUT_SECTOR *outSectorImportedFunctions = add_sector("Imported Functions", &output);
	
	while (1)
	{
		if (!fread(&fptr, (pe->architecture == PE64) ? sizeof(QWORD) : sizeof(DWORD), 1, pe->handle))
			return;
		
		if (!fptr)
			break;

		// function without name (test msb)
		if (fptr & ((pe->architecture == PE64) ? IMAGE_ORDINAL_FLAG64 : IMAGE_ORDINAL_FLAG32))
			snprintf(hintstr, 15, "%lu", fptr & 0x0fffffff);
		else
		{
			// save file pointer in functions array
			aux2 = ftell(pe->handle);
		
			if (fseek(pe->handle, rva2ofs(pe, fptr), SEEK_SET))
				return;

			// follow function pointer
			if (!fread(&hint, sizeof(hint), 1, pe->handle))
				return;
		
			for (i=0; i<MAX_FUNCTION_NAME; i++)
			{
				if (!fread(&c, sizeof(c), 1, pe->handle))
					return;
			
				if (!isprint(c)) // 0 and non-printable
					break;
			
				fname[i] = c;
			}
			snprintf(hintstr, 15, "%d", hint);
		
			// restore file pointer to functions array
			if (fseek(pe->handle, aux2, SEEK_SET))
				return;
		}

		// print things
		//add_line(hintstr, fname, outSectorImportedFunctions);
		memset(&fname, 0, sizeof(fname));
		memset(&hintstr, 0, sizeof(hintstr));
	}
		
	fseek(pe->handle, aux, SEEK_SET);
}

void print_imports(PE_FILE *pe)
{
	QWORD va; // store temporary addresses
	long aux;
	IMAGE_IMPORT_DESCRIPTOR id;
	char c = 0;
	char dllname[MAX_DLL_NAME];
	unsigned int i;

	va = pe->directories_ptr[IMAGE_DIRECTORY_ENTRY_IMPORT] ? 
	pe->directories_ptr[IMAGE_DIRECTORY_ENTRY_IMPORT]->VirtualAddress : 0;

	if (fseek(pe->handle, rva2ofs(pe, va), SEEK_SET))
		EXIT_ERROR("error seeking file");
	
	memset(&id, 0, sizeof(id));
	memset(&dllname, 0, sizeof(dllname));

	while (1)
	{
		if (!fread(&id, sizeof(id), 1, pe->handle))
			return;

		if (!id.u1.OriginalFirstThunk)
			break;

		aux = ftell(pe->handle);
		va = rva2ofs(pe, id.Name);
		
		if (!va)
			return;

		// shortcut to read DLL name
		if (fseek(pe->handle, va, SEEK_SET))
			return;

		// print dll name
		for (i=0; i < MAX_DLL_NAME; i++)
		{
			fread(&c, sizeof(c), 1, pe->handle);
			
			if (!c)
				break;
			
			dllname[i] = c;
		}
		
		//add_line(dllname, NULL);
		memset(&dllname, 0, sizeof(dllname));
		
		if (fseek(pe->handle, aux, SEEK_SET)) // restore file pointer
			return;

		// search for dll imported functions
		va = rva2ofs(pe, id.u1.OriginalFirstThunk);

		if (!va)
			return;

		print_imported_functions(pe, va);
	}
}

int main(int argc, char *argv[])
{
	PE_FILE pe;
	FILE *fp = NULL;
	
	if (argc < 2)
	{
		usage();
		exit(1);
	}
	
	parse_options(argc, argv); // opcoes

	if ((fp = fopen(argv[argc-1], "rb")) == NULL)
		EXIT_ERROR("file not found or unreadable");

	pe_init(&pe, fp); // inicializa o struct pe

	if (!ispe(&pe))
		EXIT_ERROR("not a valid PE file");

	// dos header
	if (config.dos || config.all_headers || config.all)
	{
		IMAGE_DOS_HEADER dos;

		if (pe_get_dos(&pe, &dos))
			print_dos_header(&dos);
		else { EXIT_ERROR("unable to read DOS header"); }
	}

	// coff/file header
	if (config.coff || config.all_headers || config.all)
	{
		IMAGE_COFF_HEADER coff;

		if (pe_get_coff(&pe, &coff))
			print_coff_header(&coff);
		else { EXIT_ERROR("unable to read COFF file header"); }
	}

	// optional header
	if (config.opt || config.all_headers || config.all)
	{
		if (pe_get_optional(&pe))
			print_optional_header(&pe);
		else { EXIT_ERROR("unable to read Optional (Image) file header"); }
	}

	// directories
	if (config.dirs || config.all)
	{
		if (pe_get_directories(&pe))
			print_directories(&pe);
		else { EXIT_ERROR("unable to read the Directories entry from Optional header"); }
	}
	
	// imports
	if (config.imports || config.all)
	{
		if (pe_get_directories(&pe))
			print_imports(&pe);
		else { EXIT_ERROR("unable to read the Directories entry from Optional header"); }
	}	

	// sections
	if (config.all_sections || config.all)
	{
		if (pe_get_sections(&pe))
			print_sections(&pe);
		else { EXIT_ERROR("unable to read sections"); }
	}

	// free
    dump_output(output);
	pe_deinit(&pe);
	return 0;
}
