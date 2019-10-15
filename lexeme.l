%{
#include "grammar.h"
%}

%s FUNCALL
SP [[:space:]]+
ESCAPE \\([abefnrtvz\\\'\"]|u[[:xdigit:]]{4}|U[[:xdigit:]]{8}|[uU]\{[^}]+\})
%%
new { BEGIN(INITIAL); return TOK_NEW; }
arb { BEGIN(INITIAL); return TOK_ARB; }
bal { BEGIN(FUNCALL); return TOK_BAL; }
abort { BEGIN(INITIAL); return TOK_ABORT; }
fail { BEGIN(INITIAL); return TOK_FAIL; }
rem { BEGIN(INITIAL); return TOK_REM; }
fence { BEGIN(FUNCALL); return TOK_FENCE; }
continue { BEGIN(INITIAL); return TOK_CONTINUE; }
return { BEGIN(INITIAL); return TOK_RETURN; }
spawn  { BEGIN(INITIAL); return TOK_SPAWN; }

static { BEGIN(INITIAL); return TOK_STATIC; }
local { BEGIN(INITIAL); return TOK_LOCAL; }
const { BEGIN(INITIAL); return TOK_STATIC; }
volatile { BEGIN(INITIAL); return TOK_LOCAL; }
import { BEGIN(INITIAL); return TOK_IMPORT; }
intrinsic { BEGIN(INITIAL); return TOK_INTRINSIC; }

term { BEGIN(FUNCALL); return TOK_TERM; }

meta { BEGIN(INITIAL); return TOK_META; }

[[:alpha:]_][[:alnum:]_]* { BEGIN(FUNCALL); return TOK_ID; }
[+-]?[0-9]+ { BEGIN(INITIAL); return TOK_INTEGER; }
[+-]?([0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+) { BEGIN(INITIAL); return TOK_FLOAT; }
\'([^\'\\]+|{ESCAPE})\' { BEGIN(INITIAL); return TOK_CHAR; }
\"([^\"\\]+|{ESCAPE}|\\\n)*\" { BEGIN(INITIAL); return TOK_STRING; }
0[xX][[:xdigit:]]+ { BEGIN(INITIAL); return TOK_BYTESTRING; }
[-+*=~#$^/%<>:&|?!\\@]{SP} { BEGIN(INITIAL); return *yytext; }
\\\/{SP} { BEGIN(INITIAL); return TOK_BITOR; }
\/\\{SP} { BEGIN(INITIAL); return TOK_BITAND; }
\<:{SP} { BEGIN(INITIAL); return TOK_MIN; }
:>{SP} { BEGIN(INITIAL); return TOK_MAX; }
:={SP} { BEGIN(INITIAL); return TOK_ASSIGN; }
[-+*/%^\\]={SP} { BEGIN(INITIAL); return TOK_ASSIGN_OP; }
\.={SP} { BEGIN(INITIAL); return TOK_ASSIGN_DEFAULT; }
!={SP} { BEGIN(INITIAL); return TOK_ASSIGN_EACH; }
\?={SP} { BEGIN(INITIAL); return TOK_ASSIGN_MAP; }
(\<\<|\>\>|<:|:>|\\\/|\/\\)={SP} { BEGIN(INITIAL); return TOK_ASSIGN_OP; }
\<\<{SP} { BEGIN(INITIAL); return TOK_LSHIFT; }
\>\>{SP} { BEGIN(INITIAL); return TOK_RSHIFT; }
\<={SP} { BEGIN(INITIAL); return TOK_LE; }
\>={SP} { BEGIN(INITIAL); return TOK_GE; }
\<\>{SP} { BEGIN(INITIAL); return TOK_NE; }
\<- { BEGIN(INITIAL); return TOK_IN; }
<FUNCALL>\. { BEGIN(INITIAL); return '.'; }
<FUNCALL>\( { BEGIN(INITIAL); return TOK_FUNCALL; }
<FUNCALL>\[ { BEGIN(INITIAL); return TOK_INDEX; }
[])}] { BEGIN(FUNCALL); return *yytext; }
[[({,;] { BEGIN(INITIAL); return *yytext; }
\.\. { BEGIN(INITIAL); return TOK_RANGE; }
\+ { BEGIN(INITIAL); return TOK_UPLUS; }
- { BEGIN(INITIAL); return TOK_UMINUS; }
& { BEGIN(INITIAL); return TOK_ANCHOR; }
\$ { BEGIN(INITIAL); return TOK_DEREF; }
\| { BEGIN(INITIAL); return TOK_EACH; }
\* { BEGIN(INITIAL); return TOK_DEFER; }
\\ { BEGIN(INITIAL); return TOK_BITINV; }
\^ { BEGIN(INITIAL); return TOK_MKSTRING; }
! { BEGIN(INITIAL); return TOK_ALL; }
= { BEGIN(INITIAL); return TOK_MATCH; }
: { BEGIN(INITIAL); return TOK_MATCH_TYPE; }
\? { BEGIN(INITIAL); return TOK_ERASE; }
~ { BEGIN(INITIAL); return TOK_NOT; }
\. { BEGIN(INITIAL); return TOK_ADDRESS; }
\# { BEGIN(INITIAL); return TOK_SIZEOF; }
@ { BEGIN(INITIAL); return TOK_ASSIGN_POS; }
\% { BEGIN(INITIAL); return TOK_NODE_HEAD; }
=> { BEGIN(INITIAL); return TOK_ARROW; }

{SP} { BEGIN(INITIAL); }
\/\/.*\n { BEGIN(INITIAL); }
#[#!].*\n { BEGIN(INITIAL); }
