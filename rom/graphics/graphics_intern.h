#ifndef GRAPHICS_INTERN_H
#define GRAPHICS_INTERN_H
/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Internal header file for graphics.library
    Lang: english
*/
#ifndef AROS_LIBCALL_H
#   include <aros/libcall.h>
#endif
#ifndef EXEC_EXECBASE_H
#   include <exec/execbase.h>
#endif
#ifndef GRAPHICS_GFXBASE_H
#   include <graphics/gfxbase.h>
#endif
#ifndef GRAPHICS_TEXT_H
#   include <graphics/text.h>
#endif
#ifndef GRAPHICS_RASTPORT_H
#   include <graphics/rastport.h>
#endif

extern struct GfxBase * GfxBase;

/* Macros */
#define RASSIZE(w,h)    ((ULONG)(h)*( ((ULONG)(w)+15)>>3&0xFFFE))

/* Defines */
#define BMT_STANDARD	0x0000	/* Standard bitmap */
#define BMT_RGB 	0x1234	/* RTG Bitmap. 24bit RGB chunky */
#define BMT_RGBA	0x1238	/* RTG Bitmap. 32bit RGBA chunky */
#define BMT_DRIVER	0x8000	/* Special RTG bitmap.
				   Use this as an offset. */

/* Forward declaration */
struct ViewPort;

#ifdef SysBase
#undef SysBase
#endif
#define SysBase ((struct ExecBase *)(GfxBase->ExecBase))
#ifdef UtilityBase
#undef UtilityBase
#endif
#define UtilityBase ((struct Library *)(GfxBase->UtilBase))

/* Needed for close() */
#define expunge() \
    AROS_LC0(BPTR, expunge, struct GfxBase *, GfxBase, 3, Gfx)

/* Driver prototypes */
extern struct BitMap * driver_AllocBitMap (ULONG, ULONG, ULONG, ULONG,
			struct BitMap *, struct GfxBase *);
extern LONG driver_BltBitMap ( struct BitMap * srcBitMap, LONG xSrc,
			LONG ySrc, struct BitMap * destBitMap, LONG xDest,
			LONG yDest, LONG xSize, LONG ySize, ULONG minterm,
			ULONG mask, PLANEPTR tempA, struct GfxBase *);
extern int driver_CloneRastPort (struct RastPort *, struct RastPort *,
			struct GfxBase *);
extern void driver_CloseFont (struct TextFont *, struct GfxBase *);
extern void driver_DeinitRastPort (struct RastPort *, struct GfxBase *);
extern void driver_Draw (struct RastPort *, LONG, LONG, struct GfxBase *);
extern void driver_DrawEllipse (struct RastPort *, LONG x, LONG y,
			LONG rx, LONG ry, struct GfxBase *);
extern void driver_EraseRect (struct RastPort *, LONG, LONG, LONG, LONG,
			    struct GfxBase *);
extern void driver_FreeBitMap (struct BitMap *, struct GfxBase *);
extern int  driver_InitRastPort (struct RastPort *, struct GfxBase *);
extern void driver_LoadRGB4 (struct ViewPort * vp, UWORD * colors, LONG count, struct GfxBase *);
extern void driver_LoadRGB32 (struct ViewPort * vp, ULONG * table, struct GfxBase *);
extern void driver_Move (struct RastPort *, LONG, LONG, struct GfxBase *);
extern struct TextFont * driver_OpenFont (struct TextAttr *,
			    struct GfxBase *);
extern void driver_PolyDraw (struct RastPort *, LONG, WORD *,
			    struct GfxBase *);
extern ULONG driver_ReadPixel (struct RastPort *, LONG, LONG,
			    struct GfxBase *);
extern void driver_RectFill (struct RastPort *, LONG, LONG, LONG, LONG,
			    struct GfxBase *);
extern void driver_ScrollRaster (struct RastPort *,
			    LONG, LONG, LONG, LONG, LONG, LONG,
			    struct GfxBase *);
extern void driver_SetABPenDrMd (struct RastPort *, ULONG, ULONG, ULONG,
			    struct GfxBase * GfxBase);
extern void driver_SetAPen (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetBPen (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetDrMd (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetFont (struct RastPort *, struct TextFont *,
			    struct GfxBase *);
extern void driver_SetOutlinePen (struct RastPort *, ULONG, struct GfxBase *);
extern ULONG driver_SetWriteMask (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetRast (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_Text (struct RastPort *, STRPTR, LONG, struct GfxBase *);
extern WORD driver_TextLength (struct RastPort *, STRPTR, ULONG,
			    struct GfxBase *);
extern LONG driver_WritePixel (struct RastPort *, LONG, LONG,
			    struct GfxBase *);
extern void driver_WaitTOF (struct GfxBase *);

#endif /* GRAPHICS_INTERN_H */
