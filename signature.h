#ifndef SIGNATURE_H
#define SIGNATURE_H

// https://cscott.net/Projects/GJ/signature-explained-2_4.html
// https://cscott.net/Projects/GJ/signature-explained.html

// jvm spec: https://docs.oracle.com/javase/specs/jvms/se15/html/jvms-4.html#jvms-4.7.9.1
/**
 * signature
 *
 **/ 

#include "list.h"

/**
 * The 2.4 prototype compiler uses the variance specifiers '-', '+', or '*' of the 2.0 prototype to start a
 * TypeArgument; they are no longer allowed
 * after the open-brackets in an ArrayTypeSignature.
 * The Java syntax corresponding to '-' is "? super ",
 * corresponding to '+' is "? extends ", and '*' corresponds to "?". The old '=' variance
 * specifier is no longer allowed/generated.
 * The only difference between the 2.2 grammar and the 2.4
 * grammar (that I know of at the moment) is in the
 * grammar for ParameterSignature, where an extra colon has been added.
 * A corrected version of the original JSR-14 spec follows.
 **/

typedef char* string;
typedef void* object;

typedef struct base_type_signature {
    int tag;
    /*
     * 0: B
     * 1: C
     * 2: D
     * 3: F
     * 4: I
     * 5: J
     * 6: S
     * 7: Z
     * 8: V
     **/
} base_type_signature;

typedef struct bottom_signature {
} bottom_signature;

typedef struct class_type_signature {
    list_object *path;
} class_type_signature;

typedef struct simple_class_type_signature {
    int dollor;
    string name;
    list_object *type_arguments;
} simple_class_type_signature;

typedef struct type_variable_signature {
    string name;
} type_variable_signature;

typedef struct field_type_signature field_type_signature;
typedef struct field_type_signature type_signature;
//typedef struct type_signature type_signature;
typedef struct field_type_signature type_argument;

typedef struct array_type_signature {
    type_signature *component_type;
} array_type_signature;

typedef struct wildcard {
    string tag;
    field_type_signature *super_bound;
    field_type_signature *extends_bound;
} wildcard;

typedef enum type_signature_enum {
    BASE_TYPE_SIGNATURE,
    CLASS_TYPE_SIGNATURE,
    TYPE_VARIABLE_SIGNATURE,
    ARRAY_TYPE_SIGNATURE,
    BOTTOM_SIGNATURE,
    SIMPLE_CLASS_TYPE_SIGNATURE,
    WILDCARD,
    FIELD_TYPE_SIGNATURE
} type_signature_enum;

typedef union type_signature_union {
    base_type_signature         *base_type;
    class_type_signature        *class_type;
    type_variable_signature     *type_variable;
    array_type_signature        *array_type;
    bottom_signature            *bottom;
    simple_class_type_signature *simple_class_type;
    wildcard                    *wildcard;
    field_type_signature        *field_type;
} type_signature_union;

struct field_type_signature {
    type_signature_enum tag;
    type_signature_union *type;
};

typedef struct formal_type_parameter {
    string name;
    list_object *bounds; 
} formal_type_parameter;

typedef struct class_signature {
    class_type_signature    *base_class;
    list_object             *formal_type_parameters;
    list_object             *interfaces;
} class_signature;

typedef struct method_signature {
    list_object         *formal_type_parameters;
    list_object         *parameter_types;
    type_signature      *return_type;
    list_object         *exception_types;
} method_signature;

static string parse_identifier(string sig, int *pint);

static field_type_signature* parse_field_type_signature(string sig, int *pint);

static simple_class_type_signature* parse_simple_class_type_signature(string sig, int *pint, int use_dollor);

static list_object* parse_bounds(string sig, int *pint);

extern class_signature* parse_class_signature(string sig);

extern method_signature* parse_method_signature(string sig);

#endif 
