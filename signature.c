#include "signature.h"

static inline void increase(int *int_pointer)
{
    (*int_pointer)++;
}

static inline char char_at_index(string sig, int *pint)
{
    return sig[*pint];
}

static string field_type_signature_to_string(field_type_signature *fts);

static string simple_class_signature_to_string(simple_class_type_signature *ss)
{
    size_t old_len = strlen(ss->name) + 1; // name\0
    if (ss->type_arguments != NULL && ss->type_arguments->size > 0)
        old_len += 2; // <>
    size_t new_len = old_len;
    string result = x_alloc(old_len); // name\0
    strcat(result, ss->name);
    if (ss->type_arguments != NULL)
    {
        strcat(result, "<");
        for (int i = 0; i < ss->type_arguments->size; ++i)
        {
            type_argument *ta = list_get_object(ss->type_arguments, i);
            string fts_str = field_type_signature_to_string(ta);
            new_len += strlen(fts_str);
            result = x_realloc(result, old_len, new_len);
            strcat(result, fts_str);
            old_len = new_len;
            if (i != ss->type_arguments->size-1)
            {
                new_len += 2;
                result = x_realloc(result, old_len, new_len);
                strcat(result, ", ");
                old_len = new_len;
            }
        }
        strcat(result, ">");
    }
    return result;
}

static string field_type_signature_to_string(field_type_signature *fts)
{
    switch (fts->tag) {
        case BASE_TYPE_SIGNATURE:
        {
            switch (fts->type->base_type->tag) {
                case 0:
                    return "byte";
                case 1:
                    return "char";
                case 2:
                    return "double";
                case 3:
                    return "float";
                case 4:
                    return "int";
                case 5:
                    return "long";
                case 6:
                    return "short";
                case 7:
                    return "boolean";
                case 8:
                    return "void";
                default:
                    return "unknown";
            }
        }
        case CLASS_TYPE_SIGNATURE:
        {
            class_type_signature *cts = fts->type->class_type;
            string result = x_alloc(1); // \0
            size_t old_len = 1;
            size_t new_len = 1;
            for (int i = 0; i < cts->path->size; ++i)
            {
                simple_class_type_signature *ss = list_get_object(cts->path, i);
                string s1 = simple_class_signature_to_string(ss);
                new_len = old_len + strlen(s1) + 1;
                result = x_realloc(result, old_len, new_len);
                strcat(result, s1);
                old_len = new_len;
            }
            result[new_len-1] = '\0';
            return result;
        }
        case TYPE_VARIABLE_SIGNATURE:
        {
            type_variable_signature *tvs = fts->type->type_variable;
            return tvs->name;
        }
        case ARRAY_TYPE_SIGNATURE:
        {
            array_type_signature *ats = fts->type->array_type;
            return field_type_signature_to_string(ats->component_type);
        }
        case BOTTOM_SIGNATURE:
        {
            return "java.lang.Object";
        }
        case SIMPLE_CLASS_TYPE_SIGNATURE:
        {
            simple_class_type_signature *ss = fts->type->simple_class_type;
            return simple_class_signature_to_string(ss);
        }
        case WILDCARD:
        {
            wildcard *w = fts->type->wildcard;
            field_type_signature *fsuper = w->super_bound;
            field_type_signature *fextends = w->extends_bound;
            string result = x_alloc(1); // \0
            size_t old_len = 1;
            size_t new_len = 1;
            if (STRING_EQUAL(w->tag, "+"))
            {
                size_t l = strlen("? extends ");
                new_len = old_len + l;
                result = x_realloc(result, old_len, new_len);
                strcat(result, "? extends ");
                old_len = new_len;
                if (fsuper->tag != BOTTOM_SIGNATURE) {
                    string super_str = field_type_signature_to_string(fsuper);
                    new_len = old_len + strlen(super_str);
                    result = x_realloc(result, old_len, new_len);
                    strcat(result, super_str);
                    if (fextends->tag != BOTTOM_SIGNATURE) {
                        new_len = old_len + strlen(" & ");
                        result = x_realloc(result, old_len, new_len);
                        strcat(result, " & ");
                        old_len = new_len;
                    }
                }
                if (fextends->tag != BOTTOM_SIGNATURE) {
                    string extends_str = field_type_signature_to_string(fextends);
                    new_len = old_len + strlen(extends_str);
                    result = x_realloc(result, old_len, new_len);
                    strcat(result, extends_str);
                }
            } else if (STRING_EQUAL(w->tag, "-")) {
                size_t l = strlen("? super ");
                new_len = old_len + l;
                result = x_realloc(result, old_len, new_len);
                strcat(result, "? super ");
                old_len = new_len;
                if (fsuper->tag != BOTTOM_SIGNATURE) {
                    string super_str = field_type_signature_to_string(fsuper);
                    new_len = old_len + strlen(super_str);
                    result = x_realloc(result, old_len, new_len);
                    strcat(result, super_str);
                    old_len = new_len;
                    if (fextends->tag != BOTTOM_SIGNATURE) {
                        new_len = old_len + strlen(" & ");
                        result = x_realloc(result, old_len, new_len);
                        strcat(result, " & ");
                        old_len = new_len;
                    }
                }
                if (fextends->tag != BOTTOM_SIGNATURE) {
                    string extends_str = field_type_signature_to_string(fextends);
                    new_len = old_len + strlen(extends_str);
                    result = x_realloc(result, old_len, new_len);
                    strcat(result, extends_str);
                }
            }

            return result;
        }
        case FIELD_TYPE_SIGNATURE:
        {
            return field_type_signature_to_string(fts->type->field_type);
        }
        default:
            return "unknown";
    }
}

static type_argument* parse_type_argument(string sig, int *pint)
{
    type_argument *ta = x_alloc(sizeof(type_argument));
    ta->type = x_alloc(sizeof(type_signature_union));

    char c = char_at_index(sig, pint);
    switch (c) {
        case '+':
        {
            increase(pint);
            ta->tag = WILDCARD;
            ta->type->wildcard = x_alloc(sizeof(wildcard));
            ta->type->wildcard->tag = x_strdup("+");
            ta->type->wildcard->super_bound = x_alloc(sizeof(field_type_signature));
            ta->type->wildcard->super_bound->tag = BOTTOM_SIGNATURE;
            ta->type->wildcard->super_bound->type = x_alloc(sizeof(type_signature_union));
            ta->type->wildcard->super_bound->type->bottom = x_alloc(sizeof(bottom_signature));

            ta->type->wildcard->extends_bound = parse_field_type_signature(sig, pint);
            DEBUG_PRINT("BottomSignature\n");
            return ta;
        }
        case '*':
        {
            increase(pint);
            DEBUG_PRINT("[wild card]BottomSignature extends: java.lang.Object\n");

            simple_class_type_signature *ss = x_alloc(sizeof(simple_class_type_signature));
            ss->dollor = 0;
            ss->name = x_strdup("java.lang.Object");
            ss->type_arguments = NULL;

            ta->tag = WILDCARD;
            ta->type->wildcard = x_alloc(sizeof(wildcard));
            ta->type->wildcard->tag = x_strdup("*");
            ta->type->wildcard->super_bound = x_alloc(sizeof(field_type_signature));
            ta->type->wildcard->super_bound->tag = BOTTOM_SIGNATURE;
            ta->type->wildcard->super_bound->type = x_alloc(sizeof(type_signature_union));
            ta->type->wildcard->super_bound->type->bottom = x_alloc(sizeof(bottom_signature));

            ta->type->wildcard->extends_bound = x_alloc(sizeof(field_type_signature));
            ta->type->wildcard->extends_bound->tag = SIMPLE_CLASS_TYPE_SIGNATURE;
            ta->type->wildcard->extends_bound->type = x_alloc(sizeof(type_signature_union));
            ta->type->wildcard->extends_bound->type->simple_class_type = ss;
            return ta;
        }
        case '-':
        {
            increase(pint);
            simple_class_type_signature *ss = x_alloc(sizeof(simple_class_type_signature));
            ss->dollor = 0;
            ss->name = x_strdup("java.lang.Object");
            ss->type_arguments = NULL;


            ta->tag = WILDCARD;
            ta->type->wildcard = x_alloc(sizeof(wildcard));
            ta->type->wildcard->tag = x_strdup("-");
            ta->type->wildcard->super_bound = parse_field_type_signature(sig, pint);

            ta->type->wildcard->extends_bound = x_alloc(sizeof(field_type_signature));
            ta->type->wildcard->extends_bound->tag = SIMPLE_CLASS_TYPE_SIGNATURE;
            ta->type->wildcard->extends_bound->type = x_alloc(sizeof(type_signature_union));
            ta->type->wildcard->extends_bound->type->simple_class_type = ss;
            DEBUG_PRINT("[wild card]BottomSignature super: java.lang.Object\n");
            return ta;
        }
        default:
        {
            return parse_field_type_signature(sig, pint);
        }
    }
}

static list_object* parse_type_arguments(string sig, int *pint)
{
    list_object *type_arguments = list_init_object();

    increase(pint);
    list_insert_object(type_arguments, parse_type_argument(sig, pint));
    char c = char_at_index(sig, pint);
    while (c != '>')
    {
        list_insert_object(type_arguments, parse_type_argument(sig, pint));
        c = char_at_index(sig, pint);
    }
    increase(pint);

    return type_arguments;
}

static void parse_class_type_signature_suffix(string sig, int *pint, list_object *list)
{
    char c = char_at_index(sig, pint);

    while (c == '/' || c == '.')
    {
        int use_dollor = c == '.';
        increase(pint);
        simple_class_type_signature *ss = parse_simple_class_type_signature(sig, pint, use_dollor);
        list_insert_object(list, ss);
        c = char_at_index(sig, pint);
    }
}

static simple_class_type_signature* parse_simple_class_type_signature(string sig, int *pint, int use_dollor)
{
    simple_class_type_signature *s = x_alloc(sizeof(simple_class_type_signature));
    s->name = parse_identifier(sig, pint);
    s->dollor = use_dollor;

    char c = char_at_index(sig, pint);

    switch (c) {
        case ';':
        case '/':
        case '.':
        case '$':
        {
            DEBUG_PRINT("[simple class type signature] empty type arguments\n");
            s->type_arguments = NULL;
            break;
        }
        case '<':
        {
            DEBUG_PRINT("[simple class type signature] type arguments: %c\n", c);
            s->type_arguments = parse_type_arguments(sig, pint);
            break;
        }
        default:
        {
            DEBUG_PRINT("[parse error] ---------------- 1.\n");
            break;
        }
    }
    return s;
}

static class_type_signature* parse_class_type_signature(string sig, int *pint)
{
    list_object *list = list_init_object();
    increase(pint);
    simple_class_type_signature *ss = parse_simple_class_type_signature(sig, pint, 0);
    list_insert_object(list, ss);
    parse_class_type_signature_suffix(sig, pint, list);
    increase(pint);

    class_type_signature *cs = x_alloc(sizeof(class_type_signature));
    cs->path = list;
    return cs;
}

static type_variable_signature* parse_type_variable_signature(string sig, int *pint)
{

    increase(pint);
    type_variable_signature *ts = x_alloc(sizeof(type_variable_signature));
    ts->name = parse_identifier(sig, pint);
    increase(pint);

    return ts;
}

static base_type_signature* parse_base_type(string sig, int *pint)
{
    base_type_signature *bs = x_alloc(sizeof(base_type_signature));
    char c = char_at_index(sig, pint);
    switch (c) {
        case 'B':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] byte\n");
            bs->tag = 0;
            break;
        }
        case 'C':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] char\n");
            bs->tag = 1;
            break;
        }
        case 'D':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] double\n");
            bs->tag = 2;
            break;
        }
        case 'F':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] float\n");
            bs->tag = 3;
            break;
        }
        case 'I':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] int\n");
            bs->tag = 4;
            break;
        }
        case 'J':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] long\n");
            bs->tag = 5;
            break;
        }
        case 'S':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] short\n");
            bs->tag = 6;
            break;
        }
        case 'Z':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] boolean\n");
            bs->tag = 7;
            break;
        }
        case 'V':
        {
            increase(pint);
            DEBUG_PRINT("[BaseType] void\n");
            bs->tag = 8;
            break;
        }
        default: {
            DEBUG_PRINT("[parse error] BaseType\n");
            break;
        }
    }
    return bs;
}

static type_signature* parse_type_signature(string sig, int *pint)
{
    type_signature *ts = x_alloc(sizeof(type_signature));
    ts->type = x_alloc(sizeof(type_signature_union));
    char c = char_at_index(sig, pint);
    switch (c) {
        case 'B':
        case 'C':
        case 'D':
        case 'F':
        case 'I':
        case 'J':
        case 'S':
        case 'Z':
        case 'V':
            ts->tag = BASE_TYPE_SIGNATURE;
            ts->type->base_type = parse_base_type(sig, pint);
        default:
            ts->tag = FIELD_TYPE_SIGNATURE;
            ts->type->field_type = parse_field_type_signature(sig, pint);
    }
    return ts;
}

static array_type_signature* parse_array_type_signature(string sig, int *pint)
{
    increase(pint);
    type_signature *ts = parse_type_signature(sig, pint);
    array_type_signature *ats = x_alloc(sizeof(array_type_signature));
    ats->component_type = ts;
    return ats;
}

static field_type_signature* parse_field_type_signature(string sig, int *pint)
{
    field_type_signature *ft = x_alloc(sizeof(field_type_signature));
    ft->type = x_alloc(sizeof(type_signature_union));

    char c = char_at_index(sig, pint);
    switch(c)
    {
        case 'L':
            ft->tag = CLASS_TYPE_SIGNATURE;
            ft->type->class_type = parse_class_type_signature(sig, pint);
            break;
        case 'T':
            ft->tag = TYPE_VARIABLE_SIGNATURE;
            ft->type->type_variable = parse_type_variable_signature(sig, pint);
            break;
        case '[':
            ft->tag = ARRAY_TYPE_SIGNATURE;
            ft->type->array_type = parse_array_type_signature(sig, pint);
            break;
        default: {
            DEBUG_PRINT("[parse_field_type_signature] error\n");
            break;
        }
    }
    return ft;
}

static list_object* parse_bounds(string sig, int *pint)
{
    list_object *fts = list_init_object();
    char c = char_at_index(sig, pint);
    if (c == ':')
    {
        increase(pint);
        c = char_at_index(sig, pint);
        if (c == ':')
        {
            bottom_signature *bs = x_alloc(sizeof(bottom_signature));
            type_signature *ts = x_alloc(sizeof(type_signature));
            ts->type = x_alloc(sizeof(type_signature_union));
            ts->tag = BOTTOM_SIGNATURE;
            ts->type->bottom = bs;
            list_insert_object(fts, ts);
            DEBUG_PRINT("[Bottom Signature]\n");
        }
        else
        {
            list_insert_object(fts, parse_field_type_signature(sig, pint));
        }

        c = char_at_index(sig, pint);
        while (c == ':')
        {
            increase(pint);
            list_insert_object(fts, parse_field_type_signature(sig, pint));
            c = char_at_index(sig, pint);
        }
    }
    return fts;
}

static string parse_identifier(string sig, int *pint)
{
    int start = *pint;
    char c = char_at_index(sig, pint);
    while (c != ' ')
    {
        switch (c) {
            case ';':
            case '.':
            case ':':
            case '>':
            case '<':
            {
                int end = *pint;
                int len = end - start + 1;
                string identifier = x_alloc(len);
                memcpy(identifier, sig + start, len-1);
                identifier[len] = '\0';
                DEBUG_PRINT("[identifier]: %d %s\n", len, identifier);
                return identifier;
            }
            default: {
                increase(pint);
                c = char_at_index(sig, pint);
            }
        }
    }
    return NULL;
}

static formal_type_parameter* parse_formal_type_parameter(string sig, int *pint)
{
    formal_type_parameter *ftp = x_alloc(sizeof(formal_type_parameter));

    ftp->name = parse_identifier(sig, pint);

    ftp->bounds = parse_bounds(sig, pint);

    return ftp;
}

static list_object* parse_formal_type_parameters(string sig, int *pint)
{
    if (sig[*pint] == '<')
    {
        list_object *formal_type_parameters = list_init_object();

        increase(pint);
        list_insert_object(formal_type_parameters, parse_formal_type_parameter(sig, pint));

        char c = char_at_index(sig, pint);
        while (c != '>') {
            list_insert_object(formal_type_parameters, parse_formal_type_parameter(sig, pint));
            c = char_at_index(sig, pint);
        }
        increase(pint);
        return formal_type_parameters;
    }
    else
    {
        DEBUG_PRINT("[formal type param]: EMPTY FORMAL TYPE PARAM\n");
        return NULL;
    }
}

static list_object* parse_interfaces(string sig, int *pint)
{
    list_object *list = list_init_object();
    char c = char_at_index(sig, pint);
    while (c == 'L')
    {
        list_insert_object(list, parse_class_type_signature(sig, pint));
        c = char_at_index(sig, pint);
    }
    return list;
}

static list_object* parse_type_signatures(string sig, int *pint)
{
    list_object *list = list_init_object();
    int stop = 0;
    char c = char_at_index(sig, pint);
    while (!stop) {
        switch (c) {
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'S':
            case 'Z':
            case 'L':
            case 'T':
            case '[': {
                list_insert_object(list, parse_type_signature(sig, pint));
                c = char_at_index(sig, pint);
                break;
            }
            default:
                stop = 1;
        }
    }
    return list;
}

static list_object* parse_formal_parameters(string sig, int *pint)
{
    char c = char_at_index(sig, pint);
    if (c != '(')
    {
        DEBUG_PRINT("[parse_formal_parameters] empty signature\n");
        return NULL;
    } else
    {
        increase(pint);
        list_object *list = parse_type_signatures(sig, pint);
        increase(pint);
        return list;
    }
}

static type_signature* parse_return_type(string sig, int *pint)
{
    type_signature *ts = x_alloc(sizeof(type_signature));
    ts->type = x_alloc(sizeof(type_signature_union));
    char c = char_at_index(sig, pint);
    if (c == 'V')
    {
        increase(pint);
        DEBUG_PRINT("[return type] void\n");
        ts->tag = BASE_TYPE_SIGNATURE;
        ts->type->base_type = parse_base_type(sig, pint);
        ts->type->base_type->tag = 8;
        return ts;
    }
    else
    {
        return parse_type_signature(sig, pint);
    }
}

static class_type_signature* parse_throws_signature(string sig, int *pint)
{
    increase(pint);
    return parse_class_type_signature(sig, pint);
}

static list_object* parse_throws_signatures(string sig, int *pint)
{
    list_object *list = list_init_object();
    char c = char_at_index(sig, pint);
    while (c == '^')
    {
        list_insert_object(list, parse_throws_signature(sig, pint));
        c = char_at_index(sig, pint);
    }
    return list;
}

static string formal_type_parameters_to_string(list_object *formal_type_parameters)
{
    if (formal_type_parameters == NULL) return NULL;
    size_t old_len = 1;
    size_t new_len = 1;
    if (formal_type_parameters->size > 0)
    {
        old_len = 3;
        new_len = 3;
    }
    string formal_type = x_alloc(new_len); // \0 or <>\0


    if (formal_type_parameters->size > 0)
        strcat(formal_type, "<");

    for (int i = 0; i < formal_type_parameters->size; ++i)
    {
        formal_type_parameter *ftp = list_get_object(formal_type_parameters, i);
        size_t name_len = strlen(ftp->name);
        new_len = old_len + name_len;
        formal_type = x_realloc(formal_type, old_len, new_len);
        strcat(formal_type, ftp->name);
        if (ftp->bounds->size > 0)
        {
            old_len = new_len;
            new_len = old_len + strlen(" extends ");
            formal_type = x_realloc(formal_type, old_len, new_len);
            strcat(formal_type, " extends ");
        }

        for (int j = 0; j < ftp->bounds->size; ++j)
        {
            field_type_signature *fts = list_get_object(ftp->bounds, j);
            string fts_str = field_type_signature_to_string(fts);
            old_len = new_len;
            new_len = old_len + strlen(fts_str);
            formal_type = x_realloc(formal_type, old_len, new_len);
            strcat(formal_type, fts_str);
            old_len = new_len;
            if (j != ftp->bounds->size - 1)
            {
                old_len = new_len;
                new_len = old_len + 3;
                formal_type = x_realloc(formal_type, old_len, new_len);
                strcat(formal_type, " & ");
                old_len = new_len;
            }
        }

        if (i != formal_type_parameters->size - 1)
        {
            new_len = old_len + 2;
            formal_type = x_realloc(formal_type, old_len, new_len);
            strcat(formal_type, ", ");
            old_len = new_len;
        }
    }
    if (formal_type_parameters->size > 0)
    {
        strcat(formal_type, ">");
    }
    formal_type[new_len-1] = '\0';
    return formal_type;
}

static string interfaces_to_string(class_signature *cs)
{
    if (cs->interfaces == NULL || cs->interfaces->size == 0)
        return NULL;

    size_t old_len = 1;
    size_t new_len = 1;
    if (cs->interfaces->size > 0)
    {
        old_len = 3;
        new_len = 3;
    }
    string interfaces = x_alloc(new_len); // \0 or <>\0

    for (int i = 0; i < cs->interfaces->size; ++i)
    {
        class_type_signature *cts = list_get_object(cs->interfaces, i);
        for (int j = 0; j < cts->path->size; ++j)
        {
            simple_class_type_signature *ss = list_get_object(cts->path, j);
            string sss = simple_class_signature_to_string(ss);
            new_len = old_len + strlen(sss);
            interfaces = x_realloc(interfaces, old_len, new_len);
            strcat(interfaces, sss);
            old_len = new_len;
            if (j != cts->path->size - 1)
            {
                new_len = old_len + 1;
                interfaces = x_realloc(interfaces, old_len, new_len);
                strcat(interfaces, ", ");
                old_len = new_len;
            }
        }
        if (i != cs->interfaces->size - 1)
        {
            new_len = old_len + 2;
            interfaces = x_realloc(interfaces, old_len, new_len);
            strcat(interfaces, ", ");
            old_len = new_len;
        }
    }
    interfaces[new_len-1] = '\0';
    return interfaces;
}

extern class_signature * parse_class_signature(string sig)
{
    int *pint = x_alloc(sizeof(int));
    *pint = 0;

    class_signature *cs = x_alloc(sizeof(class_signature));

    cs->formal_type_parameters = parse_formal_type_parameters(sig, pint);
    cs->base_class = parse_class_type_signature(sig, pint);
    cs->interfaces = parse_interfaces(sig, pint);

    for (int i = 0; i < cs->base_class->path->size; ++i) {
        simple_class_type_signature *ss = list_get_object(cs->base_class->path, i);
        string sss = simple_class_signature_to_string(ss);
        printf("[base class]: %s\n", sss);
    }

    string formal_type_parameters = formal_type_parameters_to_string(cs->formal_type_parameters);
    printf("[class signature]: %s\n", sig);
    printf("[class signature's formal_type parameter]: %s\n", formal_type_parameters);

    string interfaces = interfaces_to_string(cs);
    printf("[class signature's interfaces]: %s\n", interfaces);

    return cs;
}

extern method_signature* parse_method_signature(string sig)
{
    int *pint = x_alloc(sizeof(int));
    *pint = 0;

    method_signature *ms = x_alloc(sizeof(method_signature));

    ms->formal_type_parameters = parse_formal_type_parameters(sig, pint);
    ms->parameter_types = parse_formal_parameters(sig, pint);
    ms->return_type = parse_return_type(sig, pint);
    ms->exception_types = parse_throws_signatures(sig, pint);

    string formal_type = formal_type_parameters_to_string(ms->formal_type_parameters);
    printf("[method signature]: %s\n", sig);
    printf("[formal type parameters]: %s\n", formal_type);

    for (int i = 0; i < ms->parameter_types->size; ++i)
    {
        type_signature *ts = list_get_object(ms->parameter_types, i);
        string ts_str = field_type_signature_to_string(ts);
        printf("[parameter type]: %s\n", ts_str);
    }

    string return_type = field_type_signature_to_string(ms->return_type);
    printf("[return type]: %s\n", return_type);

    return ms;
}
