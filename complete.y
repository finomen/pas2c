identifier_list : identifier_list comma identifier
 | identifier
 ;

label : DIGSEQ
 ;

label_list : label_list comma label
 | label
 ;

label_declaration_part : LABEL label_list semicolon
 |
 ;

constant_definition : identifier EQUAL cexpression semicolon
 ;

constant_list : constant_list constant_definition
 | constant_definition
 ;

constant_definition_part : CONST constant_list
 |
 ;

cexpression : csimple_expression
 | csimple_expression relop csimple_expression
 ;

csimple_expression : cterm
 | csimple_expression addop cterm
 ;

cterm : cfactor
 | cterm mulop cfactor
 ;

cfactor : sign cfactor
 | cexponentiation
 ;

 cexponentiation : cprimary
 | cprimary STARSTAR cexponentiation
 ;
 
 
 cprimary : identifier
 | LPAREN cexpression RPAREN
 | unsigned_constant
 | NOT cprimary
 ;
 
 unsigned_constant : unsigned_number
 | CHARACTER_STRING
 | NIL
 ;
 
unsigned_number : unsigned_integer | unsigned_real ;

unsigned_integer : DIGSEQ
 ;

unsigned_real : REALNUMBER
 ;
 
 
 file : program
 | module
 ;
 
 
 program : program_heading semicolon block DOT
 
 program_heading : PROGRAM identifier
 | PROGRAM identifier LPAREN identifier_list RPAREN
 ;
 
 
 block : label_declaration_part
 constant_definition_part
 type_definition_part
 variable_declaration_part
 procedure_and_function_declaration_part
 statement_part
 ;
 
 module : constant_definition_part
 type_definition_part
 variable_declaration_part
 procedure_and_function_declaration_part
 ;
 
constant : non_string
 | sign non_string
 | CHARACTER_STRING
 ;

sign : PLUS
 | MINUS
 ;

non_string : DIGSEQ
 | identifier
 | REALNUMBER
 ;
 
 type_definition_part : TYPE type_definition_list
 |
 ;

type_definition_list : type_definition_list type_definition
 | type_definition
 ;

type_definition : identifier EQUAL type_denoter semicolon

type_denoter : identifier
 | new_type
 ;
 
 new_type : new_ordinal_type
 | new_structured_type
 | new_pointer_type
 ;
 
 new_pointer_type : UPARROW domain_type
 ;

domain_type : identifier ;
 
 new_ordinal_type : enumerated_type
 | subrange_type
 ;
 
 enumerated_type : LPAREN identifier_list RPAREN
 ;
 
 subrange_type : constant DOTDOT constant
 ;
 
 
new_structured_type : structured_type
 | PACKED structured_type
 ;
 
 structured_type : array_type
 | record_type
 | set_type
 | file_type
 ;
 
 array_type : ARRAY LBRAC index_list RBRAC OF component_type
 ;
 
 index_list : index_list comma index_type
 | index_type
 ;

index_type : ordinal_type ;

ordinal_type : new_ordinal_type
 | identifier
 ;
 
 component_type : type_denoter ;