/*
 * i2c-sam440.h
 *
 *  Created on: Feb 1, 2010
 *      Author: misc
 */

#ifndef I2CSAM440_H_
#define I2CSAM440_H_

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/nodes.h>
#include <exec/lists.h>

#include <dos/bptr.h>

#include <oop/oop.h>

#include <aros/arossupportbase.h>
#include <exec/execbase.h>

#include <asm/amcc440.h>

struct i2c440base {
    struct Library i2c_LibNode;
    OOP_Class *		i2c_DrvClass;
};

#define METHOD(base, id, name) \
  base ## __ ## id ## __ ## name (OOP_Class *cl, OOP_Object *o, struct p ## id ## _ ## name *msg)

#define METHOD_NAME(base, id, name) \
  base ## __ ## id ## __ ## name

#define METHOD_NAME_S(base, id, name) \
  # base "__" # id "__" # name

#define BASE(lib) ((struct pcibase*)(lib))

#endif /* I2CSAM440_H_ */
