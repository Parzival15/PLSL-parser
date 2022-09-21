/* Definitions and some functions implementations
 * parse.cpp to be completed
 * Programming Assignment 2
 * Spring 2022
*/

#include "parse.h"

map<string, bool> defVar;
map<string, Token> SymTable;

namespace Parser{
    bool pushed_back = false;
    LexItem pushed_token;
    
    static LexItem GetNextToken(istream &in, int &line){
        if (pushed_back){
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }
    
    static void PushBackToken(LexItem &t){
        if (pushed_back){
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }
    
}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;
}


bool checkVar(string lexeme, int &line)
{

    if (!(defVar.find(lexeme)->second))
    {
        ParseError(line, "Using Undefined Variable");
        return false;
    }
    return true;
}

bool addVar(string lexeme, int &line){
   
    if (!(defVar.find(lexeme) == defVar.end()))
    {
        ParseError(line, "Variable Redefinition");
        return false;
    }
    defVar.insert({lexeme, true});
    return true;
}

bool Prog(istream &in, int &line)
{

    LexItem tk = Parser::GetNextToken(in, line);
    if (tk != PROGRAM)
    {
        ParseError(line, "Missing PROGRAM.");
        return false;
    }
    tk = Parser::GetNextToken(in, line);
    if (tk != IDENT)
    {
        ParseError(line, "Missing Program Name");
        return false;
    }

    tk = Parser::GetNextToken(in, line);
    if (tk != SEMICOL)
    {
        ParseError(line, "no semicolon");
        return false;
    }

    if (!DeclBlock(in, line))
    {
        ParseError(line, "error in decl body");
        return false;
    };
    if (!ProgBody(in, line))
    {
        ParseError(line, "error in prog body");
        return false;
    };

    return true;
}

bool DeclBlock(istream &in, int &line)
{
    LexItem tk = Parser::GetNextToken(in, line);
   
    if (tk != VAR)
    {
        ParseError(line, "Non-recognizable Declaration Block.");
        return false;
    }

    Token token = SEMICOL;
    while (token == SEMICOL)
    {
        bool decl = DeclStmt(in, line);
        tk = Parser::GetNextToken(in, line);
        token = tk.GetToken();
        
        if (tk == BEGIN)
        {
            Parser::PushBackToken(tk);
            break;
        }
        if (!decl){
            ParseError(line, "decl block error");
            return false;
        }
        if (tk != SEMICOL)
        {
            ParseError(line, "semi colon missing");
            return false;
        }
    }

    return true;
}
bool DeclStmt(istream &in, int &line)
{
    LexItem tk = Parser::GetNextToken(in, line);
    if (tk == BEGIN)
    {
        Parser::PushBackToken(tk);
        return false;
    }
    while (tk == IDENT)
    {
        if (!addVar(tk.GetLexeme(), line)){
            ParseError(line, "Incorrect variable in Declaration Statement.");
            return false;
        }
        tk = Parser::GetNextToken(in, line);
        if (tk == COLON)
        {
            break;
        }
        if (tk != COMMA)
        {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tk.GetLexeme() << ")" << endl;
            return false;
        }
        tk = Parser::GetNextToken(in, line);
    }

    tk = Parser::GetNextToken(in, line);
    

    if (!((tk == INTEGER) || (tk == REAL) || (tk == STRING)))
    {
        ParseError(line, "Incorrect Declaration Type");
        return false;
    }

    return true;
}
bool ProgBody(istream &in, int &line)
{
    LexItem tk = Parser::GetNextToken(in, line);
    

    if (tk != BEGIN)
    {
        ParseError(line, "begin missing");
        return false;
    }

    Token token = SEMICOL;
    while (token == SEMICOL)
    {
        bool stmt = Stmt(in, line);
        tk = Parser::GetNextToken(in, line);
        token = tk.GetToken();
        
        if (tk.GetToken() == END)
        {
            Parser::PushBackToken(tk);
            break;
        }
        if (!stmt){
            ParseError(line, "Syntactic error in Program Body.");
            return false;
        }
        if (tk != SEMICOL)
        {
            ParseError(line, "Missing semicolon in Statement.");
            return false;
        }
    }

    tk = Parser::GetNextToken(in, line);

    if (tk.GetToken() != END)
    {
        ParseError(line, "end not found");
        return false;
    }

    return true;
}
bool Stmt(istream &in, int &line){
    bool status;
    
    LexItem t = Parser::GetNextToken(in, line);
    
    switch (t.GetToken()) {
    case WRITELN:
        status = WriteLnStmt(in, line);
        break;
    case IF:
        status = IfStmt(in, line);
        break;
    case IDENT:
        status = AssignStmt(in, line);
        break;
    case FOR:
        status = ForStmt(in, line);
        break;
    default:
        Parser::PushBackToken(t);
        return false;
    }
    return status;
}

bool WriteLnStmt(istream &in, int &line){
    LexItem t;
    
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN){
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    bool ex = ExprList(in, line);
    if (!ex){
        ParseError(line, "Missing expression after WriteLn");
        return false;}
    t = Parser::GetNextToken(in, line);
    if (t != RPAREN){
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    return ex;
}
bool ExprList(istream &in, int &line){
    bool expr = Expr(in, line);
    if (!expr){
        ParseError(line, "wring expression");
        return false;
    }

    LexItem tk = Parser::GetNextToken(in, line);
    while (tk == COMMA){

        expr = Expr(in, line);
        if (!expr) {
            ParseError(line, "wring expression");
            return false;
        }
        tk = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tk);
    return true;
}
bool ForStmt(istream &in, int &line){
    bool var = Var(in, line);

    if (!var){
        ParseError(line, "wrong var");
        return false;
    }

    LexItem tk = Parser::GetNextToken(in, line);

    if (tk != ASSOP) {
        ParseError(line, "assignment operation missing");
        return false;
    }

    tk = Parser::GetNextToken(in, line);

    if (tk != ICONST){
        ParseError(line, "Missing Initialization Value in For Statement.");
        return false;
    }

    tk = Parser::GetNextToken(in, line);

    if (!(tk == TO || tk == DOWNTO)){
        ParseError(line, "For Statement Syntax Error");
        return false;
    }

    tk = Parser::GetNextToken(in, line);

    if (tk != ICONST){
        ParseError(line, "Missing Termination Value in For Statement.");
        return false;
    }

    tk = Parser::GetNextToken(in, line);

    if (tk != DO){
        ParseError(line, "missing do");
        return false;
    }

    bool stmt = Stmt(in, line);
    if (!stmt){
        ParseError(line, "error in statement");
        return false;
    }

    return true;
}


bool AssignStmt(istream &in, int &line){
    LexItem tk = Parser::GetNextToken(in, line);
   

    if (tk != ASSOP){
        ParseError(line, "Missing Assignment Operator");
        return false;
    }

    bool ex = Expr(in, line);
    if (!ex){
        ParseError(line, "Invalid expression");
        return false;
    }

    return true;
}
bool LogicExpr(istream &in, int &line){
    bool exp = Expr(in, line);

    if (!exp){
        ParseError(line, "Missing expression after relational operator");
        return false;
    }

    LexItem tk = Parser::GetNextToken(in, line);

    if (!(tk == EQUAL || tk == LTHAN || tk == GTHAN)){
        ParseError(line, "symbol not found");
        return false;
    }

    exp = Expr(in, line);

    if (!exp){
        ParseError(line, "expression is invalid");
        return false;
    }

    return true;
}
bool Var(istream &in, int &line){
    LexItem tk = Parser::GetNextToken(in, line);
    if (tk != IDENT){
        ParseError(line, "ident not found");
        return false;
    }
    return true;
}
bool IfStmt(istream &in, int &line){
    LexItem tk = Parser::GetNextToken(in, line);
    if (tk != LPAREN){
        ParseError(line, "left paren missing");
        return false;
    }

    bool ex = LogicExpr(in, line);
    if (!ex){
        ParseError(line, "error in Logic");
        return false;
    }
    tk = Parser::GetNextToken(in, line);

    if (tk != RPAREN){
        ParseError(line, "right paren missing");
        return false;
    }

    tk = Parser::GetNextToken(in, line);
    if (tk != THEN){
        ParseError(line, "If-Stmt Syntax Error");
        return false;
    }

    bool stmt = Stmt(in, line);
    if (!stmt){
        ParseError(line, "error in statement");
        return false;
    }

    tk = Parser::GetNextToken(in, line);
    if (tk != ELSE){
        Parser::PushBackToken(tk);
    }
    else{
        stmt = Stmt(in, line);
        if (!stmt){
            ParseError(line, "error in statement");
            return false;
        }
    }

    return true;
}

bool Expr(istream &in, int &line){
    bool t1 = Term(in, line);
    LexItem tok;
    if (!t1){
        
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognised Input Pattern ");
        return false;
    }
    while (tok == PLUS || tok == MINUS){
        t1 = Term(in, line);
        if (!t1){
            ParseError(line, "Missing operand after operator");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
}
bool Term(istream &in, int &line){
    bool sf = SFactor(in, line);
    if (!sf){
        
        return false;
    }

    LexItem tk = Parser::GetNextToken(in, line);
    while ((tk == DIV) || (tk == MULT)){
        sf = SFactor(in, line);
        if (!sf){
            
            return false;
        }
        tk = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tk);
    

    return true;
}
bool SFactor(istream &in, int &line){
    bool lparen = false;
    LexItem tk = Parser::GetNextToken(in, line);
    
    if (tk == LPAREN){
        tk = Parser::GetNextToken(in, line);
        lparen = true;
    }
    if (!(tk == PLUS || tk == MINUS)){
        Parser::PushBackToken(tk);
    }

    if (tk == EQUAL || tk == MULT || tk == DIV){
        
        return false;
    }

    bool fac = Factor(in, line, 0);
    if (!fac){
        ParseError(line, "the variable or const not available");
        return false;
    }

    if (lparen){
        tk = Parser::GetNextToken(in, line);
        if (tk != RPAREN){
            ParseError(line, "Missing ) after expression");
            return false;
        }
    }

    return true;
}
bool Factor(istream &in, int &line, int sign){
    LexItem tk = Parser::GetNextToken(in, line);
    if (!(tk == IDENT || tk == ICONST || tk == RCONST || tk == SCONST)){
        Parser::PushBackToken(tk);
        bool expr = Expr(in, line);
        if (!expr){
            ParseError(line, "invalid expression");
            return false;
        }
    }
    if (tk == IDENT){
        if (!checkVar(tk.GetLexeme(), line)){
            ParseError(line, "Undeclared Variable");
            return false;
        }

    }

    return true;
}

