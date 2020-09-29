/* Macro that gets the contents of a location using PC relative
 * addressing, and only 16-bit relative addressing.
 *
 */


#define GET_FAR_VALUE_PC_RELATIVE(__loc__)             \
	 ({ void *__result__;                          \
	    asm(   "0: pea 0b(%%pc);"                  \
               "   addi.l #(" #__loc__ "- 0b),(%%sp);" \
			   "   move.l (%%sp)+,%0"                 \
			: "=r" (__result__) :);                \
	     __result__; })
