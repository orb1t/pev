/*
    pev - the PE file analyzer toolkit
    
    output.h - definitions for output results in differents formats
    
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

#ifndef OUTPUT_H
#define OUTPUT_H

/* Macros */

#define KEY_MAX_LEN 255
#define VALUE_MAX_LEN 1000
#define TITLE_MAX_LEN 1000

#define HTML_HEAD "<!DOCTYPE HTML>\n<html>\n\t<head>\n\t\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n\t\t<title>PEV Output for %s (%s)</title>\n\t\t<style type=\"text/css\">\n\t\t\tdt { font-weight: bold; }\n\t\t\th1 { padding-top: 11px; padding-bottom: 11px; padding-left: 70px; background: transparent url(data:image/jpeg;base64,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) scroll no-repeat center left; }\n\t\th2 { clear: both; float: none; margin-bottom: 0px; }\n\t\tdl { margin-bottom: 20px; float: left; }\n\t\tdt { float: left; clear: left; width: 240px; font-weight: bold; margin-bottom: 5px; }\n\t\tdd { float: left; }\n\t\t</style>\n\t</head>\n\t<body>\n"
#define HTML_FOOTER "\t</body>\n</html>"

#define XML_HEAD "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<pe>\n"
#define XML_FOOTER "</pe>"

/* structs, enums, typedefs etc... */

typedef enum {
    FORMAT_TEXT = 1,
    FORMAT_HTML = 2,
    FORMAT_XML = 3,
    FORMAT_CSV = 4
} format_e;

format_e format;

typedef struct Sector
{

    char title[TITLE_MAX_LEN];

    struct Line *firstLine;

    struct Sector *parent;
    struct Sector *child;

    struct Sector *next;
    struct Sector *prev;

} PEV_OUTPUT_SECTOR;//end :: struct :: Sector

typedef struct Line
{
    char key[KEY_MAX_LEN];
    char value[VALUE_MAX_LEN];

    struct Line *next;
    struct Line *prev;
} PEV_OUTPUT_LINE;//end :: struct :: Line

/* Prototypes */
// Core Functions
PEV_OUTPUT_SECTOR * get_last_sector(PEV_OUTPUT_SECTOR *sector);
PEV_OUTPUT_SECTOR * add_sector(char *name, PEV_OUTPUT_SECTOR ** currentSector);
PEV_OUTPUT_LINE * get_last_sector_line(PEV_OUTPUT_SECTOR *sector);
PEV_OUTPUT_LINE add_line(char *key, char *value, PEV_OUTPUT_SECTOR *sector);

// Output Parsers
void dump_output_text(PEV_OUTPUT_SECTOR *sector);
void dump_output_html(PEV_OUTPUT_SECTOR *sector);
void dump_output_xml(PEV_OUTPUT_SECTOR *sector);
void dump_output_csv(PEV_OUTPUT_SECTOR *sector);
void dump_output(PEV_OUTPUT_SECTOR *sector);

// Util Functions
void parse_format(const char *optarg);
char * slugify(char *string);

// Parser functions
void html_print_header();
void html_print_footer();
void xml_print_header();
void xml_print_footer();
#endif

