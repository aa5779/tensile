%s FUNCALL
%%
[[:alpha:]_][[:alnum:]_]* { BEGIN(FUNCALL); return TOK_ID; }
[+-]?[0-9]+ { BEGIN(INITIAL); return TOK_INTEGER; }
[+-]?([0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+) { BEGIN(INITIAL); return TOK_FLOAT; }
'([^'\\]+|\\.)*' |
"([^'\\]+|\\.)*" { BEGIN(FUNCALL); return TOK_STRING; }
[-+*=#$^/%<>:&|?!][[:space:]]+ { BEGIN(INITIAL); return *yytext; }
:=[[:space:]]+ { BEGIN(INITIAL); return TOK_ASSIGN; }
[+-*/%^&]=[[:space:]]+ { BEGIN(INITIAL; return TOK_ASSIGN_OP; }
\<\<[[:space:]]+ { BEGIN(INITIAL); return TOK_LSHIFT; }
\>\>[[:space:]]+ { BEGIN(INITIAL); return TOK_RSHIFT; }
\<=[[:space:]]+ { BEGIN(INITIAL); return TOK_LE; }
\>=[[:space:]]+ { BEGIN(INITIAL); return TOK_GE; }
\<\>[[:space:]]+ { BEGIN(INITIAL); return TOK_NE; }
<FUNCALL>\. { BEGIN(INITIAL); return TOK_ACCESS; }
<FUNCALL>\( { BEGIN(INITIAL); return TOK_FUNCALL; }
[])] { BEGIN(FUNCALL); return *yytext; }
[[(.{}] { BEGIN(INITIAL); return *yytext; }
+ { BEGIN(INITIAL); return TOK_UPLUS; }
- { BEGIN(INITIAL); return TOK_UMINUS; }
& { BEGIN(INITIAL); return TOK_GLOBAL; }
\$ { BEGIN(INITIAL); return TOK_DEREF; }
