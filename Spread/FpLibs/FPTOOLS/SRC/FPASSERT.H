#ifndef FPASSERT_H
#define FPASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _DEBUG
   void PASCAL fp_assert (LPSTR, LPSTR, unsigned);
   #define assert(exp) \
       ( (exp) ? (void) 0 : fp_assert(#exp, __FILE__, __LINE__) )
#else
   #define assert(exp)
#endif

#ifdef __cplusplus
}
#endif    

#endif // FPASSERT_H

