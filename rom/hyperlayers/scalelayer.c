/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <proto/exec.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <layers_intern.h>
#include <aros/libcall.h>
#include <proto/graphics.h>
#include "basicfuncs.h"

/*****************************************************************************

    NAME */
#include <proto/layers.h>
	AROS_LH6(ULONG, ScaleLayer,

/*  SYNOPSIS */
	AROS_LHA(struct Layer  *, l          , A0),
	AROS_LHA(ULONG          , factor     , D0),
	AROS_LHA(ULONG          , numerator  , D1),
	AROS_LHA(ULONG          , denominator, D2),
	AROS_LHA(ULONG          , centerx    , D3),
	AROS_LHA(ULONG          , centery    , D4),

/*  LOCATION */
	struct LayersBase *, LayersBase, 42, Layers)

/*  FUNCTION
        Scale the given layer. This function will use the
        current shape of the layer and resize it aaccording to
        the parameters factor, numerator and denominator.
        See graphics/ScalerDiv() on how to use these parameters.
        centerx and centery are used as center for the scaling
        operation. If you want to keep the upper left corner
        of the layer fixed then use that corner as centerx/centery.

    INPUTS
       L           - pointer to layer 
       factor      - a number in the range of 0..16383
       numerator   - a number in the range of 1..16383
       denominator - a number in the ranfe of 1..16383

    RESULT
       TRUE if everything went alright, FALSE otherwise
  
    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  return FALSE;

  AROS_LIBFUNC_EXIT
} /* ScaleLayer */
