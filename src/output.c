/*
pev - the PE file analyzer toolkit
output.c - functions to output results in different formats

Copyright (C) 2012 Fernando Mercês
Copyright (C) 2012 Gabriel Duarte
Copyright (C) 2012 Jan Seidl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "output.h"
#include "common.h"

extern format_e format;

PEV_OUTPUT_SECTOR * get_last_sector(PEV_OUTPUT_SECTOR *sector)
{

    if (!sector) return NULL;

    PEV_OUTPUT_SECTOR *current_sector = sector;

    while (current_sector != NULL)
    {

        if (current_sector->next == NULL) {
            return current_sector;
        }
        current_sector = current_sector->next;

    }//end :: while

    return NULL;

}//end :: get_last_sector

PEV_OUTPUT_SECTOR * add_sector(char *name, PEV_OUTPUT_SECTOR ** currentSector)
{

    PEV_OUTPUT_SECTOR *sector;

    sector = (PEV_OUTPUT_SECTOR *)xmalloc(sizeof(PEV_OUTPUT_SECTOR));

    strncpy(sector->title, name, TITLE_MAX_LEN);

    sector->firstLine = NULL;

    PEV_OUTPUT_SECTOR *lastSector;
    lastSector =  (PEV_OUTPUT_SECTOR *) get_last_sector(*currentSector);

    // First sector?
    if (NULL != lastSector)
    {
        PEV_OUTPUT_SECTOR *last = lastSector->next;
        lastSector->next = sector;
        if (last != NULL) last->prev = sector;
        sector->prev = lastSector;
    } else {
        sector->prev = NULL;
        if (NULL == *currentSector) {
            *currentSector = sector;
        }//end :: if
    }//end :: if
    sector->next = NULL;

    return sector;
        
}//end :: add_sector

PEV_OUTPUT_LINE * get_last_sector_line(PEV_OUTPUT_SECTOR *sector)
{

    PEV_OUTPUT_LINE *current_line = sector->firstLine;

    while (current_line != NULL)
    {

        if (current_line->next == NULL) return current_line;
        current_line = current_line->next;

    }//end :: while

    return NULL;

}//end :: get_last_sector_line

PEV_OUTPUT_LINE add_line(char *key, char *value, PEV_OUTPUT_SECTOR *sector)
{

    PEV_OUTPUT_LINE *line;
    line = (PEV_OUTPUT_LINE *)xmalloc(sizeof(PEV_OUTPUT_LINE));


    strncpy(line->key, key, KEY_MAX_LEN);
    strncpy(line->value, value, VALUE_MAX_LEN);

    PEV_OUTPUT_LINE *lastLine;
    lastLine = (PEV_OUTPUT_LINE *) get_last_sector_line(sector);

    if (NULL != lastLine)
    {
        lastLine->next = line;
        line->prev = lastLine;
    } else {
        sector->firstLine = line;
    }

    line->next = NULL;

    return *line;

}//end :: add_line

void dump_output_text(PEV_OUTPUT_SECTOR *sector) 
{

    PEV_OUTPUT_SECTOR *current = sector;

    while (current != NULL)
    {
        printf("%s\n", current->title);

        PEV_OUTPUT_LINE *current_line = current->firstLine;

        while (current_line != NULL)
        {

            printf("\t%s: %s\n", current_line->key, current_line->value);
            current_line = current_line->next;

        }//end :: while

        printf("\n");

        current = current->next;
    }//end :: while

}//end :: dump_output_text

// CSV

void dump_output_csv(PEV_OUTPUT_SECTOR *sector)
{

    PEV_OUTPUT_SECTOR *current = sector;

    while (current != NULL)
    {
        PEV_OUTPUT_LINE *current_line = current->firstLine;

        while (current_line != NULL)
        {

            printf("\"%s\",\"%s\",\"%s\"\n", current->title, current_line->key, current_line->value);
            current_line = current_line->next;

        }//end :: while

        current = current->next;
    }//end :: while



}//end :: dump_output_sector

// HTML FUNCTIONS

void html_print_header()
{

    printf(HTML_HEAD, "sampleprog.txt", "2010-11-12");
    printf("\t<h1>PEV Output</h1>\n");

}//end :: html_print_header

void html_print_footer()
{

    printf(HTML_FOOTER);

}//end :: html_print_footer

char * slugify(char *string)
{

    char *cp;
    cp = string;

    while (*cp) {
        if (64 < *cp && 91 > *cp) *cp = *cp+32;
        if (32 == *cp) *cp = '_';
        if ('(' == *cp || ')' == *cp || '/' == *cp || '\\' == *cp || '.' == *cp) *cp = '_';

        cp++;
    }//end :: while
    return string;

}//end :: slugify

void dump_output_html(PEV_OUTPUT_SECTOR *sector)
{


    PEV_OUTPUT_SECTOR *current = sector;

    html_print_header();

    while (NULL != current) {
        printf("\t<h2>%s</h2>\n", current->title);

        PEV_OUTPUT_LINE *current_line = current->firstLine;

        printf("\t<dl>\n");

        while (current_line != NULL)
        {

            printf("\t\t<dt>%s</dt>\n", current_line->key);
            printf("\t\t<dd>%s</dd>\n", current_line->value);
            current_line = current_line->next;

        }//end :: while

        printf("\t</dl>\n");

        current = current->next;
    }//end :: while

    html_print_footer();

}//end :: dump_output_html

// XML Functions

void xml_print_header()
{

    printf(XML_HEAD);

}//end :: xml_print_header

void xml_print_footer()
{

    printf(XML_FOOTER);

}//end :: xml_print_footer


void dump_output_xml(PEV_OUTPUT_SECTOR *sector)
{


    PEV_OUTPUT_SECTOR *current = sector;

    xml_print_header();

    while (NULL != current) {

        char sector_slug[TITLE_MAX_LEN];

        strncpy(sector_slug, current->title, TITLE_MAX_LEN);
        slugify(sector_slug);

        printf("\t<%s title=\"%s\">\n", sector_slug, current->title);

        PEV_OUTPUT_LINE *current_line = current->firstLine;

        while (current_line != NULL)
        {

            printf("\t\t<%s>%s</%s>\n", slugify(current_line->key), current_line->value, slugify(current_line->key));
            current_line = current_line->next;

        }//end :: while

        printf("\t</%s>\n", slugify(current->title));

        current = current->next;
    }//end :: while

    xml_print_footer();

}//end :: dump_output_xml

void parse_format(const char *optarg)
{

    if (! strcmp(optarg, "text"))
        format = FORMAT_TEXT;
    else if (! strcmp(optarg, "xml"))
        format = FORMAT_XML;
    else if (! strcmp(optarg, "csv"))
        format = FORMAT_CSV;
    else if (! strcmp(optarg, "html"))
        format = FORMAT_HTML;
    else
        EXIT_ERROR("invalid format option");

}//end :: parse_format

void dump_output(PEV_OUTPUT_SECTOR *sector) 
{

    switch (format)
    {
        case FORMAT_XML:
            dump_output_xml(sector);
            break;
        case FORMAT_CSV:
            dump_output_csv(sector);
            break;
        case FORMAT_HTML:
            dump_output_html(sector);
            break;
        case FORMAT_TEXT:
        default:
            dump_output_text(sector);
    }//end :: switch

    free_output(sector);

}//end :: dump_output

void free_output(PEV_OUTPUT_SECTOR *sector) 
{

    PEV_OUTPUT_SECTOR *current = sector;

    while (current != NULL)
    {

        PEV_OUTPUT_LINE *current_line = current->firstLine;

        while (current_line != NULL)
        {

            PEV_OUTPUT_LINE *temp_line;
            temp_line = current_line;

            current_line = current_line->next;

            free(temp_line);

        }//end :: while
        
        PEV_OUTPUT_SECTOR *temp_sector;
        temp_sector = current;
        current = current->next;
        free(temp_sector);
    }//end :: while


}//end :: free_output
