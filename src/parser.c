#include "parser.h"
#include <assert.h>
#include <stdlib.h>

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
