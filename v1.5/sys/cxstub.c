/*
 * RECONSTRUCTED -- not an original UniSoft file (2026).
 *
 * slp.c, text.c and sys1.c call cxrelse() and cxtxfree(), which are
 * defined in context.c.  context.c is for an MMU with many hardware
 * contexts (it needs cxmap, USERCX, NUMUCONTX and context[], none of
 * which the Lisa headers define) and is not part of the Lisa build, so
 * linking the kernel left both symbols undefined (called as address 0).
 *
 * On the Lisa no context is ever allocated (only context.c's cxalloc()
 * sets p_context), so p_context is always 0 and text x_cxaddr is always
 * 0.  With those values the real functions in context.c do nothing:
 * cxrelse() returns at "if (cx == 0)", and cxtxfree() only calls
 * cxrelse(0) and returns at "if (xp->x_cxaddr == 0 ...)".  These stubs
 * are therefore equivalent for the Lisa.  See include/PROVENANCE.md.
 */

/* ARGSUSED */
cxrelse(cx)
char *cx;
{
}

/* ARGSUSED */
cxtxfree(xp)
char *xp;
{
}
