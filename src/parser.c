#include "parser.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

xml_tag_t* xml_tag_create(const char* name, const char* data)
{
    xml_tag_t* tag = malloc(sizeof(*tag));
    if (!tag) return NULL;
    tag->name = strdup(name);
    if (!tag->name)
    {
        free(tag);
        return NULL;
    }
    tag->data = strdup(data);
    if (!tag->data)
    {
        free(tag->name);
        free(tag);
        return NULL;
    }
    tag->next = tag->prev = tag->in = tag->out = NULL;
    return tag;
}

void xml_tag_free(xml_tag_t* tag)
{
    free(tag->name);
    free(tag->data);
    free(tag);
}

static void _xml_tag_recursive_destroy(xml_tag_t* tag)
{
    assert(tag);
    if (tag->in)
        _xml_tag_recursive_destroy(tag->in);
    if (tag->next)
        _xml_tag_recursive_destroy(tag->next);
    xml_tag_free(tag);
}

void xml_tag_recursive_destroy(xml_tag_t* tag)
{
    assert(tag);
    _xml_tag_recursive_destroy(tag->in);
    tag->out->in = tag->next;
    xml_tag_free(tag);
}

xml_tag_t* xml_load_from_text(const char* data)
{
    xml_tag_t *first_tag = NULL, *current_tag = NULL, *parent_tag = NULL;
    bool in_tag = false, in_string = false;
    while (*data)
    {
        while (*data && (*data != '<' || in_string))
        {
            if (*data == '>')
                in_tag = false;
            if (*data == '\"')
                in_string ^= true;
            if (*data == '/' && in_tag && !in_string)
            {
                current_tag = parent_tag;
                if (parent_tag) parent_tag = parent_tag->out;
            }
            data++;
        }
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
            xml_tag_t* new_tag = xml_tag_create("Node", "");
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
            if (*data == '?' || *data == '!')
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
            data++;
        }
    }
    return first_tag;
}

static void _xml_print_data(xml_tag_t* tag, int depth)
{
    if (!tag) return;
    printf("%*s<%s>\n", 4 * depth, "", tag->name);
    xml_tag_t* child = tag->in;
    while (child)
    {
        _xml_print_data(child, depth + 1);
        child = child->next;
    }
    if (strcmp(tag->data, "") != 0)
        printf("%*s%s\n", 4 * depth, "", tag->data);
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
