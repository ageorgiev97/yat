%code requires {
	#include <iostream>

    #include <Expressions.hpp>
    #include <Statements.hpp>
    #include <Block.hpp>
    #include <Function.hpp>

    extern yat::Statement last_statement;
    
    #define YY_DECL yat::parser::symbol_type yylex()

	void yyerror(std::string_view message);
}

%{
    #include "parser.hpp"

    #include <Expressions.hpp>
    #include <Statements.hpp>

    using namespace yat;

    extern YY_DECL;

    Statement last_statement;
%}

%language "c++"

%define api.token.constructor
%define api.value.type variant
%define api.namespace {yat}
%define parse.error verbose

%token <std::string> IDENTIFIER
%token <int>         INTEGER
%token <float>       FLOAT

%token END 0

%token OR
%token AND
%token EQUALS
%token ASSIGN
%token NOT_EQUALS 
%token ADD
%token SUB
%token MUL
%token DIV
%token PARENTHESIS_OPEN
%token PARENTHESIS_CLOSE
%token BRACKET_OPEN
%token BRACKET_CLOSE
%token BRACE_OPEN
%token BRACE_CLOSE
%token COMMA
%token SEMICOLON
%token RETURN
%token IF
%token ELSE

%type <Expression> Expression
%type <Expression> And
%type <Expression> Equality
%type <Expression> Sum
%type <Expression> Product
%type <Expression> Call
%type <Expression> Term

%type <Statement>             Statement
%type <Statement>             SimpleStatement
%type <Statement>             BlockSeparatorStatement

%type <ReturnExpression>      ReturnExpression
%type <ReturnVoid>            ReturnVoid
%type <ScopeStatement>        ScopeStatement
%type <IfStatement>           IfStatement

%type <ExpressionStatement>   ExpressionStatement
%type <DeclarationAssignment> DeclarationAssignment
%type <DeclarationStatement>  DeclarationStatement
%type <AssignmentStatement>   AssignmentStatement

%type <Block>              Block
%type <std::vector<Block>> BlockList
%type <std::vector<Block>> FunctionBody

%type <std::vector<Statement>> SimpleStatementList

%start Start

%%

Start: Statement { last_statement = std::move($1); };

Statement: 
    SimpleStatement { $$ = std::move($1); } |
    BlockSeparatorStatement { $$ = std::move($1); };

Expression:
    Expression OR And { $$ = BinaryExpression(Operator::Or, std::move($1), std::move($3)); } |
    And { $$ = std::move($1); };

And:
    And AND Equality { $$ = BinaryExpression(Operator::And, std::move($1), std::move($3)); } |
    Equality { $$ = std::move($1); };

Equality:
    Equality EQUALS     Sum { $$ = BinaryExpression(Operator::Equals, std::move($1), std::move($3)); } |
    Equality NOT_EQUALS Sum { $$ = BinaryExpression(Operator::NotEquals, std::move($1), std::move($3)); } |
    Sum { $$ = std::move($1); };

Sum:
    Sum ADD Product { $$ = BinaryExpression(Operator::Add, std::move($1), std::move($3)); } |
    Sum SUB Product { $$ = BinaryExpression(Operator::Sub, std::move($1), std::move($3)); } |
    Product { $$ = std::move($1); };

Product:
    Product MUL Call { $$ = BinaryExpression(Operator::Mul, std::move($1), std::move($3)); } |
    Product DIV Call { $$ = BinaryExpression(Operator::Div, std::move($1), std::move($3)); } |
    Call { $$ = std::move($1); };

Call:
    Call PARENTHESIS_OPEN ArgumentList PARENTHESIS_CLOSE |
    Call BRACKET_OPEN ArgumentList BRACKET_CLOSE  |
    Term { $$ = std::move($1); };

Term:
    IDENTIFIER { $$ = IdentifierLiteral(std::move($1)); } |
    INTEGER    { $$ = IntLiteral($1); } |
    FLOAT      { $$ = FloatLiteral($1); } |
    PARENTHESIS_OPEN Expression PARENTHESIS_CLOSE { $$ = std::move($2); } |
    BRACKET_OPEN     Expression BRACKET_CLOSE     { $$ = Cast(std::move($2)); };

ArgumentList:
    ArgumentList COMMA Expression |
    Expression |
    %empty;

Block:
    SimpleStatementList BlockSeparatorStatement {
        $$.statements = std::move($1);
        $$.statements.push_back(std::move($2));
    } |
    BlockSeparatorStatement { $$.statements = { std::move($1) }; };

BlockList:
    BlockList Block {
        $$ = std::move($1);
        $$.push_back(std::move($2));
    } |
    Block { $$ = { std::move($1) }; };

FunctionBody:
    BlockList { $$ = std::move($1); } |
    BlockList SimpleStatementList {
        $$ = std::move($1);
        Block b;
        b.statements = std::move($2);
        b.statements.push_back(DefaultBlockEnd());
        $$.push_back(std::move(b));
    } |
    SimpleStatementList {
        Block b;
        b.statements = std::move($1);
        b.statements.push_back(DefaultBlockEnd());
        $$.push_back(std::move(b));
    };

SimpleStatementList:
    SimpleStatementList SimpleStatement {
        $$ = std::move($1);
        $$.push_back(std::move($2));
    } |
    SimpleStatement { $$ = { std::move($1) }; };;

SimpleStatement:
    ExpressionStatement { $$ = std::move($1); } |
    DeclarationAssignment { $$ = std::move($1); } |
    DeclarationStatement { $$ = std::move($1); } |
    AssignmentStatement { $$ = std::move($1); };
    
BlockSeparatorStatement:
    ReturnExpression { $$ = std::move($1); } |
    ReturnVoid { $$ = std::move($1); } |
    ScopeStatement { $$ = std::move($1); } |
    IfStatement { $$ = std::move($1); };

DeclarationAssignment:
    IDENTIFIER IDENTIFIER ASSIGN Expression SEMICOLON {
        yat::Type type;
        if ($1 == "i32" || $1 == "bool") {
            type = Type::INT;
        } else if ($1 == "float") {
            type = Type::FLOAT;
        } else {
            type = Type::ERROR;
        }

        $$ = DeclarationAssignment(
            DeclarationStatement(type, $2),
            AssignmentStatement($2, std::move($4))
        );
    };

DeclarationStatement:
    IDENTIFIER IDENTIFIER SEMICOLON {
        yat::Type type;
        if ($1 == "i32" || $1 == "bool") {
            type = Type::INT;
        } else if ($1 == "float") {
            type = Type::FLOAT;
        } else {
            type = Type::ERROR;
        }

        $$ = DeclarationStatement(type, std::move($2));
    };

AssignmentStatement:
    IDENTIFIER ASSIGN Expression SEMICOLON { $$ = AssignmentStatement(std::move($1), std::move($3)); };

ExpressionStatement:
    Expression SEMICOLON { $$ = std::move($1); };
     
ReturnExpression:
    RETURN Expression SEMICOLON { $$ = std::move($2); };

ReturnVoid:
    RETURN SEMICOLON { };

ScopeStatement:
    BRACE_OPEN BlockList BRACE_CLOSE { $$ = std::move($2); } |
    BRACE_OPEN BRACE_CLOSE { };

IfStatement:
    IF PARENTHESIS_OPEN Expression PARENTHESIS_CLOSE Statement {
        $$ = IfStatement(std::move($3), { std::move($5) });
    } |
    IF PARENTHESIS_OPEN Expression PARENTHESIS_CLOSE Statement ELSE Statement {
        $$ = IfStatement(std::move($3), { std::move($5), std::move($7) });
    }
%%

void parser::error(std::string const& message)
{
	std::cerr << message << std::endl;
}