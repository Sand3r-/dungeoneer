#ifndef DATA_DESK_H_INCLUDED_
#define DATA_DESK_H_INCLUDED_

#ifndef DATA_DESK_NO_CRT
#include <stdio.h>
#endif

#if defined(_MSC_VER)
#define DATA_DESK_EXPORT __declspec(dllexport)
#else
#define DATA_DESK_EXPORT
#endif

#if defined(__cplusplus)
#define DATA_DESK_EXTERN_C extern "C"
#else
#define DATA_DESK_EXTERN_C
#endif

#define DATA_DESK_FUNC DATA_DESK_EXPORT DATA_DESK_EXTERN_C





/*
| /////////////////////////////////////////////////////////////////
 |  Callback Structs/Functions
 | /////////////////////////////////////////////////////////////////
 |
 | These are structures and function typedefs that correspond to
 | the callbacks that a custom layer can receive. A custom layer
 | doesn't need to define every possible callback. The symbol
 | names that Data Desk expects for each callback are listed
 | next to the corresponding function typedef.
 */

typedef struct DataDeskASTNode DataDeskASTNode;

typedef struct DataDeskParsedNode
{
    char *name;
    char *name_lowercase_with_underscores;
    char *name_uppercase_with_underscores;
    char *name_lower_camel_case;
    char *name_upper_camel_case;
    char *name_with_spaces;
    DataDeskASTNode *root;
}
DataDeskParsedNode;

typedef DataDeskParsedNode DataDeskConstant;
typedef DataDeskParsedNode DataDeskStruct;
typedef DataDeskParsedNode DataDeskEnum;
typedef DataDeskParsedNode DataDeskFlags;
typedef DataDeskParsedNode DataDeskDeclaration;

/* DataDeskCustomInitCallback */
typedef void DataDeskInitCallback(void);

/* DataDeskCustomFileCallback */
typedef void DataDeskFileCallback(DataDeskASTNode *root, char *filename);

/* DataDeskCustomConstantCallback */
typedef void DataDeskConstantCallback(DataDeskConstant constant, char *filename);

/* DataDeskCustomStructCallback */
typedef void DataDeskStructCallback(DataDeskStruct parsed_struct, char *filename);

/* DataDeskCustomEnumCallback */
typedef void DataDeskEnumCallback(DataDeskEnum parsed_enum, char *filename);

/* DataDeskCustomFlagsCallback */
typedef void DataDeskFlagsCallback(DataDeskFlags parsed_flags, char *filename);

/* DataDeskCustomDeclarationCallback */
typedef void DataDeskDeclarationCallback(DataDeskDeclaration declaration, char *filename);

/* DataDeskCustomCleanUpCallback */
typedef void DataDeskCleanUpCallback(void);





/*
| /////////////////////////////////////////////////////////////////
 |  Abstract Syntax Tree Representation
 | /////////////////////////////////////////////////////////////////
 |
 | The following code outlines the general structure for the
 | abstract syntax trees that Data Desk generates. Each sub-struct
 | contained inside of the DataDeskASTNode struct is only safe to
 | access if the "int type" variable in the struct is set to the
 | corresponding type value. All of the type constants are defined
 | in the following enum.
 */

enum
{
    DATA_DESK_AST_NODE_TYPE_invalid,
    DATA_DESK_AST_NODE_TYPE_identifier,
    DATA_DESK_AST_NODE_TYPE_numeric_constant,
    DATA_DESK_AST_NODE_TYPE_string_constant,
    DATA_DESK_AST_NODE_TYPE_char_constant,
    DATA_DESK_AST_NODE_TYPE_binary_operator,
    DATA_DESK_AST_NODE_TYPE_struct_declaration,
    DATA_DESK_AST_NODE_TYPE_enum_declaration,
    DATA_DESK_AST_NODE_TYPE_flags_declaration,
    DATA_DESK_AST_NODE_TYPE_declaration,
    DATA_DESK_AST_NODE_TYPE_type_usage,
    DATA_DESK_AST_NODE_TYPE_tag,
    DATA_DESK_AST_NODE_TYPE_constant_definition,
};

// NOTE(rjf): These must be in order of precedence
enum
{
    DATA_DESK_BINARY_OPERATOR_TYPE_invalid,
    DATA_DESK_BINARY_OPERATOR_TYPE_add,
    DATA_DESK_BINARY_OPERATOR_TYPE_subtract,
    DATA_DESK_BINARY_OPERATOR_TYPE_multiply,
    DATA_DESK_BINARY_OPERATOR_TYPE_divide,
    DATA_DESK_BINARY_OPERATOR_TYPE_modulus,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitshift_left,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitshift_right,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitwise_and,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitwise_or,
    DATA_DESK_BINARY_OPERATOR_TYPE_boolean_and,
    DATA_DESK_BINARY_OPERATOR_TYPE_boolean_or,
    DATA_DESK_BINARY_OPERATOR_TYPE_MAX
};

typedef struct DataDeskASTNode
{
    int type;
    DataDeskASTNode *next;
    
    char *string;
    int string_length;
    
    DataDeskASTNode *first_tag;
    
    union
    {
        struct BinaryOperator
        {
            int type;
            DataDeskASTNode *left;
            DataDeskASTNode *right;
        }
        binary_operator;
        
        struct StructDeclaration
        {
            DataDeskASTNode *first_member;
        }
        struct_declaration;
        
        struct EnumDeclaration
        {
            DataDeskASTNode *first_constant;
        }
        enum_declaration;
        
        struct FlagsDeclaration
        {
            DataDeskASTNode *first_flag;
        }
        flags_declaration;
        
        struct Declaration
        {
            DataDeskASTNode *type;
            DataDeskASTNode *initialization;
        }
        declaration;
        
        struct TypeUsage
        {
            int pointer_count;
            DataDeskASTNode *first_array_size_expression;
            DataDeskASTNode *struct_declaration;
        }
        type_usage;
        
        struct ConstantDefinition
        {
            DataDeskASTNode *expression;
        }
        constant_definition;
    };
}
DataDeskASTNode;





/*
| /////////////////////////////////////////////////////////////////
 |  Introspection Helper Functions
 | /////////////////////////////////////////////////////////////////
 |
 | The following function prototypes are convenience utility
 | functions that can be helpful when introspecting upon abstract
 | syntax trees.
 */

int DataDeskStringHasSubString(char *tag, char *substring);
char *DataDeskGetTagStringWithSubString(DataDeskASTNode *root, char *tag);
int DataDeskNodeHasTag(DataDeskASTNode *root, char *tag);
int DataDeskConstantHasTag(DataDeskConstant constant_info, char *tag);
int DataDeskStructHasTag(DataDeskStruct struct_info, char *tag);
int DataDeskDeclarationHasTag(DataDeskDeclaration declaration_info, char *tag);
int DataDeskDeclarationIsType(DataDeskASTNode *root, char *type);
int DataDeskStructMemberIsType(DataDeskASTNode *root, char *type);

#ifndef DATA_DESK_NO_CRT
void DataDeskFWriteConstantAsC(FILE *file, DataDeskConstant constant_info);
void DataDeskFWriteStructAsC(FILE *file, DataDeskStruct struct_info);
#endif





/*
| /////////////////////////////////////////////////////////////////
|  Helper Function Implementation Code
| /////////////////////////////////////////////////////////////////
*/

int
DataDeskCharIsAlpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int
DataDeskCharIsDigit(int c)
{
    return (c >= '0' && c <= '9');
}

int
DataDeskStringHasAlphanumericBlock(char *string, char *substring)
{
    int matches = 0;
    
    if(string)
    {
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] == substring[0])
            {
                matches = 1;
                
                int string_i = i+1;
                int substring_i = 1;
                for(;;)
                {
                    if(!substring[substring_i])
                    {
                        if(substring_i > 0 &&
                           (DataDeskCharIsAlpha(substring[substring_i-1]) ||
                            DataDeskCharIsDigit(substring[substring_i-1]) ||
                            substring[substring_i-1] == '_'))
                        {
                            if(DataDeskCharIsAlpha(string[string_i]) ||
                               DataDeskCharIsDigit(string[string_i]) ||
                               string[string_i] == '_')
                            {
                                matches = 0;
                            }
                        }
                        break;
                    }
                    
                    if(string[string_i] != substring[substring_i])
                    {
                        matches = 0;
                        break;
                    }
                    
                    ++string_i;
                    ++substring_i;
                }
            }
            
            if(matches)
            {
                break;
            }
        }
    }
    
    return matches;
}

char *
DataDeskGetTagStringWithSubString(DataDeskASTNode *root, char *tag)
{
    char *str = 0;
    for(DataDeskASTNode *tag_node = root->first_tag;
        tag_node;
        tag_node = tag_node->next)
    {
        if(DataDeskStringHasAlphanumericBlock(tag_node->string, tag))
        {
            str = tag_node->string;
            break;
        }
    }
    return str;
}

int
DataDeskNodeHasTag(DataDeskASTNode *node, char *tag)
{
    char *tag_str = DataDeskGetTagStringWithSubString(node, tag);
    return tag_str != 0;
}

int
DataDeskStructHasTag(DataDeskStruct struct_info, char *tag)
{
    return DataDeskNodeHasTag(struct_info.root, tag);
}

int
DataDeskDeclarationHasTag(DataDeskDeclaration declaration_info, char *tag)
{
    return DataDeskNodeHasTag(declaration_info.root, tag);
}

int
DataDeskDeclarationIsType(DataDeskASTNode *root, char *type)
{
    int pointer_count = 0;
    char *type_name = type;
    
    for(int i = 0; type[i]; ++i)
    {
        if(type[i] == '*')
        {
            ++pointer_count;
        }
        else
        {
            type_name = type+i;
            break;
        }
    }
    
    int matches = pointer_count == root->declaration.type->type_usage.pointer_count;
    if(matches)
    {
        for(int i = 0; type_name[i] && root->declaration.type->string[i]; ++i)
        {
            if(type_name[i] != root->declaration.type->string[i])
            {
                matches = 0;
                break;
            }
        }
    }
    
    return matches;
}

int
DataDeskStructMemberIsType(DataDeskASTNode *root, char *type)
{
    return DataDeskDeclarationIsType(root, type);
}

#ifndef DATA_DESK_NO_CRT
void
_DataDeskFWriteASTFromRootAsC(FILE *file, DataDeskASTNode *root, int follow_next, int nest)
{
    if(root)
    {
        switch(root->type)
        {
            case DATA_DESK_AST_NODE_TYPE_identifier:
            case DATA_DESK_AST_NODE_TYPE_numeric_constant:
            case DATA_DESK_AST_NODE_TYPE_string_constant:
            case DATA_DESK_AST_NODE_TYPE_char_constant:
            {
                fprintf(file, "%s", root->string);
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_binary_operator:
            {
                fprintf(file, "(");
                _DataDeskFWriteASTFromRootAsC(file, root->binary_operator.left, 0, nest);
                char *binary_operator_string = "";
                
                switch(root->binary_operator.type)
                {
                    case DATA_DESK_BINARY_OPERATOR_TYPE_add: { binary_operator_string = "+"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_subtract: { binary_operator_string = "-"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_multiply: { binary_operator_string = "*"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_divide: { binary_operator_string = "/"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_modulus: { binary_operator_string = "%"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_bitshift_left: { binary_operator_string = "<<"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_bitshift_right: { binary_operator_string = ">>"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_bitwise_and: { binary_operator_string = "&"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_bitwise_or: { binary_operator_string = "|"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_boolean_and: { binary_operator_string = "&&"; break; }
                    case DATA_DESK_BINARY_OPERATOR_TYPE_boolean_or: { binary_operator_string = "||"; break; }
                    default: break;
                }
                
                fprintf(file, "%s", binary_operator_string);
                _DataDeskFWriteASTFromRootAsC(file, root->binary_operator.right, 0, nest+1);
                fprintf(file, ")");
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_struct_declaration:
            {
                if(root->string)
                {
                    fprintf(file, "struct %s\n{\n", root->string);
                }
                else
                {
                    fprintf(file, "struct\n{\n");
                }
                
                for(DataDeskASTNode *member = root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    _DataDeskFWriteASTFromRootAsC(file, member, 0, nest+1);
                    fprintf(file, ";\n");
                }
                fprintf(file, "}");
                
                if(nest == 0)
                {
                    fprintf(file, ";\n\n");
                }
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_enum_declaration:
            {
                if(root->string)
                {
                    fprintf(file, "enum %s\n{\n", root->string);
                }
                else
                {
                    fprintf(file, "enum\n{\n");
                }
                
                for(DataDeskASTNode *member = root->enum_declaration.first_constant;
                    member;
                    member = member->next)
                {
                    fprintf(file, "%s", member->string);
                    fprintf(file, ",\n");
                }
                fprintf(file, "}");
                
                fprintf(file, ";\n\n");
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_flags_declaration:
            {
                int needed_bits_for_flag_type = 32;
                int current_bit = 0;
                for(DataDeskASTNode *member = root->flags_declaration.first_flag;
                    member;
                    member = member->next)
                {
                    fprintf(file, "#define %s (1<<%i)\n", member->string, current_bit);
                    ++current_bit;
                }
                
                if(current_bit >= 31)
                {
                    needed_bits_for_flag_type = 64;
                }
                
                if(root->string)
                {
                    if(needed_bits_for_flag_type == 32)
                    {
                        fprintf(file, "typedef unsigned int %s;\n\n", root->string);
                    }
                    else if(needed_bits_for_flag_type > 32)
                    {
                        fprintf(file, "typedef unsigned long int %s;\n\n", root->string);
                    }
                }
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_declaration:
            {
                _DataDeskFWriteASTFromRootAsC(file, root->declaration.type, 0, nest+1);
                fprintf(file, "%s", root->string);
                
                for(DataDeskASTNode *array = root->declaration.type->type_usage.first_array_size_expression;
                    array;
                    array = array->next)
                {
                    fprintf(file, "[");
                    _DataDeskFWriteASTFromRootAsC(file, array, 0, nest);
                    fprintf(file, "]");
                }
                
                break;
            }
            
            case DATA_DESK_AST_NODE_TYPE_type_usage:
            {
                if(root->type_usage.struct_declaration)
                {
                    _DataDeskFWriteASTFromRootAsC(file, root->type_usage.struct_declaration, 0, nest+1);
                    fprintf(file, "\n");
                }
                else
                {
                    fprintf(file, "%s ", root->string);
                }
                
                for(int i = 0; i < root->type_usage.pointer_count; ++i)
                {
                    fprintf(file, "*");
                }
                
                break;
            }
            
            default: break;
        }
        
        if(root->next && follow_next)
        {
            _DataDeskFWriteASTFromRootAsC(file, root->next, follow_next, nest);
        }
    }
}

void
DataDeskFWriteASTFromRootAsC(FILE *file, DataDeskASTNode *root, int follow_next)
{
    _DataDeskFWriteASTFromRootAsC(file, root, follow_next, 0);
}

void
DataDeskFWriteConstantAsC(FILE *file, DataDeskConstant constant_info)
{
    fprintf(file, "#define %s (", constant_info.name);
    DataDeskFWriteASTFromRootAsC(file, constant_info.root->constant_definition.expression, 0);
    fprintf(file, ")\n");
}

void
DataDeskFWriteStructAsC(FILE *file, DataDeskStruct struct_info)
{
    fprintf(file, "typedef struct %s %s;\n", struct_info.name, struct_info.name);
    DataDeskFWriteASTFromRootAsC(file, struct_info.root, 0);
}

#endif // DATA_DESK_NO_CRT

#endif // DATA_DESK_H_INCLUDED_