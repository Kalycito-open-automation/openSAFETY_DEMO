//-------------------------------------------------------------------------
// DO NOT MODIFY THIS FILE!
// This file is generated automatically depending on the ipcore settings!
// Hence, it is highly recommended to avoid manual modifications!
//
// timestamp = 2013-10-31 09:39:07
//-------------------------------------------------------------------------

#ifndef __APPIF_CFG_H__
#define __APPIF_CFG_H__

#define TBUF_OFFSET_CONACK 0
#define TBUF_SIZE_CONACK 4
#define TBUF_PORTA_ISPRODUCER_CONACK -1

#define TBUF_OFFSET0 4
#define TBUF_SIZE0 12
#define TBUF_PORTA_ISPRODUCER0 0

#define TBUF_OFFSET1 16
#define TBUF_SIZE1 12
#define TBUF_PORTA_ISPRODUCER1 0

#define TBUF_OFFSET2 28
#define TBUF_SIZE2 28
#define TBUF_PORTA_ISPRODUCER2 0

#define TBUF_OFFSET3 56
#define TBUF_SIZE3 36
#define TBUF_PORTA_ISPRODUCER3 0

#define TBUF_OFFSET4 92
#define TBUF_SIZE4 4
#define TBUF_PORTA_ISPRODUCER4 1

#define TBUF_OFFSET5 96
#define TBUF_SIZE5 12
#define TBUF_PORTA_ISPRODUCER5 1

#define TBUF_OFFSET6 108
#define TBUF_SIZE6 36
#define TBUF_PORTA_ISPRODUCER6 1

#define TBUF_OFFSET7 144
#define TBUF_SIZE7 36
#define TBUF_PORTA_ISPRODUCER7 1

#define TBUF_OFFSET_PROACK 180
#define TBUF_SIZE_PROACK 4
#define TBUF_PORTA_ISPRODUCER_PROACK -1

#define TBUF_NUM_CON 4
#define TBUF_NUM_PRO 4

#define TBUF_INIT_VEC { \
                        { TBUF_OFFSET_CONACK, TBUF_SIZE_CONACK, TBUF_PORTA_ISPRODUCER_CONACK },  \
                        { TBUF_OFFSET0, TBUF_SIZE0, TBUF_PORTA_ISPRODUCER0 },  \
                        { TBUF_OFFSET1, TBUF_SIZE1, TBUF_PORTA_ISPRODUCER1 },  \
                        { TBUF_OFFSET2, TBUF_SIZE2, TBUF_PORTA_ISPRODUCER2 },  \
                        { TBUF_OFFSET3, TBUF_SIZE3, TBUF_PORTA_ISPRODUCER3 },  \
                        { TBUF_OFFSET4, TBUF_SIZE4, TBUF_PORTA_ISPRODUCER4 },  \
                        { TBUF_OFFSET5, TBUF_SIZE5, TBUF_PORTA_ISPRODUCER5 },  \
                        { TBUF_OFFSET6, TBUF_SIZE6, TBUF_PORTA_ISPRODUCER6 },  \
                        { TBUF_OFFSET7, TBUF_SIZE7, TBUF_PORTA_ISPRODUCER7 },  \
                        { TBUF_OFFSET_PROACK, TBUF_SIZE_PROACK, TBUF_PORTA_ISPRODUCER_PROACK } \
                      }

#endif
