/*	@(#)assert.h UniPlus+ v.0.1.1 */
/*	@(#)assert.h	1.4	*/
#ifdef NDEBUG
#define assert(EX)
#else
extern void _assert();
#define assert(EX) if (EX) ; else _assert("EX", __FILE__, __LINE__)
#endif
