#pragma strings(readonly)

#define INCL_OS2MM


#include <os2.h>
#include <os2me.h>

#include <dive.h>
#include <fourcc.h>

#include <memory.h>
#include <malloc.h>
#include <stdlib.h>


#include "graphlib.h"

#include "debug.h"


ULONG _Inline calc_bitmap_size(LONG cx, LONG cy, ULONG cBitCount, ULONG cPlanes);


PPIXELBUFFER _Optlink CreatePixelBuffer(LONG cx, LONG cy)
{
   PPIXELBUFFER pb = NULL;
   ULONG cbBitmap = calc_bitmap_size(cx, cy, 8, 1);
   ULONG cbAlloc = sizeof(PIXELBUFFER)-1+cbBitmap;

   if((pb = malloc(cbAlloc)) != NULL)
   {
      memset(pb, 0x00, cbAlloc);
      pb->cx = cx;
      pb->cy = cy;
      pb->cbBitmap = cbBitmap;
      pb->cbLine = calc_bitmap_size(cx, 1, 8, 1);
      if((pb->afLineMask = calloc(cy, sizeof(BYTE))) != NULL)
      {
         memset(pb->afLineMask, 0x00, cy*sizeof(BYTE));
      }
   }
   return pb;
}

PPIXELBUFFER _Optlink CreateDivePixelBuffer(HDIVE hDive, LONG cx, LONG cy)
{
   PPIXELBUFFER pb = NULL;
   PPIXELBUFFER pbTemp = NULL;

   pbTemp = CreatePixelBuffer(cx, cy);
   if(pbTemp)
   {
      pbTemp->hDive = hDive;
      if(AssocDive(hDive, pbTemp))
      {
         pb = pbTemp;
      }
      else
      {
         free(pbTemp);
         _heapmin();
      }
   }
   return pb;
}

BOOL _Optlink DestroyDivePixelBuffer(PPIXELBUFFER pb)
{
   DisassocDive(pb);
   free(pb);
   return TRUE;
}

BOOL _Optlink AssocDive(HDIVE hDive, PPIXELBUFFER pb)
{
   ULONG rc = DIVE_SUCCESS;

   if((rc = DiveAllocImageBuffer(hDive, &pb->ulBuffer, FOURCC_LUT8, pb->cx, pb->cy, pb->cbLine, pb->data)) != DIVE_SUCCESS)
   {
      dprintf(("e %s: DiveAllocImageBuffer() returned %08x (%u).\n", __FUNCTION__, rc, rc));
   }
   return rc == DIVE_SUCCESS;
}

BOOL _Optlink DisassocDive(PPIXELBUFFER pb)
{
   ULONG rc = DIVE_SUCCESS;

   if((rc = DiveFreeImageBuffer(pb->hDive, pb->ulBuffer)) != DIVE_SUCCESS)
   {
      dprintf(("e %s: DiveFreeImageBuffer() returned %08x (%u).\n", __FUNCTION__, rc, rc));
   }
   return rc == DIVE_SUCCESS;
}



BOOL _Inline is_within_radius(PPOINTL center, PPOINTL p, LONG radius)
{
   LONG dx = center->x - p->x;
   LONG dy = center->y - p->y;

   if( ((dx*dx) + (dy*dy)) <= (radius*radius) )
   {
      return TRUE;
   }
   return FALSE;
}


PRLECIRCLE generate_rlecircle_data(LONG radius)
{
   typedef struct _LISTNODE
   {
      struct _LISTNODE *next;
      RLEDATA data;
   }LISTNODE, *PLISTNODE;
   PRLECIRCLE prc = NULL;
   POINTL ptlCenter = { 0, 0 };
   POINTL p = { 0, 0 };
   PLISTNODE firstnode = NULL;
   PLISTNODE node = NULL;
   ULONG nodes = 0;
   ULONG cbAlloc = 0;
   int i = 0;

   for(p.y = -(radius+1); p.y < (radius+1); p.y++)
   {
      BOOL fNewLine = TRUE;
      for(p.x = -(radius+2); p.x < (radius+2); p.x++)
      {
         if(fNewLine)
         {
            if(!is_within_radius(&ptlCenter, &p, radius))
            {
               continue;
            }
            fNewLine = FALSE;
            if(node)
            {
               node->next = malloc(sizeof(LISTNODE));
               node = node->next;
               memset(node, 0, sizeof(LISTNODE));
            }
            else
            {
               node = firstnode = malloc(sizeof(LISTNODE));
               memset(node, 0, sizeof(LISTNODE));
            }
            nodes++;
            node->data.y = p.y;
            node->data.xLeft = p.x;
            continue;
         }

         if(!is_within_radius(&ptlCenter, &p, radius))
         {
            node->data.xRight = p.x;
            node->data.cb = (node->data.xRight-node->data.xLeft)+1;
            p.x = (radius+2);
            continue;
         }
      }
   }

   /*
    * Allocate the drawing data
    */
   cbAlloc = sizeof(RLECIRCLE) + (nodes-1)*sizeof(RLEDATA);
   if((prc = malloc(cbAlloc)) != NULL)
   {
      memset(prc, 0, cbAlloc);
   }

   /*
    * Transfer list to array
    */
   i = 0;
   prc->lines = nodes;
   prc->radius = radius;
   dprintf(("Lines: %d\n", prc->lines));
   while(firstnode)
   {
      prc->xMin = min(prc->xMin, firstnode->data.xLeft);
      prc->xMax = max(prc->xMax, firstnode->data.xRight);
      prc->yMin = min(prc->yMin, firstnode->data.y);
      prc->yMax = max(prc->yMax, firstnode->data.y);

      memcpy(&prc->line[i++], &firstnode->data, sizeof(RLEDATA));
      node = firstnode->next;
      free(firstnode);
      firstnode = node;
   }

   _heapmin();

   return prc;
}


ULONG _Inline calc_bitmap_size(LONG cx, LONG cy, ULONG cBitCount, ULONG cPlanes)
{
   return (((cBitCount * cx) + 31) / 32) * 4 * cy * cPlanes;
}

