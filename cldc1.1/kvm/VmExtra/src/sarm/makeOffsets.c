/*
 * Copyright (c) 2001 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Build
 * FILE:      makeOffset.c
 * OVERVIEW:  This file generates a table of constants and offsets
 *            that is used in building the fast arm assembly-language
              interpreter.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/



#include <global.h>
#include <stdarg.h>

typedef struct GlobalStateStruct *GLOBALSTATE;


static void printConstants(void);
static void printByteCodes(void);
static void printStructures(void);

static void printStruct(const char*, const char*, long value, long offset, 
                        long structSize, bool_t isArray);

static void 
getNth(int multiplier, int offset, char *getter, 
       char *result, char *base, char *index);

int main(int argc, char **argv) {     
    printConstants();
    printByteCodes();
    printStructures();
    return 0;
}


#define PRINT_CONSTANT(A) printf("#define " #A " 0x%lx\n", (long)A);

/* Print constants needed by the assembly language interpreter */
static void printConstants() {
    PRINT_CONSTANT(KILLTHREAD);
    PRINT_CONSTANT(CLASS_READY);
    PRINT_CONSTANT(RESERVEDFORNATIVE);
    printf("\n");    
    PRINT_CONSTANT(ACC_PUBLIC);
    PRINT_CONSTANT(ACC_NATIVE);
    PRINT_CONSTANT(ACC_ABSTRACT);
    PRINT_CONSTANT(ACC_STATIC);
    PRINT_CONSTANT(ACC_SYNCHRONIZED);
    printf("\n");    
    PRINT_CONSTANT(MHC_UNLOCKED);
    PRINT_CONSTANT(MHC_SIMPLE_LOCK);
    PRINT_CONSTANT(MHC_EXTENDED_LOCK);
    PRINT_CONSTANT(MHC_MONITOR);
    PRINT_CONSTANT(MonitorStatusError);
    printf("\n");
    PRINT_CONSTANT(GCT_INSTANCE);
}

/* Print the byte codes.  The byte codes will be printed 
 * form:
 *   #define FOR_EACH_BYTE_CODE(_macro_)    \
 *       FOR_EACH_NORMAL_BYTE_CODE(_macro_) \
 *       FOR_EACH_EXTRA_BYTE_CODE(_macro_)\n
 *
 *   #define FOR_EACH_NORMAL_BYTE_CODE(_macro_) \
 *        _macro(name, hexValue) 
 *         ......
 *   #define FOR_EACH_EXTRA_BYTE_CODE(_macro_) \
 *        _macro(name, hexValue) 
 * 
 * Where the "normal" byte codes are the normal "in-range" byte codes
 * and the "fast" byte codes.  The extra byte codes are the 32 or so
 * unused codes at the end.
 */


void printByteCodes() {
    static const char *byteCodes[] = BYTE_CODE_NAMES;
    int i;

    printf("#define FOR_EACH_BYTE_CODE(_macro_)    \\\n");
    printf("    FOR_EACH_NORMAL_BYTE_CODE(_macro_) \\\n");
    printf("    FOR_EACH_EXTRA_BYTE_CODE(_macro_)\n");
    printf("\n");
    printf("#define FOR_EACH_NORMAL_BYTE_CODE(_macro_) \\\n");
    for (i = 0; i < sizeof(byteCodes)/sizeof(byteCodes[0]); i++) { 
        printf("        _macro_(%s, 0x%x) \\\n", byteCodes[i], i);
    }
    printf("\n");
    printf("#define FOR_EACH_EXTRA_BYTE_CODE(_macro_) \\\n");
    for ( ; i < 256; i++) {
        printf("        _macro_(UNUSED_%X, 0x%x) \\\n", i, i);
    }
    printf("\n");
    for (i = 0; i < sizeof(byteCodes)/sizeof(byteCodes[0]); i++) { 
        printf("#define OP_%s 0x%x\n", byteCodes[i], i);
    }
    printf("\n\n");
}


/* Generate code to get or set various fields of a structure.
 * This has been heavily macro-ized to make it simple to add 
 * new structures and new fields.
 */
static 
void printStructures() { 
#  define INIT_STRUCT(ptype)  { \
        ptype sampleStructure;   \
        int structSize = sizeof(*sampleStructure); \
        const char *structName = #ptype; \
        bool_t isArray = FALSE;  \
        sampleStructure = (ptype)malloc(structSize); \
	memset(sampleStructure, 0x80, structSize);  \
        printf("#define SIZEOF_%s_bytes %ld\n", structName, (long)structSize); \
        printf("#define SIZEOF_%s_cells %ld\n", structName, \
                           (long)ByteSizeToCellSize(structSize));

#  define END_STRUCT }

#  define PRINT_STRUCT(fieldName, field)             \
        printStruct(structName,   fieldName,         \
                    (long)sampleStructure->field,     \
                    (char *)&sampleStructure->field - (char *)sampleStructure, \
                    structSize, isArray);

    INIT_STRUCT(ARRAY) 
	PRINT_STRUCT("ofClass", ofClass);
	PRINT_STRUCT("length",  length);
	PRINT_STRUCT("data",    data[0].cellp);
    END_STRUCT

    INIT_STRUCT(INSTANCE)
	PRINT_STRUCT("ofClass",   ofClass);
        PRINT_STRUCT("hashCode",  mhc.hashCode);    
        PRINT_STRUCT("monitor",   mhc.address);    
	PRINT_STRUCT("data",      data[0].cellp);
    END_STRUCT

    INIT_STRUCT(FRAME)
	PRINT_STRUCT("thisMethod", thisMethod);
	PRINT_STRUCT("previousFp", previousFp);
	PRINT_STRUCT("previousSp", previousSp);
	PRINT_STRUCT("previousIp", previousIp);
	PRINT_STRUCT("syncObject", syncObject);
	PRINT_STRUCT("stack",      stack);
    END_STRUCT

    INIT_STRUCT(METHOD)
	PRINT_STRUCT("nameTypeKey", nameTypeKey.i);
	PRINT_STRUCT("accessFlags", accessFlags);
	PRINT_STRUCT("ofClass",     ofClass);
	PRINT_STRUCT("frameSize",   frameSize);
	PRINT_STRUCT("argCount",    argCount);
	PRINT_STRUCT("maxStack",    u.java.maxStack);
	PRINT_STRUCT("javaCode",    u.java.code);
    END_STRUCT

    INIT_STRUCT(FIELD)
	PRINT_STRUCT("nameTypeKey",   nameTypeKey.i);
	PRINT_STRUCT("accessFlags",   accessFlags);
	PRINT_STRUCT("ofClass",       ofClass);
	PRINT_STRUCT("offset",        u.offset);
	PRINT_STRUCT("staticAddress", u.staticAddress);
    END_STRUCT

    INIT_STRUCT(INSTANCE_CLASS)
	PRINT_STRUCT("constPool",  constPool);
	PRINT_STRUCT("status",     status);
	PRINT_STRUCT("initThread", initThread);
	PRINT_STRUCT("instSize",   instSize);
    END_STRUCT

    INIT_STRUCT(ARRAY_CLASS)
	PRINT_STRUCT("elemClass",  u.elemClass);
    END_STRUCT

    INIT_STRUCT(ICACHE)
        /* Is array causes GET...._nth macros to be created
         * to access the nth element of the array */
        isArray = TRUE;
	PRINT_STRUCT("contents", contents);
        PRINT_STRUCT("codeLoc",  codeLoc);
    END_STRUCT

    INIT_STRUCT(GLOBALSTATE)
	PRINT_STRUCT("ip", gs_ip);
	PRINT_STRUCT("fp", gs_fp);
	PRINT_STRUCT("cp", gs_cp);
	PRINT_STRUCT("lp", gs_lp);
	PRINT_STRUCT("sp", gs_sp);
    END_STRUCT
        
    INIT_STRUCT(STACK)
        PRINT_STRUCT("size",   size);
        PRINT_STRUCT("cells",  cells);
    END_STRUCT

}

/* This is called by PRINT_STRUCT above;
 *    name:       name of structure
 *    field:      name of field in the structure
 *    value:      The value we read when we write 80808080.. to the field
 *    offset:     Offset of this field with the structure
 *    structSize: Size of the structure
 *    isArray:    Also generate GET......nth macros.
 */
static void printStruct(const char* name, const char* field, 
			long value, long offset, long structSize, 
                        bool_t isArray) 
{ 
    char *getter;
    char *setter;
    char *cc = "##cc##";
    switch(value) { 
    case 0x80808080:
        /* 4-byte field */
	getter = setter = "";  cc = "##cc"; break;
    case 0x8080:
        /* 2-byte unsigned field */
	getter = setter = "h"; break;
    case 0x80:
        /* 1-byte unsigned field */
	getter = setter = "b"; break;
    case 0xFFFF8080:
        /* 2-byte signed field */
	getter = "sh"; setter = "h"; break;
	break;
    case 0xFFFFFF80:
        /* 1-byte signed field */
	getter = "sb"; setter = "b"; break;
    default:
	getter = "xx"; setter = "xx"; break;

    }
    printf("#define GET_%s_%s(a, b) ldr%s a, [b, $%ld]\n",
	   name, field, getter, offset);
    printf("#define SET_%s_%s(a, b) str%s a, [b, $%ld]\n",
	   name, field, setter, offset);
    printf("#define GET_%s_%s_IF(cc, a, b) ldr%s%s a, [b, $%ld]\n",
	   name, field, cc, getter, offset); 
    printf("#define SET_%s_%s_IF(cc, a, b) str%s%s a, [b, $%ld]\n",
	   name, field, cc, setter, offset);
    if (isArray) { 
        /* Generate "multiply and add" code that accesses the nth
         * field
         */
        int shift = 0;
        int multiplier = structSize;
        char *result = "a";
        char *base   = "b";
        char *index  = "n";
        printf("#define GET_%s_%s_nth(%s, %s, %s) \\\n", name, field, 
               result, base, index);
        for (;;) { 
            while ((multiplier & 0x1) == 0) { 
                multiplier >>= 1;
                shift++;
            }
            /* The multiplier is now odd */
            if (multiplier == 1 && offset == 0) { 
                printf("\tldr%s %s, [%s, %s, lsl $%d]\n", 
                       getter, result, base, index, shift);
                break;
            } else { 
                if ((multiplier & 0x3) == 3) { 
                    printf("\tsub %s, %s, %s, lsl $%d; \\\n", 
                           result, base, index, shift);
                    multiplier++;
                } else { 
                    printf("\tadd %s, %s, %s, lsl $%d; \\\n", 
                           result, base, index, shift);
                    multiplier--;
                }
                base = result;

            }
            if (multiplier == 0) { 
                printf("\tldr%s %s, [%s, $%d]\n", 
                       getter, result, base, offset);
                break;
            }
        }
    }

    if (isArray) { 
        /* Generate "multiply and add" code that accesses the nth
         * field
         */
        int shift = 0;
        int multiplier = structSize;
        char *result = "a";
        char *base   = "b";
        char *index  = "n";
        char *location = "loc";

        printf("#define GET_%s_%s_nthX(%s, %s, %s, %s) \\\n", 
               name, field, result, location, base, index);
        for (;;) { 
            while ((multiplier & 0x1) == 0) { 
                multiplier >>= 1;
                shift++;
            }
            /* The multiplier is now odd */
            if (multiplier == 1 && offset == 0) { 
                printf("\tldr%s %s, [%s, %s, lsl $%d]; \\\n", 
                       getter, result, base, index, shift);
                printf("\tadd %s, %s, %s, lsl $%d\n", 
                       location, base, index, shift);
                break;
            } else { 
                if ((multiplier & 0x3) == 3) { 
                    printf("\tsub %s, %s, %s, lsl $%d; \\\n", 
                           location, base, index, shift);
                    multiplier++;
                } else { 
                    printf("\tadd %s, %s, %s, lsl $%d; \\\n", 
                           location, base, index, shift);
                    multiplier--;
                }
                base = location;
            }
            if (multiplier == 0) { 
                if (base != location) { 
                    printf("\tmov %s, %s\\\n", location, base);
                }
                printf("\tldr%s %s, [%s, $%d]; \n", 
                       getter, result, location, offset);
                break;
            }
        }
    }








    printf("#define %s_%s_OFFSET %ld\n", name, field, offset);
    printf("\n\n");
}       
