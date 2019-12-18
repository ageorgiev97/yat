%option noyywrap

%{
    #include <any>
    #include <iostream>

    #include <parser.hpp>
    #include <Expressions.hpp>
    #include <Statements.hpp>
    #include <Generator.hpp>

    using namespace yat;
    
    extern Statement last_statement;
%}

Identifier [_a-zA-Z][_a-zA-Z0-9]*
Integer    [0-9]+
Float      [0-9]+\.[0-9]+
 
Whitespace [ \t\n\r]+

CommentBlockStart [/][*]
CommentBlockStop  [*][/]

Anything .

%x BLOCK_COMMENT

%%

<*>{Whitespace}

{CommentBlockStart} {
    BEGIN(BLOCK_COMMENT);
}

<BLOCK_COMMENT>{Anything}

<BLOCK_COMMENT>{CommentBlockStop} {
    BEGIN(INITIAL);
}

[/]{2}.*$

ret  { return parser::make_RETURN(); }
else { return parser::make_ELSE(); }
if   { return parser::make_IF(); }

{Identifier} { 
    std::string text = yytext;

    if (text == "false") {
        return parser::make_INTEGER(0);
    } else if (text == "true") {
        return parser::make_INTEGER(1);
    }

    return parser::make_IDENTIFIER(std::move(text));
}

{Integer}    { return parser::make_INTEGER(std::stoi(yytext)); }
{Float}      { return parser::make_FLOAT(std::stof(yytext));   }

[|]{2} { return parser::make_OR(); }
[&]{2} { return parser::make_AND(); }
[=]{2} { return parser::make_EQUALS(); }
[!][=] { return parser::make_NOT_EQUALS(); }
[=]    { return parser::make_ASSIGN(); }
[+]    { return parser::make_ADD(); }
[-]    { return parser::make_SUB(); }
[*]    { return parser::make_MUL(); }
[/]    { return parser::make_DIV(); }
[(]    { return parser::make_PARENTHESIS_OPEN(); }
[)]    { return parser::make_PARENTHESIS_CLOSE(); }
[\[]   { return parser::make_BRACKET_OPEN(); }
[\]]   { return parser::make_BRACKET_CLOSE(); }
[{]    { return parser::make_BRACE_OPEN(); }
[}]    { return parser::make_BRACE_CLOSE(); }
[,]    { return parser::make_COMMA(); }
[;]    { return parser::make_SEMICOLON(); }

<<EOF>> { return parser::make_END(); }

%%

int main()
{
    parser p;
    p.parse();
    GenerateCode(last_statement);
    return 0;
}
