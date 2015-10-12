
%include {
    #include <stdlib.h>
    #include <string.h>
    #include <assert.h>
    #include "parser_types.h"

    extern bool TryParseNote(const char*, Note*);
}

%extra_argument { PpFile* FileHead }
%parse_accept {
    fprintf(stderr, "Successfully parsed file.\n");
}
%parse_failure {
    fprintf(stderr, "Syntax error somewhere\n");
    FileHead->instances = NULL;
    FileHead->staves = NULL;
}

%token_type { char* }

%type section_list { PpFile }
%type section { PpFile }

%type instance_section { PpInstanceList* }
%type instance_list { PpInstanceList* }
%type instance { PpInstance }

%type input_section { PpStaffList* }
%type staff_list { PpStaffList* }
%type staff { PpStaff }
%type type { char }

%type param_list { PpParamList* }
%type key_val_list { PpParamList* }
%type key_val { PpParam }
%type value { PpValue }
%type values { PpValueList* }
%type list_value { PpValueList* }

%nonassoc LCURLY RCURLY LSQUARE RSQUARE EQUALS SECTION END INSTANCES NOTES PCM OUTPUT.

%start_symbol file

file ::= section_list(val). {
    *FileHead = val;
}

section_list(R) ::= . {
    R.instances = NULL;
    R.staves = NULL;
}
section_list(R) ::= section_list(head) section(curr). {
    PpInstanceList* p1 = head.instances;
    if(!p1) head.instances = curr.instances;
    else {
        while(p1 && p1->next) p1 = p1->next;
        p1->next = curr.instances;
    }
    PpStaffList* p2 = head.staves;
    if(!p2) head.staves = curr.staves;
    else {
        while(p2 && p2->next) p2 = p2->next;
        p2->next = curr.staves;
    }
    R = head;
}

section(R) ::= instance_section(val). {
    R.instances = val;
    R.staves = NULL;
}
section(R) ::= input_section(val). {
    R.instances = NULL;
    R.staves = val;
}

instance_section(R) ::= SECTION INSTANCES instance_list(val) END SECTION. {
    R = val;
}

instance_list(R) ::= . {
    R = NULL;
}
instance_list(R) ::= instance_list(head) instance(curr). {
    PpInstanceList* next = (PpInstanceList*)malloc(sizeof(PpInstanceList));
    next->next = NULL;
    next->value = curr;
    PpInstanceList* p = head;
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

instance(R) ::= OUTPUT param_list(LEparams). {
    static const char* output = "OUTPUT";
    R.name = NULL;
    R.type = (char*)malloc(sizeof(char) * (sizeof(output)/sizeof(char) + 1));
    strcpy(R.type, output);
    R.params = LEparams;
}

instance(R) ::= STRING(LEname) STRING(LEtype) param_list(LEparams). {
    R.name = LEname;
    R.type = LEtype;
    R.params = LEparams;
}

input_section(R) ::= SECTION NOTES staff_list(val) END SECTION. {
    R = val;
}
input_section(R) ::= SECTION NOTES END SECTION. {
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