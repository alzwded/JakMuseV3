#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

struct PpValueList;
struct PpValue {
    enum { PpSTRING, PpNUMBER, PpLIST, PpNOTE } type;
    union {
        char* str;
        int num;
        PpValueList* list;
        Note note;
    };
};
struct PpParam {
    char* key;
    PpValue value;
};
struct PpParamList {
    PpParamList* next;
    PpParam value;
};
struct PpValueList {
    PpValueList* next;
    PpValue value;
};
struct PpStaff {
    char* name;
    char type;
    PpParamList* params;
};
struct PpStaffList {
    PpStaffList* next;
    PpStaff value;
};

#endif
