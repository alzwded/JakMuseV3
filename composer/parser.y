
%include {
    #include <assert.h>
    #include "note.h"
    #include "parser_types.h"

    extern bool TryParseNote(const char*, Note*);
}

%extra_argument { PpStaffList** FileHead }
%parse_accept {
    fprintf(stderr, "Successfully parsed file.\n");
}
%parse_failure {
    fprintf(stderr, "Syntax error somewhere\n");
    *FileHead = NULL;
}

%token_type { char* }

%type staff_list { PpStaffList* }
%type staff { PpStaff }
%type type { char }
%type param_list { PpParamList* }
%type key_val_list { PpParamList* }
%type key_val { PpParam }
%type value { PpValue }
%type values { PpValueList* }
%type list_value { PpValueList* }

%nonassoc LCURLY RCURLY LSQUARE RSQUARE EQUALS.

%start_symbol file

file ::= staff_list(val). {
    *FileHead = val;
}
file(R) ::= . {
    R = NULL;
}

staff_list(R) ::= staff(curr). {
    PpStaffList* next = (PpStaffList*)malloc(sizeof(PpStaffList));
    next->next = NULL;
    next->value = curr;
    R = next;
}
staff_list(R) ::= staff_list(head) staff(curr). {
    PpStaffList* next = (PpStaffList*)malloc(sizeof(PpStaffList));
    next->next = NULL;
    next->value = curr;
    PpStaffList* p = head;
    while(p && p->next) {
        p = p->next;
    }
    if(p) {
        p->next = next;
        R = head;
    } else {
        R = next;
    }
}

staff(R) ::= STRING(staffName) type(LEtype) param_list(LEparams). {
    R.name = staffName;
    R.type = LEtype;
    R.params = LEparams;
}

type(R) ::= NOTES. {
    R = 'N';
}
type(R) ::= PCM. {
    R = 'P';
}

param_list(R) ::= LCURLY key_val_list(val) RCURLY. {
    R = val;
}

key_val_list(R) ::= key_val_list(head) key_val(curr). {
    PpParamList* next = (PpParamList*)malloc(sizeof(PpParamList));
    next->next = NULL;
    next->value = curr;
    PpParamList* p = head;
    while(p && p->next) {
        p = p->next;
    }
    if(p) {
        p->next = next;
        R = head;
    } else {
        R = next;
    }
}
key_val_list(R) ::= . {
    R = NULL;
}

key_val(R) ::= STRING(LEkey) EQUALS value(LEval). {
    R.key = LEkey;
    R.value = LEval;
}

value(R) ::= NUMBER(val). {
    int num = atoi(val);
    PpValue ret;
    ret.type = PpValue::PpNUMBER;
    ret.num = num;
    free(val);
    R = ret;
}
value(R) ::= NOTE(val). {
    PpValue ret;
    ret.type = PpValue::PpNOTE;
    (void) TryParseNote(val, &ret.note);
    free(val);
    R = ret;
}
value(R) ::= STRING(val). {
    PpValue ret;
    ret.type = PpValue::PpSTRING;
    ret.str = val;
    R = ret;
}
value(R) ::= list_value(val). {
    PpValue ret;
    ret.type = PpValue::PpLIST;
    ret.list = val;
    R = ret;
}

list_value(R) ::= LSQUARE values(val) RSQUARE. {
    R = val;
}

values(R) ::= values(head) value(curr). {
    PpValueList* next = (PpValueList*)malloc(sizeof(PpValueList));
    next->next = NULL;
    next->value = curr;
    PpValueList* p = head;
    while(p && p->next) {
        p = p->next;
    }
    if(p) {
        p->next = next;
        R = head;
    } else {
        R = next;
    }
}
values(R) ::= . {
    R = NULL;
}
