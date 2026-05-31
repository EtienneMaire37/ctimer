#include "parser.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

xml_tag_t* xml_tag_create_from_buffers(const char* name, size_t name_len, const char* data, size_t data_len)
{
    xml_tag_t* tag = malloc(sizeof(*tag));
    if (!tag) return NULL;
    tag->name = malloc(name_len + 1);
    if (!tag->name)
    {
        free(tag);
        return NULL;
    }
    memcpy(tag->name, name, name_len);
    tag->name[name_len] = 0;
    tag->data = malloc(data_len + 1);
    if (!tag->data)
    {
        free(tag->name);
        free(tag);
        return NULL;
    }
    memcpy(tag->data, data, data_len);
    tag->data[data_len] = 0;
    tag->next = tag->prev = tag->in = tag->out = NULL;
    return tag;
}

void xml_tag_free(xml_tag_t* tag)
{
    if (!tag) return;
    free(tag->name);
    free(tag->data);
    free(tag);
}

static void _xml_tag_recursive_destroy(xml_tag_t* tag)
{
    if (!tag) return;
    _xml_tag_recursive_destroy(tag->in);
    _xml_tag_recursive_destroy(tag->next);
    xml_tag_free(tag);
}

void xml_tag_recursive_destroy(xml_tag_t* tag)
{
    if (!tag) return;
    _xml_tag_recursive_destroy(tag->in);
    if (tag->out) tag->out->in = tag->next;
    xml_tag_free(tag);
}

xml_tag_t* xml_load_from_text(const char* data)
{
    xml_tag_t *first_tag = NULL, *current_tag = NULL, *parent_tag = NULL;
    bool in_tag = false, in_string = false;
    while (*data)
    {
        while (*data && *data != '<')
            data++;
        if (!*data) break;
        in_tag = true;
        in_string = false;
        data++;
        if (!*data) break;
        while (*data && isspace(*data))
            data++;
        if (!*data) break;
        if (*data == '/')
        {
            current_tag = parent_tag;
            if (parent_tag) parent_tag = parent_tag->out;
            data++;
        }
        else
        {
            bool single_tag = false;
            const char* node_name = data++;
            char first_char = *node_name;
            while (*data && !isspace(*data) && *data != '>' && (first_char == '!' || *data != '/'))
                data++;
            if (!*data) break;
            size_t node_name_len = (uintptr_t)data - (uintptr_t)node_name;
            while (*data && (*data != '>' || in_string))
            {
                if (*data == '>')
                    in_tag = false;
                if (*data == '\"')
                    in_string ^= true;
                if (*data == '/' && in_tag && !in_string)
                    single_tag = true;
                data++;
            }
            if (!*data) break;
            data++;
            const char* node_data = data;
            while (*data && *data != '<')
                data++;
            if (!*data) break;
            size_t node_data_len = (uintptr_t)data - (uintptr_t)node_data;
            xml_tag_t* new_tag = xml_tag_create_from_buffers(node_name, node_name_len, node_data, node_data_len);
            assert(new_tag);
            if (current_tag)
            {
                current_tag->next = new_tag;
                new_tag->prev = current_tag;
            }
            else if (parent_tag)
                parent_tag->in = new_tag;
            new_tag->out = parent_tag;
            if (!first_tag)
                first_tag = new_tag;
            if (first_char == '?' || first_char == '!' || single_tag)
            {
                current_tag = new_tag;
                in_tag = false;
                in_string = false;
            }
            else
            {
                parent_tag = new_tag;
                current_tag = NULL;
            }
        }
    }
    return first_tag;
}

static void _xml_print_data(xml_tag_t* tag, int depth)
{
    if (!tag) return;
    printf("%*s<%s", 4 * depth, "", tag->name);
    xml_tag_t* child = tag->in;
    printf(">");
    while (child)
    {
        _xml_print_data(child, depth + 1);
        child = child->next;
    }
    if (strcmp(tag->data, "") != 0)
        printf("%*s%s", 4 * depth, "", tag->data);
    printf("%*s</%s>\n", 4 * depth, "", tag->name);
}

void xml_print_data(xml_tag_t* first_tag)
{
    while (first_tag)
    {
        _xml_print_data(first_tag, 0);
        first_tag = first_tag->next;
    }
}
