#ifndef _DEVICETREE_H
#define _DEVICETREE_H

#include "of_intern.h"

of_node_t * dt_parse(void *dt);
of_node_t *dt_find_node_by_phandle(unsigned int phandle);
of_node_t *dt_find_node(char *key);
of_property_t *dt_find_property(void *key, char *propname);
void *dt_get_prop_value(of_property_t *);

#endif /* _DEVICETREE_H */
