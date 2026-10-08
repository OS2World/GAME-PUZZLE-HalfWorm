typedef struct _PIXELBUFFER
{
   HDIVE hDive;
   LONG cx;
   LONG cy;
   ULONG cbBitmap;
   ULONG cbLine;
   ULONG ulBuffer;                       /* DIVE buffer number */
   PBYTE afLineMask;                     /* DIVE line mask information */
   BYTE data[1];
}PIXELBUFFER, *PPIXELBUFFER;


typedef struct _RLEDATA
{
   LONG xLeft;
   LONG xRight;
   LONG y;
   ULONG cb;
}RLEDATA, *PRLEDATA;

typedef struct _RLECIRCLE
{
   LONG lines; /* number of lines */
   LONG xMin;
   LONG xMax;
   LONG yMin;
   LONG yMax;
   LONG radius;
   RLEDATA line[1];
}RLECIRCLE, *PRLECIRCLE;


PPIXELBUFFER _Optlink CreatePixelBuffer(LONG cx, LONG cy);
PPIXELBUFFER _Optlink CreateDivePixelBuffer(HDIVE hDive, LONG cx, LONG cy);
BOOL _Optlink DestroyDivePixelBuffer(PPIXELBUFFER pb);

BOOL _Optlink AssocDive(HDIVE hDive, PPIXELBUFFER pb);
BOOL _Optlink DisassocDive(PPIXELBUFFER pb);

void _Inline setPixel(PPIXELBUFFER pb, PPOINTL p, BYTE color);
BYTE _Inline getPixel(PPIXELBUFFER pb, PPOINTL p);
PBYTE _Inline getPixelP(PPIXELBUFFER pb, PPOINTL p);


PRLECIRCLE generate_rlecircle_data(LONG radius);


/*
 * Inline functions
 */
void _Inline setPixel(PPIXELBUFFER pb, PPOINTL p, BYTE color)
{
   pb->data[(p->y*pb->cbLine)+p->x] = color;
   pb->afLineMask[p->y] = 0xff;
}
BYTE _Inline getPixel(PPIXELBUFFER pb, PPOINTL p)
{
   return pb->data[(p->y*pb->cbLine)+p->x];
}
PBYTE _Inline getPixelP(PPIXELBUFFER pb, PPOINTL p)
{
   return &pb->data[(p->y*pb->cbLine)+p->x];
}
