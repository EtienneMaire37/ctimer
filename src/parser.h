#pragma once

typedef struct xml_tag xml_tag_t;
typedef struct xml_tag
{
    char* name;
    xml_tag_t *next, *prev, *in, *out;
    char* data;
} xml_tag_t;

void xml_tag_free(xml_tag_t* tag);
void xml_tag_recursive_destroy(xml_tag_t* tag);
xml_tag_t* xml_load_from_text(const char* data);
void xml_print_data(xml_tag_t* first_tag);
