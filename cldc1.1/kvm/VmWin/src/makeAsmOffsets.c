#include <global.h>
#include <stdio.h>

#define CALC_FIELD_OFFSET(structure, field) \
    ((char *)(&((structure *)NULL)->field))

int main(int argc, char ** argv) {
    printf("; File: defines.asm\n");
    printf("; \n");
    printf("; NOTE: This file is generated automatically. Any changes\n");
    printf("; you make will be overwritten the next time this file is\n");
    printf("; generated.\n");

    printf("\n");
    printf("; field offsets\n");
    printf("ARRAY_LENGTH_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct arrayStruct, length));
    printf("ARRAY_DATA_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct arrayStruct, data));
    printf("ARRAY_DATA_OFFSET_LONG equ %d\n", CALC_FIELD_OFFSET(struct arrayStruct, data) + 4);
    printf("INSTANCE_CLASS_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct instanceStruct, ofClass));
    printf("INSTANCE_DATA_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct instanceStruct, data));
    printf("CLASS_STATUS_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct instanceClassStruct, status));
    printf("CLASS_THREAD_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct instanceClassStruct, initThread));

    printf("IP_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct GlobalStateStruct, gs_ip));
    printf("SP_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct GlobalStateStruct, gs_sp));
    printf("LP_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct GlobalStateStruct, gs_lp));
    printf("CP_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct GlobalStateStruct, gs_cp));
    printf("FP_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct GlobalStateStruct, gs_fp));
    printf("FRAME_PREV_IP_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct frameStruct, previousIp));
    printf("FRAME_METHOD_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct frameStruct, thisMethod));
    printf("FRAME_SYNC_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct frameStruct, syncObject));
    printf("METHOD_CLASS_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct methodStruct, ofClass));
    printf("METHOD_ACCESSFLAGS_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct methodStruct, accessFlags));
    printf("METHOD_ARGCOUNT_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct methodStruct, argCount));
    printf("METHOD_KEY_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct methodStruct, nameTypeKey));
    printf("FIELD_CLASS_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct fieldStruct, ofClass));
    printf("FIELD_ADDR_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct fieldStruct, u.staticAddress));
    printf("FIELD_ADDR_OFFSET_LONG equ %d\n", CALC_FIELD_OFFSET(struct fieldStruct, u.staticAddress) + 4);
    printf("CP_ENTRY_CLASS_OFFSET equ %d\n", CALC_FIELD_OFFSET(union constantPoolEntryStruct, clazz));
    printf("CP_ENTRY_INT_OFFSET equ %d\n", CALC_FIELD_OFFSET(union constantPoolEntryStruct, integer));
    printf("ELEMENT_CLASS_OFFSET equ %d\n", CALC_FIELD_OFFSET(struct arrayClassStruct, u.elemClass));

    printf("\n");
    printf("; struct sizes\n");
    printf("FP_INC equ %d\n", sizeof(struct frameStruct));
    printf("CP_ENTRY_SIZE equ %d\n", sizeof(((struct constantPoolStruct *)NULL)->entries[0]));

    printf("\n");
    printf("; enumerations\n");
    printf("MONITOR_STATUS_ERROR equ %d\n", MonitorStatusError);
    printf("KILLTHREAD equ %d\n", KILLTHREAD);
    printf("CLASS_READY equ %d\n", CLASS_READY);

    printf("\n");
    printf("; bit masks\n");
    printf("ACC_NATIVE equ %.4xh\n", ACC_NATIVE);
    printf("ACC_ABSTRACT equ %.4xh\n", ACC_ABSTRACT);
    printf("ACC_SYNCHRONIZED equ %.4xh\n", ACC_SYNCHRONIZED);
    printf("ACC_PUBLIC equ %.4xh\n", ACC_PUBLIC);
    printf("ACC_PUBLIC_OR_STATIC equ %.4xh\n", ACC_PUBLIC | ACC_STATIC);

    return 0;
}
