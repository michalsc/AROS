#include <aros/macros.h>
#include <exec/lists.h>
#include <stdint.h>
#include "devicetree.h"

#define D(x)

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct fdt_reserve_entry {
    uint64_t address;
    uint64_t size;
};

struct fdt_prop_entry {
    uint32_t len;
    uint32_t nameoffset;
};

#define FDT_END         0x00000009
#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004

#define FDT_MAGIC       0xd00dfeed

static uint32_t *data;
static char *strings;
static of_node_t *root = NULL;
static unsigned char space[65536];
static unsigned long _free;
static void * memptr;
static struct fdt_header *hdr;

static void *_malloc(int size)
{
    if (size <= _free)
    {
        void *ptr = memptr;
        memptr += size;
        _free -= size;
        return ptr;
    }
    else return NULL;
}

static int _strlen(const char *str)
{
    int len = 0;

    while(*str++) len++;

    return len;
}

static int _strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return 0;
	return *(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1);
}

static of_node_t * dt_build_node(of_node_t *parent)
{
    of_node_t *e = _malloc(sizeof(of_node_t));

    if (e != NULL)
    {
        e->on_parent = parent;
        if (parent != NULL)
        {
            ADDTAIL(&parent->on_children, e);
        }
        NEWLIST(&e->on_children);
        NEWLIST(&e->on_properties);
        e->on_name = (char *)data;
        data += (_strlen((char *)data) + 4) / 4;

        D(kprintf("[BOOT] new node %s\n", e->on_name));

        while(1)
        {
            switch (AROS_BE2LONG(*data++))
            {
                case FDT_BEGIN_NODE:
                {
                    dt_build_node(e);
                    break;
                }

                case FDT_PROP:
                {
                    of_property_t *p = _malloc(sizeof(of_property_t));
                    p->op_length = AROS_BE2LONG(*data++);
                    p->op_name = &strings[AROS_BE2LONG(*data++)];
                    if (p->op_length)
                        p->op_value = data;
                    else
                        p->op_value = NULL;
                    ADDTAIL(&e->on_properties, p);
                    data += (p->op_length + 3)/4;
                    D(kprintf("[BOOT] prop %s with length %d\n", p->op_name, p->op_length));
                    break;
                }

                case FDT_NOP:
                    break;

                case FDT_END_NODE:
                    return e;
            }
        }
    }
    return e;
}

of_node_t * dt_parse(void *dt)
{
    memptr = space;
    _free = sizeof(space);

    uint32_t token = 0;
    hdr = dt;

    if (hdr->magic == AROS_LONG2BE(FDT_MAGIC))
    {
        strings = dt + AROS_BE2LONG(hdr->off_dt_strings);
        data = dt + AROS_BE2LONG(hdr->off_dt_struct);

        if (hdr->off_mem_rsvmap)
        {
            struct fdt_reserve_entry *rsrvd = dt + AROS_BE2LONG(hdr->off_mem_rsvmap);

            while (rsrvd->address != 0 || rsrvd->size != 0) {
                rsrvd++;
            }
        }

        do
        {
            token = AROS_BE2LONG(*data++);

            switch (token)
            {
                case FDT_BEGIN_NODE:
                    root = dt_build_node(NULL);
                    break;
                case FDT_PROP:
                {
                    D(kprintf("[BOOT] Property outside root node?"));
                    break;
                }
            }
        } while (token != FDT_END);
    }
    else
    {
        hdr = NULL;
    }

    return root;
}

of_property_t *dt_find_property(void *key, char *propname)
{
    of_node_t *node = (of_node_t *)key;
    of_property_t *p, *prop = NULL;

    if (node)
    {
        ForeachNode(&node->on_properties, p)
        {
            if (!_strcmp(p->op_name, propname))
            {
                prop = p;
                break;
            }
        }
    }
    return prop;
}

void *dt_get_prop_value(of_property_t *prop)
{
    if (prop)
    {
        return prop->op_value;
    }
    return NULL;
}

#define MAX_KEY_SIZE    64
char ptrbuf[64];

of_node_t * dt_find_node(char *key)
{
    int i;
    of_node_t *node, *ret = NULL;

    if (*key == '/')
    {
        ret = root;

        while(*key)
        {
            key++;
            for (i=0; i < 63; i++)
            {
                if (*key == '/' || *key == 0)
                    break;
                ptrbuf[i] = *key;
                key++;
            }

            ptrbuf[i] = 0;

            ForeachNode(&ret->on_children, node)
            {
                if (!_strcmp(node->on_name, ptrbuf))
                {
                    ret = node;
                    break;
                }
            }
        }
    }

    return ret;
}

static of_node_t * dt_find_by_phandle(uint32_t phandle, of_node_t *root)
{
    of_property_t *p = dt_find_property(root, "phandle");

    if (p && *((uint32_t *)p->op_value) == AROS_LONG2BE(phandle))
        return root;
    else {
        of_node_t *c;
        ForeachNode(&root->on_children, c)
        {
            of_node_t *found = dt_find_by_phandle(phandle, c);
            if (found)
                return found;
        }
    }
    return NULL;
}

of_node_t * dt_find_node_by_phandle(uint32_t phandle)
{
    return dt_find_by_phandle(phandle, root);
}
