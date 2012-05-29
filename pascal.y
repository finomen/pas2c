%{
/*
 * grammar.y
 *
 * Pascal grammar in Yacc format, based originally on BNF given
 * in "Standard Pascal -- User Reference Manual", by Doug Cooper.
 * This in turn is the BNF given by the ANSI and ISO Pascal standards,
 * and so, is PUBLIC DOMAIN. The grammar is for ISO Level 0 Pascal.
 * The grammar has been massaged somewhat to make it LALR, and added
 * the following extensions.
 *
 * constant expressions
 * otherwise statement in a case
 * productions to correctly match else's with if's
 * beginnings of a separate compilation facility
 */

%}

%token AND ARRAY ASSIGNMENT CASE CHARACTER_STRING COLON COMMA CONST DIGSEQ
%token DIV DO DOT DOTDOT DOWNTO ELSE END EQUAL EXTERNAL FOR FORWARD FUNCTION
%token GE GOTO GT IDENTIFIER IF IN LABEL LBRAC LE LPAREN LT MINUS MOD NIL NOT
%token NOTEQUAL OF OR OTHERWISE PACKED PBEGIN PFILE PLUS PROCEDURE PROGRAM RBRAC
%token REALNUMBER RECORD REPEAT RPAREN SEMICOLON SET SLASH STAR STARSTAR THEN
%token TO TYPE UNTIL UPARROW VAR WHILE WITH

%%



record_type : RECORD record_section_list END
 | RECORD record_section_list semicolon variant_part END
 | RECORD variant_part END
 ;

record_section_list : record_section_list semicolon record_section
 | record_section
 ;

record_section : identifier_list COLON type_denoter
 ;

variant_part : CASE variant_selector OF variant_list semicolon
 | CASE variant_selector OF variant_list
 |
 ;

variant_selector : tag_field COLON tag_type
 | tag_type
 ;

variant_list : variant_list semicolon variant
 | variant
 ;

variant : case_constant_list COLON LPAREN record_section_list RPAREN
 | case_constant_list COLON LPAREN record_section_list semicolon
  variant_part RPAREN
 | case_constant_list COLON LPAREN variant_part RPAREN
 ;


tag_field : identifier ;

tag_type : identifier ;

set_type : SET OF base_type
 ;

base_type : ordinal_type ;

file_type : PFILE OF component_type
 ;


