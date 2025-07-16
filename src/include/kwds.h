#ifndef KWDS_H_INCLUDED
#define KWDS_H_INCLUDED

#define KW_RETURN  "return"
#define KW_TYPEDEF "typedef"
#define KW_STRUCT  "struct"
#define KW_ENUM    "enum"
#define KW_UNION   "union"

#define KWDS {                                  \
                KW_RETURN,                      \
                KW_TYPEDEF,                     \
                KW_STRUCT,                      \
                KW_ENUM,                        \
                KW_UNION,                       \
        }

#define TYPE_INT "int"
#define TYPE_VOID "void"
#define TYPE_CHAR "char"

#define TYPES {                                 \
                TYPE_INT,                       \
                TYPE_VOID,                      \
                TYPE_CHAR,                      \
        }

#endif // KWDS_H_INCLUDED
