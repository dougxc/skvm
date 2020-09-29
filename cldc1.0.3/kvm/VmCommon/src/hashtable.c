/*
 * Copyright (c) 1998-2001 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: Internal hashtables
 * FILE:      hashtable.c
 * OVERVIEW:  Maintains all the hashtables in the system:
 *               ClassTable:        Mapping from char* to name
 *               InternStringTable: Mapping from char* to String
 *               UTFStringTable:    Mapping from char* to unique char*
 *            
 * AUTHOR:    Frank Yellin
 * NOTE:      See hashtable.h for further information.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Global variables
 *=======================================================================*/

#if ROMIZING
extern
#endif
HASHTABLE InternStringTable;    /* char* to String */

#if ROMIZING
extern
#endif
HASHTABLE UTFStringTable;       /* char* to unique instance */

#if ROMIZING
extern
#endif
HASHTABLE ClassTable;           /* package/base to CLASS */

#if SVM
#if 0 && ROMIZING
extern
#endif
HASHTABLE KeyTable;            /* KEY to unique instance */
#endif

/*=========================================================================
 * Hashtable creation and deletion
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      createHashTable
 * OVERVIEW:      Creates a hashtable, assigns it to a location, and 
 *                registers that location as a root for GC.
 * INTERFACE:
 *   parameters:  tablePtr:  Address of variable holding location
 *                bucketCount: Size of hashtable
 *   returns:     no value, but as a side effect, puts the newly created
 *                hashtable into *tablePtr
 *=======================================================================*/

void
createHashTable(HASHTABLE *tablePtr, int bucketCount) { 
    int objectSize = SIZEOF_HASHTABLE(bucketCount);
    HASHTABLE table = (HASHTABLE)callocPermanentObject(objectSize);
    table->bucketCount = bucketCount;
    *tablePtr = table;
}

/*=========================================================================
 * FUNCTION:      InitializeHashTable
 * OVERVIEW:      Called at startup to create system hashtables.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void InitializeHashtables() { 
    if (!ROMIZING) { 
        createHashTable(&UTFStringTable, UTF_TABLE_SIZE);
        createHashTable(&InternStringTable, INTERN_TABLE_SIZE);
        createHashTable(&ClassTable, CLASS_TABLE_SIZE);
#if SVM
        createHashTable(&KeyTable, KEY_TABLE_SIZE);
#endif
    }
}

/*=========================================================================
 * FUNCTION:      finalizeROMHashTable
 * OVERVIEW:      Return a ROM hashtable back to its "virgin" state.
 * INTERFACE:
 *   parameters:  table:  The hashtable
 *                nextOffset:  Offset of the "next" field within each
 *                entry of the hashtable.
 *
 * This code assumes that the non ROM entries are at the beginning of each
 * chain, and that the ROM entries are at the end.  Hence, it can scan
 * each chain, and just pop off the non-ROM entries until it reaches the end
 * or a ROM entry.
 *=======================================================================*/

#if ROMIZING

void finalizeROMHashTable(HASHTABLE table, int nextOffset) 
{
    int bucketCount = table->bucketCount;
    int i;
    for (i = 0; i < bucketCount; i++) {
        void *p = table->bucket[i];
        while (p != NULL && inAnyHeap(p)) { 
            /* Move to the next bucket */
            p = *(void **)((char *)p + nextOffset);
        }
        table->bucket[i] = p;
    }
}

#endif /* ROMIZING */

void FinalizeHashtables() { 
    if (!ROMIZING) { 
        UTFStringTable = NULL;
        InternStringTable = NULL;
        ClassTable = NULL;
#if SVM
        KeyTable = NULL;
#endif /* SVM */
    }
}

/*=========================================================================
 * Functions on hashtables
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      stringHash
 * OVERVIEW:      Returns a hash value for a C string
 * INTERFACE:
 *   parameters:  s:    pointer to a string
 *                len:  length of string, in bytes
 *   returns:     a hash value
 *=======================================================================*/

static unsigned int
stringHash(const char *s, int length)
{
    unsigned raw_hash = 0;
    while (--length >= 0) { 
        int c = *s++;
        /* raw_hash = raw_hash * 37 + c; */
        raw_hash =   (((raw_hash << 3) + raw_hash)  << 2) + raw_hash  + c;
    }
    return raw_hash;
}

/*=========================================================================
 * FUNCTION:      getUString, getUStringX
 * OVERVIEW:      Returns a unique instance of a given C string
 * INTERFACE:
 *   parameters:  string:       Pointer to an array of characters
 *                stringLength: Length of string (implicit for getUString)
 *   returns:     A unique instance.  If strcmp(x,y)==0, then
 *                 getUString(x) == getUString(y), and
 *                 strcmp(UStringInfo(getUString(x)), x) == 0.
 * 
 * Note that getUString actually allows "string"s with embedded NULLs.
 * Two arrays of characters are equal only if they have the same length 
 * and the same characters.
 *=======================================================================*/

UString
getUString(const char* string) { 
    if (INCLUDEDEBUGCODE && inCurrentHeap(string)) { 
        fatalError(KVM_MSG_BAD_CALL_TO_GETUSTRING);
    }
    return getUStringX(&string, 0, strlen(string));
}

UString
getUStringX(CONST_CHAR_HANDLE nameH, int offset, int stringLength)
{
    const char *start = unhand(nameH);
    const char *string = start + offset; 
    HASHTABLE table = UTFStringTable;
    /* The bucket in the hash table */
    int index =  stringHash(string, stringLength) % table->bucketCount;
    UTF_HASH_ENTRY *bucketPtr = (UTF_HASH_ENTRY *)&table->bucket[index];
    UTF_HASH_ENTRY bucket;
    /* Search the bucket for the corresponding string. */
    for (bucket = *bucketPtr; bucket != NULL; bucket = bucket->next) { 
        const char *key = bucket->string;
        int length = bucket->length;
        if (length == stringLength) { 
            if ((key == string) ||
                (key[0] == string[0] && memcmp(key, string, length) == 0)) {
            if (EXCESSIVE_GARBAGE_COLLECTION && !ASYNCHRONOUS_NATIVE_FUNCTIONS){
                /* We'd garbage collect if it weren't found. . . */
                garbageCollect(0);
            } 
                return bucket;
            }
        }
    }

    /* We have to create a new bucket.  Note that the string is embedded
     * into the bucket.  We always append a '\0' to the string so that if
     * the thing we were passed is a C String, it will continue to be a C
     * String */
    bucket = (UTF_HASH_ENTRY)callocPermanentObject(
                 SIZEOF_UTF_HASH_ENTRY(stringLength));

    /* Install the new item into the hash table.
     * The allocation above may cause string to no longer be valid 
     */
    
    bucket->next = *bucketPtr;
    memcpy((char *)bucket->string, unhand(nameH) + offset, stringLength);
    bucket->string[stringLength] = '\0';
    /* Give the item a key that uniquely represents it.  On purpose, we assign
     * a key that makes it easy for us to find the right bucket.
     * (key % table->bucketCount) = index.
     */
    if (bucket->next == NULL) { 
        bucket->key = table->bucketCount + index;
    } else { 
        unsigned long nextKey = table->bucketCount + bucket->next->key;
        if (nextKey >= 0x10000) { 
            fatalError(KVM_MSG_TOO_MANY_NAMETABLE_KEYS);
        }
        bucket->key = (unsigned short)nextKey;
    }

    bucket->length = stringLength;
    *bucketPtr = bucket;

    /* Increment the count, in case we need this information */
    table->count++;

    /* Return the string */
    return bucket;
}

#if SVM

/*=========================================================================
 * FUNCTION:      retrieveKey
 * OVERVIEW:      Returns a unique instance of a KEY.
 * INTERFACE:
 *   parameters:  length - the length of the KEY data data (in bytes)
 *                data   - the key data
 *   returns:     A system-wide unique instance.
 *=======================================================================*/

KEY
retrieveKey(unsigned int length, unsigned char* data)
{
/*static unsigned int total = 0;*/
    HASHTABLE table = KeyTable;
    /* The bucket in the hash table (should we use a different hashing
     * function for keys? - Doug) */
    int index =  stringHash(data, length) % table->bucketCount;
    KEY *bucketPtr = (KEY*)&table->bucket[index];
    KEY bucket;

/*printf("KeyTable:\ttotal%d, unique %ld\n",total++,table->count);*/

    /*
     * The input data must not be on the heap. It will be stack
     * allocated data in the implementation of one of the crypto provider
     * interface functions.
     */
    if (INCLUDEDEBUGCODE && inCurrentHeap(data)) { 
        fatalError(KVM_MSG_BAD_CALL_TO_GETUSTRING);
    }

    /* Search the bucket for the corresponding data. */
    for (bucket = *bucketPtr; bucket != NULL; bucket = bucket->next) {
        if (length == bucket->size &&
            data[0] == bucket->key[0] &&
            memcmp(data,bucket->key,length) == 0) {
            if (EXCESSIVE_GARBAGE_COLLECTION &&
                !ASYNCHRONOUS_NATIVE_FUNCTIONS) {
                    /* We'd garbage collect if it weren't found. . . */
                    garbageCollect(0);
            } 
            return bucket;
        }
    }

    /* We have to create a new bucket. */
    bucket = (KEY)callocPermanentObject(SIZEOF_KEY(length));
      

    /* Install the new item into the hash table.
     */
    
    bucket->next = *bucketPtr;
    bucket->size = length;
    memcpy((unsigned char *)bucket->key, data, length);
    *bucketPtr = bucket;
    /* Increment the count, in case we need this information */
    table->count++;
    return bucket;
}

#endif /* SVM */

/*=========================================================================
 * FUNCTION:      internString
 * OVERVIEW:      Returns a unique Java String that corresponds to a
 *                particular char* C string
 * INTERFACE:
 *   parameters:  string:  A C string
 *   returns:     A unique Java String, such that if strcmp(x,y) == 0, then
 *                internString(x) == internString(y).
 *=======================================================================*/

INTERNED_STRING_INSTANCE
internString(const char *utf8string, int length)
{ 
    HASHTABLE table = InternStringTable;
    unsigned int hash = stringHash(utf8string, length);
    unsigned int index = hash % table->bucketCount;
    unsigned int utfLength = utfStringLength(utf8string, length);

    INTERNED_STRING_INSTANCE string, *stringPtr;

    stringPtr = (INTERNED_STRING_INSTANCE *)&table->bucket[index];
    
    for (string = *stringPtr; string != NULL; string = string->next) { 
        if (string->length == utfLength) { 
            SHORTARRAY chars = string->array;
            int offset = string->offset;
            const char *p = utf8string;
            unsigned int i;
            for (i = 0; i < utfLength; i++) { 
                short unichar = utf2unicode(&p);
                if (unichar != chars->sdata[offset + i]) { 
                    /* We want to do "continue  <outerLoop>", but this is C */;
                    goto continueOuterLoop;
                }
            }
            if (EXCESSIVE_GARBAGE_COLLECTION && !ASYNCHRONOUS_NATIVE_FUNCTIONS){
                /* We might garbage collect, so we do  */
                garbageCollect(0);
            }
            return string;
        }
    continueOuterLoop:
        ;
    }

    
    string = instantiateInternedString(utf8string, length);
    string->next = *stringPtr;
    *stringPtr = string;
    return string;
}

/*=========================================================================
 * Conversion operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      utf2unicode
 * OVERVIEW:      Converts UTF8 string to unicode char.
 *
 *   parameters:  utfstring_ptr: pointer to a UTF8 string. Set to point 
 *                to the next UTF8 char upon return.
 *   returns      unicode char
 *=======================================================================*/

short utf2unicode(const char **utfstring_ptr) {
    unsigned char *ptr = (unsigned char *)(*utfstring_ptr);
    unsigned char ch, ch2, ch3;
    int length = 1;     /* default length */
    short result = 0x80;    /* default bad result; */

    switch ((ch = ptr[0]) >> 4) {
        default:
        result = ch;
        break;

    case 0x8: case 0x9: case 0xA: case 0xB: case 0xF:
        /* Shouldn't happen. */
        break;

    case 0xC: case 0xD: 
        /* 110xxxxx  10xxxxxx */
        if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
            unsigned char high_five = ch & 0x1F;
            unsigned char low_six = ch2 & 0x3F;
            result = (high_five << 6) + low_six;
            length = 2;
        } 
        break;

    case 0xE:
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
            if (((ch3 = ptr[2]) & 0xC0) == 0x80) {
                unsigned char high_four = ch & 0x0f;
                unsigned char mid_six = ch2 & 0x3f;
                unsigned char low_six = ch3 & 0x3f;
                result = (((high_four << 6) + mid_six) << 6) + low_six;
                length = 3;
            } else {
                length = 2;
            }
        }
        break;
    } /* end of switch */

    *utfstring_ptr = (char *)(ptr + length);
    return result;
}


/*=========================================================================
 * FUNCTION:      utfStringLength
 * OVERVIEW:      Determine the number of 16-bit characters in a UTF-8 string.
 *
 *   parameters:  utfstring_ptr: pointer to a UTF8 string. 
 *                length of string
 *   returns      string length
 *=======================================================================*/

unsigned int 
utfStringLength(const char *utfstring, int length) 
{
    const char *ptr = utfstring;
    const char *end = utfstring + length;
    unsigned int count;

    for (count = 0; ptr < end; count++) { 
        unsigned char ch = (unsigned char)ptr[0];
        if (ch < 0x80) { 
            /* 99% of the time */
            ptr++;
        } else { 
            switch(ch >> 4) { 
                default:
                    fatalError("Bad UTF string");
                    break;
                case 0xC: case 0xD: 
                    ptr += 2; break;
                case 0xE:
                    ptr += 3; break;
            }
        }
    }
    return count;
}

#if ENABLE_JAVA_DEBUGGER

/*
 * For now, only the debugger code needs to convert back from unicode
 * to utf when it sends a string value back to the debugger.
 * So these functions are in the ENABLE_JAVA_DEBUGGER block
 */

/* Given a unicode string and its length, convert it to a utf string.  But 
 * the result into the given buffer, whose length is buflength.  The utf
 * string should include a null terminator.
 *
 * If both buffer and buflength are 0, then malloc an appropriately sized
 * buffer for the result.
 */

char *unicode2utf(unsigned short *unistring, int length, char *buffer, int buflength)
{
    int i;
    unsigned short *uniptr;
    char *bufptr;
    unsigned bufleft;

    bufleft = buflength - 1; /* take note of null now! */

    for (i = length, uniptr = unistring, bufptr = buffer; --i >= 0; uniptr++) {
        unsigned short ch = *uniptr;
        if ((ch != 0) && (ch <=0x7f)) {
            if ((int)(--bufleft) < 0)     /* no space for character */
                break;
            *bufptr++ = (char)ch;
        } else if (ch <= 0x7FF) { 
            /* 11 bits or less. */
            unsigned char high_five = ch >> 6;
            unsigned char low_six = ch & 0x3F;
            if ((int)(bufleft -= 2) < 0)  /* no space for character */
                break;
            *bufptr++ = high_five | 0xC0; /* 110xxxxx */
            *bufptr++ = low_six | 0x80;   /* 10xxxxxx */
        } else {
            /* possibly full 16 bits. */
            char high_four = ch >> 12;
            char mid_six = (ch >> 6) & 0x3F;
            char low_six = ch & 0x3f;
            if ((int)(bufleft -= 3) < 0) /* no space for character */
                break;
            *bufptr++ = high_four | 0xE0; /* 1110xxxx */
            *bufptr++ = mid_six | 0x80;   /* 10xxxxxx */
            *bufptr++ = low_six | 0x80;   /* 10xxxxxx*/
        }
    }
    *bufptr = 0;
    return buffer;
}

/* Return the number of characters that would be needed to hold the unicode
 * string in utf.
 */
int unicode2utfstrlen(unsigned short *unistring, int unilength)
{
    int result_length = 0;
    
    for (; unilength > 0; unistring++, unilength--) {
        unsigned short ch = *unistring;
        if ((ch != 0) && (ch <= 0x7f)) /* 1 byte */
            result_length++;
        else if (ch <= 0x7FF)
            result_length += 2;        /* 2 byte character */
        else 
            result_length += 3;        /* 3 byte character */
    }
    return result_length;
}

#endif /* ENABLE_JAVA_DEBUGGER */

/*=========================================================================
 * FUNCTION:      change_Name_to_Key, change_Key_to_Name
 * OVERVIEW:      Converts between an array of bytes, and a unique 16-bit
 *                number (the Key)
 *
 * INTERFACE:   change_Name_to_key
 *   parameters:  string:       Pointer to an array of characters
 *                stringLength: Length of array
 *   returns      A unique 16-bit key
 *
 * INTERFACE:   change_key_to_Name
 *   parameters:  key: A unique 16-bit key
 *   lengthP:     If non-NULL, the length of the result is returned here.
 *
 *   returns      A pointer to the array of characters.
 *=======================================================================*/

NameKey
change_Name_to_Key(CONST_CHAR_HANDLE nameH, int offset, int length) { 
    UString UName = getUStringX(nameH, offset, length);
    return UName->key;
}

char *
change_Key_to_Name(NameKey key, int *lengthP) { 
    HASHTABLE table = UTFStringTable;
    int index =  key % table->bucketCount;
    UTF_HASH_ENTRY *bucketPtr = (UTF_HASH_ENTRY *)&table->bucket[index];
    UTF_HASH_ENTRY bucket;
    /* Search the bucket for the corresponding string. */
    for (bucket = *bucketPtr; bucket != NULL; bucket = bucket->next) { 
        if (key == bucket->key) { 
            if (lengthP) { 
                *lengthP = bucket->length;
            }
            return UStringInfo(bucket);
        }
    }
    return NULL;
}

/*=========================================================================
 * FUNCTION:      change_Name_to_CLASS, change_Key_to_CLASS
 * OVERVIEW:      Internal function used by internString and internClass.
 *                It maps a C string to a specific location capable of
 *                holding a value.  This location is initially NULL.
 * INTERFACE:
 *   parameters:  table:  A hashtable whose buckets are MAPPING_HASH_ENTRY.
 *                string:  A C string
 *   returns:     A unique location, capable of holding a value, corresponding
 *                to that string.
 *=======================================================================*/

CLASS
change_Name_to_CLASS(UString packageName, UString baseName) { 
    HASHTABLE table = ClassTable;
    unsigned int hash;
    unsigned int index;
    unsigned int lastKey = 0;
    CLASS *clazzPtr, clazz;

    hash = stringHash(UStringInfo(baseName), baseName->length) + 37;
    if (packageName != NULL) { 
        hash += stringHash(UStringInfo(packageName), packageName->length) * 3;
    } 
    index = hash % table->bucketCount;
    clazzPtr = (CLASS *)&table->bucket[index];

    for (clazz = *clazzPtr; clazz != NULL; clazz = clazz->next) { 
        if (clazz->packageName == packageName && clazz->baseName == baseName) { 
            if (EXCESSIVE_GARBAGE_COLLECTION && !ASYNCHRONOUS_NATIVE_FUNCTIONS){
                /* We might garbage collect, so we do */
                garbageCollect(0);
            }
            return clazz;
        }
        if (lastKey == 0) { 
            unsigned thisKey = clazz->key;
            int pseudoDepth = thisKey >> FIELD_KEY_ARRAY_SHIFT;
            if (pseudoDepth == 0 || pseudoDepth == MAX_FIELD_KEY_ARRAY_DEPTH) { 
                lastKey = thisKey & 0x1FFF;
            }
        }
    }
    if (UStringInfo(baseName)[0] == '[') { 
        clazz = (CLASS)callocPermanentObject(SIZEOF_ARRAY_CLASS);
    } else { 
#if ROMIZING
        if (packageName != NULL) { 
            char *name = UStringInfo(packageName);
            if (IS_RESTRICTED_PACKAGE_NAME(name)) { 
                fatalError(KVM_MSG_CREATING_CLASS_IN_SYSTEM_PACKAGE);
            }
        }
#endif
        clazz = (CLASS)callocPermanentObject(SIZEOF_INSTANCE_CLASS);
    }
    clazz->next = *clazzPtr;
    *clazzPtr = clazz;

    clazz->packageName = packageName;
    clazz->baseName = baseName;

    /* The caller may change this value to be one of the values with 
     * depth 1 <= depth <= MAX_FIELD_KEY_ARRAY_DEPTH. */
    clazz->key = lastKey == 0 ? (256 + index) : (lastKey + table->bucketCount);
    if (clazz->key & ITEM_NewObject_Flag) { 
        fatalError(KVM_MSG_TOO_MANY_CLASS_KEYS);
    }

    table->count++;
    return clazz;

}
    
CLASS
change_Key_to_CLASS(FieldTypeKey key) 
{
    int depth = key >> FIELD_KEY_ARRAY_SHIFT;
    if (depth == 0 || depth == MAX_FIELD_KEY_ARRAY_DEPTH) { 
        HASHTABLE table = ClassTable;
        int index =  ((key & 0x1FFF) - 256) % table->bucketCount;
        CLASS *clazzPtr = (CLASS *)&table->bucket[index];
        CLASS clazz;
        /* Search the clazz for the corresponding string. */
        for (clazz = *clazzPtr; clazz != NULL; clazz = clazz->next) { 
            if (key == clazz->key) { 
                return clazz;
            }
        }
        return NULL;
    } else {
        FieldTypeKey baseClassKey = key & 0x1FFF;
        if (baseClassKey <= 255) { 
            return (CLASS)getArrayClass(depth, NULL, (char)baseClassKey);
        } else { 
            CLASS temp = change_Key_to_CLASS(baseClassKey);
            return (CLASS)getArrayClass(depth, (INSTANCE_CLASS)temp, '\0');
        }
    }
}

/*=========================================================================
 * Printing and debugging operations
 *=======================================================================*/

#if INCLUDEDEBUGCODE

void
printUTFStringTable() { 
    HASHTABLE table = UTFStringTable;
    int count = table->bucketCount;
    while (--count >= 0) { 
        UString bucket = (UString)table->bucket[count];
        for ( ; bucket != NULL; bucket = bucket->next) { 
            int length = bucket->length;
            char *string = bucket->string;
            if (length > 0 && string[0] < 20) { 
                change_Key_to_MethodSignature_inBuffer(bucket->key, str_buffer);
                fprintf(stdout, "***  \"%s\"\n", str_buffer);
            } else { 
                fprintf(stdout, "     \"%s\"\n", string);
            }
        }
    }
}

#endif /* INCLUDEDEBUGCODE */


