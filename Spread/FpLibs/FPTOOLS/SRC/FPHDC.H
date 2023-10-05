#ifdef UREGIONAL
   #include "clipping.h"
   #define fpGetDC UGetDC
   #define fpGetWindowDC UGetWindowDC
   #define fpBeginPaint UBeginPaint
#else
   #define fpGetDC GetDC
   #define fpGetWindowDC GetWindowDC
   #define fpBeginPaint BeginPaint
#endif
