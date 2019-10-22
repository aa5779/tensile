%{
/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "grammar.h"

extern int yylex(void);
extern void yyerror(const char *msg);
%}

%token TOK_ID

%token TOK_LOCAL
%token TOK_STATIC
%token TOK_CONST
%token TOK_VOLATILE
%token TOK_IMPORT
%token TOK_INTRINSIC
%token TOK_TERM
%token TOK_RETURN
%token TOK_ERROR
%token TOK_META

%token TOK_ARROW

%right '?' '!' '~' '@'
%right '|'
%right '&'
%right TOK_ASSIGN TOK_ASSIGN_OP TOK_ASSIGN_DEFAULT
       TOK_ASSIGN_MAP TOK_ASSIGN_EACH
%nonassoc '=' TOK_NE '<' '>' TOK_LE TOK_GE ':' TOK_IN
%left TOK_MIN TOK_MAX
%precedence TOK_CONCAT
%nonassoc TOK_RANGE
%left TOK_LSHIFT TOK_RSHIFT
%left '+' '-' '#' TOK_BITOR
%left '*' '/' '%' TOK_BITAND '\\'
%left '$'
%left '^'
%right TOK_NEW TOK_SPAWN
%right TOK_UPLUS TOK_UMINUS TOK_DEFER TOK_DEREF TOK_MKSTRING TOK_SIZEOF
       TOK_ALL TOK_EACH TOK_ANCHOR TOK_BITINV TOK_NOT TOK_ERASE
       TOK_ASSIGN_POS TOK_MATCH TOK_MATCH_TYPE '[' TOK_ADDRESS TOK_NODE_HEAD
%precedence TOK_FUNCALL TOK_INDEX '.'
%precedence TOK_INTEGER TOK_FLOAT TOK_CHAR TOK_STRING TOK_BYTESTRING
            TOK_ABORT TOK_CONTINUE TOK_FAIL TOK_SUCCEED TOK_ARB TOK_BAL TOK_REM
            TOK_FENCE '{' '('

%%


module:                 definitions
                        ;

definitions:            %empty
        |               definitions definition ';'
                        ;

definition:             qdefinition
        |               TOK_ERROR termhead
        |               TOK_IMPORT module_expr import_list
        |               TOK_INTRINSIC TOK_ID
        |               TOK_META TOK_ID TOK_STRING
                        ;

qdefinition:            staticlocal0 qdefinition0
                        ;

qdefinition0:           objdef
        |               termdef
        ;

expression:             expression '?' expression
        |               expression '!' expression
        |               expression '~' expression
        |               expression '@' expression
        |               expression '&' expression
        |               expression TOK_ASSIGN expression
        |               expression TOK_ASSIGN_OP expression
        |               expression TOK_ASSIGN_DEFAULT expression
        |               expression TOK_ASSIGN_MAP expression
        |               expression TOK_ASSIGN_EACH expression
        |               expression '=' expression
        |               expression TOK_NE expression
        |               expression '<' expression
        |               expression '>' expression
        |               expression TOK_LE expression
        |               expression TOK_GE expression
        |               expression ':' typedecl
        |               expression TOK_IN expression
        |               expression TOK_MIN expression
        |               expression TOK_MAX expression
        |               expression expression %prec TOK_CONCAT
        |               expression TOK_RANGE expression
        |               expression TOK_LSHIFT expression
        |               expression TOK_RSHIFT expression
        |               expression '+' expression
        |               expression '-' expression
        |               expression '#' expression
        |               expression TOK_BITOR expression
        |               expression '*' expression
        |               expression '/' expression
        |               expression '%' expression
        |               expression TOK_BITAND expression
        |               expression '\\' expression
        |               expression '$' expression
        |               expression '^' expression
        |               TOK_UPLUS expression
        |               TOK_UMINUS expression
        |               TOK_DEFER expression
        |               TOK_DEREF expression
        |               TOK_MKSTRING expression
        |               TOK_SIZEOF expression
        |               TOK_ALL expression
        |               TOK_EACH expression
        |               TOK_ANCHOR expression
        |               TOK_ERASE expression
        |               TOK_BITINV expression
        |               TOK_NOT expression
        |               TOK_ASSIGN_POS expression
        |               TOK_MATCH expression
        |               TOK_MATCH_TYPE expression
        |               expression TOK_FUNCALL arguments ')' %prec TOK_FUNCALL
        |               expression TOK_INDEX expression ']' %prec TOK_INDEX
        |               TOK_ADDRESS expression
        |               TOK_NEW constvolatile0 expression
        |               TOK_SPAWN expression
        |               '[' expression0 ']' expression %prec '['
        |               expression '.' TOK_ID %prec '.'
        |               TOK_NODE_HEAD node
        |               literal
        |               atomic_pattern
        |               '(' expression ')'
                        ;

expression0:            %empty
        |               expression
                        ;

atomic_pattern:         TOK_ARB
        |               TOK_BAL
        |               TOK_ABORT
        |               TOK_FAIL
        |               TOK_SUCCEED
        |               TOK_REM
        |               TOK_FENCE
        |               TOK_CONTINUE
        ;

module_expr:            module_id
        |               TOK_ID TOK_ASSIGN module_id module_inst0
        ;

module_inst0:           TOK_INDEX module_inst_list ']'
        ;

module_inst_list:       module_inst_item
        |               module_inst_list ',' module_inst_item
        ;

module_inst_item:       module_id TOK_ARROW module_id
        ;

module_id:              TOK_ID
        |               module_id '.' TOK_ID
                        ;

import_list:            %empty
        |               TOK_FUNCALL ')'
        |               TOK_FUNCALL import_specs ')'
                        ;

import_specs:           import_spec
        |               import_specs ',' import_spec
                        ;

import_spec:            TOK_ID
        |               TOK_ID TOK_ARROW TOK_ID
        ;

staticlocal0:           %empty
        |               staticlocal
                        ;

staticlocal:            TOK_LOCAL
        |               TOK_STATIC
                        ;

constvolatile0:         %empty
        |               constvolatile
                        ;

constvolatile:          TOK_VOLATILE
        |               TOK_CONST
                        ;

arguments:              %empty
        |               expression_list
        |               named_arguments
                        ;

expression_list:        expression
        |               expression_list ',' expression
                        ;

named_arguments:        named_argument
        |               named_arguments ',' named_argument
                        ;

named_argument:         TOK_ID TOK_ARROW expression
                        ;

literal:                TOK_INTEGER
        |               TOK_FLOAT
        |               TOK_CHAR
        |               TOK_STRING
        |               TOK_BYTESTRING
        |               '{' expression_list '}'
        |               '{' mappings '}'
                        ;

mappings:               mapping
        |               mappings ',' mapping
                        ;

mapping:                key TOK_ARROW expression
                        ;

node:                   key0 '{' node_content '}'
                        ;

key0:                   %empty
        |               key
                        ;

node_content:           %empty
        |               node_mappings
        ;

node_mappings:          node_mapping
        |               node_mappings ',' node_mapping
        ;

node_mapping:           mapping
        |               expression
        ;

key:                    TOK_ID
        |               TOK_STRING
        |               TOK_INTEGER
        |               TOK_CHAR
        |               TOK_BYTESTRING
        |               '(' expression ')'
                        ;



typedecl:               TOK_ID typefunc
        |               TOK_TERM
        |               TOK_ERROR
        |               TOK_ARB
        |               TOK_ADDRESS typedecl
        |               TOK_DEFER typedecl
        |               typedecl '|' typedecl
        |               '{' typedecl0 '}'
        |               '{' typedecl0 TOK_ARROW typedecl0 '}'
        |               '(' typedecl ')'
                        ;

typefunc:               %empty
        |               TOK_ID TOK_FUNCALL type_arguments ')'
        ;

typedecl0:              %empty
        |               typedecl
                        ;

type_arguments:         %empty
        |               named_type_args
        |               typedecl_list
                        ;

typedecl_list:          typedecl
        |               typedecl_list ',' typedecl
                        ;

named_type_args:        named_type
        |               named_type_args ',' named_type
                        ;

named_type:             TOK_ID TOK_ARROW typedecl
                        ;

objdef:                 constvolatile0 TOK_ID initialisation
        ;

initialisation:         %empty
        |               TOK_ASSIGN expression
        ;

termdef:                TOK_TERM augment termhead constructor rewrites
                        ;

augment:                %empty
        |               TOK_MKSTRING
        ;

termhead:               TOK_ID TOK_FUNCALL idlist0 ')'
                        ;

idlist0:                %empty
        |               idlist
                        ;

idlist:                 termarg
        |               idlist ',' termarg
                        ;

termarg:                TOK_ID ':' typedecl
        |               TOK_ID
                        ;

constructor:            %empty
        |               '{' definitions '}'
                        ;

rewrites:               %empty
        |               rule
        |               rewrites ',' termhead rule
        ;

rule:                   TOK_ARROW '{' actions '}'
                        ;

actions:                %empty
        |               actions action
                        ;

action:                 expression target0 ';'
                        ;

target0:                %empty
        |               TOK_ARROW target
                        ;

target:                 TOK_RETURN
        |               TOK_CONTINUE
        |               TOK_FAIL
        |               TOK_ABORT
        ;
