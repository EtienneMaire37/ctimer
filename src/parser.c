#include "parser.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    return xml_tag_create("FirstNode", "aaa");
}

static void _xml_print_data(xml_tag_t* tag, int depth)
{
    if (!tag) return;
    printf("%.*s<%s>", 4 * depth, "", tag->name);
    xml_tag_t* child = tag->in;
    while (child)
    {
        _xml_print_data(child, depth + 1);
        child = child->next;
    }
    printf("%.*s%s", 4 * depth, "", tag->data);
    printf("%.*s</%s>", 4 * depth, "", tag->name);
}

void xml_print_data(xml_tag_t* first_tag)
{
    while (first_tag)
    {
        _xml_print_data(first_tag, 0);
        first_tag = first_tag->next;
    }
}
