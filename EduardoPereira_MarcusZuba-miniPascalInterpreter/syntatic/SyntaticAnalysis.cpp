#include <cstdlib>
#include <string>
#include <iostream>
#include <cstdio>

#include "SyntaticAnalysis.h"

using namespace std;

SyntaticAnalysis::SyntaticAnalysis(LexicalAnalysis &lex) : m_lex(lex), m_current(m_lex.nextToken())
{
}

SyntaticAnalysis::~SyntaticAnalysis()
{
}

Command *SyntaticAnalysis::start()
{
    Command *cmd = procProgram();
    eat(TKN_END_OF_FILE);
    return cmd;
}

void SyntaticAnalysis::advance() {
    m_current = m_lex.nextToken();
}

void SyntaticAnalysis::eat(enum TokenType type) {
    // printf("Match token: %s -> %s (\"%s\")\n", tt2str(type).c_str(), tt2str(m_current.type).c_str(), m_current.token.c_str());
    if (type == m_current.type) {
        m_current = m_lex.nextToken();
    } else {
        showError();
    }
}

void SyntaticAnalysis::showError()
{
    printf("%02d: ", m_lex.line());

    switch (m_current.type)
    {
    case TKN_INVALID_TOKEN:
        printf("Lexema inválido [%s]\n", m_current.token.c_str());
        break;
    case TKN_UNEXPECTED_EOF:
    case TKN_END_OF_FILE:
        printf("Fim de arquivo inesperado\n");
        break;
    default:
        printf("Lexema não esperado [%s]\n", m_current.token.c_str());
        break;
    }

    exit(1);
}

// <program>  ::= program <id> ';'
//                [ const <const> { <const> } ]
//                [ var <var> { <var> } ]
//                <block> '.'
Command* SyntaticAnalysis::procProgram()
{
    eat(TKN_PROGRAM);
    procId();
    eat(TKN_SEMICOLON);
    if (m_current.type == TKN_CONST)
    {
        eat(TKN_CONST);
        procConst();
        while (m_current.type == TKN_ID)
        {
            procConst();
        }
    }
    if (m_current.type == TKN_VAR)
    {
        eat(TKN_VAR);
        procVar();
        while (m_current.type == TKN_ID)
        {
            procVar();
        }
    }
    Command* cmd = procBlock();
    eat(TKN_DOT);
    return cmd;
}

// <const>    ::= <id> = <value> ';'
void SyntaticAnalysis::procConst()
{
    Variable* var = procId();
    eat(TKN_EQUAL);
    Type* value = procValue();
    Memory::registryConstant(var->getName(), value);
    eat(TKN_SEMICOLON);
}

// <var>      ::= <id> { ',' <id> } [ = <value> ] ';'
void SyntaticAnalysis::procVar()
{
    Variable* var = procId();
    Memory::registryVariable(var->getName(), new IntegerValue(0));
    while (m_current.type == TKN_COMMA)
    {
        eat(TKN_COMMA);
        var = procId();
        Memory::registryVariable(var->getName(), new IntegerValue(0));
    }
    if (m_current.type == TKN_EQUAL)
    {
        eat(TKN_EQUAL);
        Type* value = procValue();
        var->setValue(value);
    }
    eat(TKN_SEMICOLON);
}

// <block>    ::= begin [ <cmd> { ';' <cmd> } ] end
BlocksCommand* SyntaticAnalysis::procBlock()
{
    int line = m_lex.line();
    BlocksCommand* bc = new BlocksCommand(line);
    eat(TKN_BEGIN);
    if (m_current.type == TKN_ID || m_current.type == TKN_IF || m_current.type == TKN_CASE || m_current.type == TKN_WHILE ||
        m_current.type == TKN_FOR || m_current.type == TKN_REPEAT || m_current.type == TKN_WRITE || m_current.type == TKN_WRITELN || m_current.type == TKN_READLN)
    {
        bc->addCommand(procCmd());
        while (m_current.type == TKN_SEMICOLON)
        {
            eat(TKN_SEMICOLON);
            bc->addCommand(procCmd());
        }
    }
    eat(TKN_END);
    return bc;
}

// <body>     ::= <block> | <cmd>
Command* SyntaticAnalysis::procBody()
{
    if (m_current.type == TKN_BEGIN)
    {
        return procBlock();
    }
    else if (m_current.type == TKN_ID || m_current.type == TKN_IF || m_current.type == TKN_CASE || m_current.type == TKN_WHILE ||
             m_current.type == TKN_FOR || m_current.type == TKN_REPEAT || m_current.type == TKN_WRITE || m_current.type == TKN_WRITELN || m_current.type == TKN_READLN)
    {
        return procCmd();
    }
    else
    {
        showError();
    }
    return NULL;
}

// <cmd>      ::= <assign> | <if> | <case> | <while> | <for> | <repeat> | <write> | <read>
Command* SyntaticAnalysis::procCmd()
{
    if (m_current.type == TKN_ID)
    {
        return procAssign();
    }
    else if (m_current.type == TKN_IF)
    {
        return procIf();
    }
    else if (m_current.type == TKN_CASE)
    {
        return procCase();
    }
    else if (m_current.type == TKN_WHILE)
    {
        return procWhile();
    }
    else if (m_current.type == TKN_FOR)
    {
        return procFor();
    }
    else if (m_current.type == TKN_REPEAT)
    {
        return procRepeat();
    }
    else if (m_current.type == TKN_WRITE || m_current.type == TKN_WRITELN)
    {
        return procWrite();
    }
    else if (m_current.type == TKN_READLN)
    {
        return procRead();
    }
    else
    {
        showError();
    }
    return NULL;
}

// <assign>   ::= <id> := <expr>
AssignCommand* SyntaticAnalysis::procAssign()
{
    int line = m_lex.line();
    Variable* var = procId();
    eat(TKN_ASSIGN);
    Expr* e = procExpr();
    return new AssignCommand(line, var, e);
}

// <if>       ::= if <boolexpr> then <body> [else <body>]
IfCommand* SyntaticAnalysis::procIf()
{
    int line = m_lex.line();
    eat(TKN_IF);
    BoolExpr* cond = procBoolExpr();
    eat(TKN_THEN);
    Command* thenCommands = procBody();
    IfCommand* ifc = new IfCommand(line, cond, thenCommands);
    if (m_current.type == TKN_ELSE)
    {
        eat(TKN_ELSE);
        Command* elseCommands = procBody();
        ifc->setElseCommands(elseCommands);
    }
    return ifc;
}

// <case>     ::= case <expr> of { <value> : <body> ';' } [ else <body> ';' ] end
CaseCommand* SyntaticAnalysis::procCase()
{
    int line = m_lex.line();
    eat(TKN_CASE);
    Expr* expr = procExpr();
    CaseCommand* cc = new CaseCommand(line, expr);
    eat(TKN_OF);
    while(m_current.type == TKN_INTEGER || m_current.type == TKN_REAL ||m_current.type == TKN_STRING){
        Type* value = procValue();
        eat(TKN_COLON);
        Command* cmd = procBody();
        cc->addOption(value, cmd);
        eat(TKN_SEMICOLON);
    }
    if(m_current.type == TKN_ELSE){
        eat(TKN_ELSE);
        Command* cmd = procBody();
        cc->setOtherwise(cmd);
        eat(TKN_SEMICOLON);
    }
    eat(TKN_END);
    return cc;
}

// <while>    ::= while <boolexpr> do <body>
WhileCommand* SyntaticAnalysis::procWhile()
{
    int line = m_lex.line();
    eat(TKN_WHILE);
    BoolExpr* cond = procBoolExpr();
    eat(TKN_DO);
    Command* cmds = procBody();
    return new WhileCommand(line, cond, cmds);
}

// <repeat>   ::= repeat [ <cmd> { ';' <cmd> } ] until <boolexpr>
RepeatCommand* SyntaticAnalysis::procRepeat()
{
    int line = m_lex.line();
    std::list<Command*> cmds = {};
    eat(TKN_REPEAT);
    if (m_current.type == TKN_ID || m_current.type == TKN_IF || m_current.type == TKN_CASE || m_current.type == TKN_WHILE ||
        m_current.type == TKN_FOR || m_current.type == TKN_REPEAT || m_current.type == TKN_WRITE || m_current.type == TKN_WRITELN || m_current.type == TKN_READLN)
    {
        cmds.push_back(procCmd());
        while (m_current.type == TKN_SEMICOLON)
        {
            eat(TKN_SEMICOLON);
            cmds.push_back(procCmd());
        }
    }
    eat(TKN_UNTIL);
    BoolExpr* cond = procBoolExpr();
    return new RepeatCommand(line, cmds, cond);
}

// <for>      ::= for <id> := <expr> to <expr> do <body>
ForCommand* SyntaticAnalysis::procFor()
{
    int line = m_lex.line();
    eat(TKN_FOR);
    Variable* var = procId();
    Memory::registryVariable(var->getName(), new IntegerValue(0));
    eat(TKN_ASSIGN);
    Expr* src = procExpr();
    eat(TKN_TO);
    Expr* dst = procExpr();
    eat(TKN_DO);
    Command* cmd = procBody();
    return new ForCommand(line, var, src, dst, cmd);
}

// <write>    ::= (write | writeln) '(' [ <expr> { ',' <expr> } ] ')'
WriteCommand* SyntaticAnalysis::procWrite()
{
    int line = m_lex.line();
    WriteCommand* wc = new WriteCommand(line, false);
    if (m_current.type == TKN_WRITE)
    {
        eat(TKN_WRITE);
    }
    else if (m_current.type == TKN_WRITELN)
    {
        eat(TKN_WRITELN);
        wc = new WriteCommand(line, true);
    }
    eat(TKN_OPEN_PAR);
    if (m_current.type == TKN_INTEGER || m_current.type == TKN_REAL || m_current.type == TKN_STRING || m_current.type == TKN_ID || m_current.type == TKN_OPEN_PAR)
    {
        wc->addExpr(procExpr());
        while (m_current.type == TKN_COMMA)
        {
            eat(TKN_COMMA);
            wc->addExpr(procExpr());
        }
    }
    eat(TKN_CLOSE_PAR);
    return wc;
}

// <read>     ::= readln '(' <id> { ',' <id> } ')'
ReadCommand* SyntaticAnalysis::procRead()
{
    int line = m_lex.line();
    ReadCommand* rc = new ReadCommand(line);
    eat(TKN_READLN);
    eat(TKN_OPEN_PAR);
    Variable* var = procId();
    rc->addVariable(var);
    while (m_current.type == TKN_COMMA)
    {
        eat(TKN_COMMA);
        var = procId();
        rc->addVariable(var);
    }
    eat(TKN_CLOSE_PAR);
    return rc;
}

// <boolexpr> ::= [ not ] <cmpexpr> [ (and | or) <boolexpr> ]
BoolExpr* SyntaticAnalysis::procBoolExpr()
{
    int line = m_lex.line();
    bool notBoolExpr = false;
    if (m_current.type == TKN_NOT)
    {
        notBoolExpr = true;
        eat(TKN_NOT);
    }
    BoolExpr* left = procCmpExpr();
    if (m_current.type == TKN_AND || m_current.type == TKN_OR)
    {
        enum CompositeBoolExpr::BoolOp op;
        if (m_current.type == TKN_AND)
        {
            op = CompositeBoolExpr::And;
            eat(TKN_AND);
        }
        else if (m_current.type == TKN_OR)
        {
            op = CompositeBoolExpr::Or;
            eat(TKN_OR);
        }
        BoolExpr* right = procBoolExpr();
        left = new CompositeBoolExpr(line, left, op, right);
    }
    if(notBoolExpr){
        return new NotBoolExpr(line, left);
    }
    else{
        return left;
    }
}

// <cmpexpr>  ::= <expr> ('=' | '<>' | '<' | '>' | '<=' | '>=') <expr>
SingleBoolExpr* SyntaticAnalysis::procCmpExpr()
{
    int line = m_lex.line();
    Expr* left = procExpr();
    enum SingleBoolExpr::Op op;
    if (m_current.type == TKN_EQUAL)
    {
        op = SingleBoolExpr::EQUAL;
        eat(TKN_EQUAL);
    }
    else if (m_current.type == TKN_NOT_EQUAL)
    {
        op = SingleBoolExpr::NOT_EQUAL;
        eat(TKN_NOT_EQUAL);
    }
    else if (m_current.type == TKN_LOWER)
    {
        op = SingleBoolExpr::LOWER_THAN;
        eat(TKN_LOWER);
    }
    else if (m_current.type == TKN_GREATER)
    {
        op = SingleBoolExpr::GREATER_THAN;
        eat(TKN_GREATER);
    }
    else if (m_current.type == TKN_LOWER_EQ)
    {
        op = SingleBoolExpr::LOWER_EQUAL;
        eat(TKN_LOWER_EQ);
    }
    else if (m_current.type == TKN_GREATER_EQ)
    {
        op = SingleBoolExpr::GREATER_EQUAL;
        eat(TKN_GREATER_EQ);
    }
    else
    {
        showError();
    }
    Expr* right = procExpr();
    return new SingleBoolExpr(line, left, op, right);
}

// <expr>     ::= <term> { ('+' | '-') <term> }
Expr* SyntaticAnalysis::procExpr()
{
    int line = m_lex.line();
    Expr* left = procTerm();
    enum BinaryExpr::BinaryOp op;
    while (m_current.type == TKN_ADD || m_current.type == TKN_SUB)
    {
        if (m_current.type == TKN_ADD)
        {
            op = BinaryExpr::AddOp;
            eat(TKN_ADD);
        }
        else if (m_current.type == TKN_SUB)
        {
            op = BinaryExpr::SubOp;
            eat(TKN_SUB);
        }
        Expr* right = procTerm();
        left = new BinaryExpr(line, left, op, right);
    }
    return left;
}

// <term>     ::= <factor> { ('*' | '/' | '%') <factor> }
Expr* SyntaticAnalysis::procTerm()
{
    int line = m_lex.line();
    enum BinaryExpr::BinaryOp op;
    Expr* left = procFactor();
    while (m_current.type == TKN_MUL || m_current.type == TKN_DIV || m_current.type == TKN_MOD)
    {
        if (m_current.type == TKN_MUL)
        {
            op = BinaryExpr::MulOp;
            eat(TKN_MUL);
        }
        else if (m_current.type == TKN_DIV)
        {
            op = BinaryExpr::DivOp;
            eat(TKN_DIV);
        }
        else if (m_current.type == TKN_MOD)
        {
            op = BinaryExpr::ModOp;
            eat(TKN_MOD);
        }
        Expr* right = procFactor();
        left = new BinaryExpr(line, left, op, right);
    }
    return left;
}

// <factor>   ::= <value> | <id> | '(' <expr> ')'
Expr* SyntaticAnalysis::procFactor()
{
    if (m_current.type == TKN_INTEGER || m_current.type == TKN_REAL || m_current.type == TKN_STRING)
    {
        int line = m_lex.line();
        Type* value = procValue();
        ConstExpr* ce = new ConstExpr(line, value);
        return ce;
    }
    else if (m_current.type == TKN_ID)
    {
        return procId();
    }
    else if (m_current.type == TKN_OPEN_PAR)
    {
        eat(TKN_OPEN_PAR);
        return procExpr();
        eat(TKN_CLOSE_PAR);
    }
    else
    {
        showError();
    }
    return NULL;
}

// <value>    ::= <integer> | <real> | <string>
Type* SyntaticAnalysis::procValue()
{

    if (m_current.type == TKN_INTEGER)
    {
        return procInteger();
    }
    else if (m_current.type == TKN_REAL)
    {
        return procReal();
    }
    else if (m_current.type == TKN_STRING)
    {
        return procString();
    }
    else
    {
        showError();
    }
    return NULL;
}

Variable* SyntaticAnalysis::procId()
{
    std::string name = m_current.token;
    eat(TKN_ID);
    int line = m_lex.line();
    Variable* var = new Variable(line, name); 
    return var; 
}

IntegerValue* SyntaticAnalysis::procInteger()
{
    std::string tmp = m_current.token;
    eat(TKN_INTEGER);
    int value = atoi(tmp.c_str());
    IntegerValue* iv = new IntegerValue(value);
    return iv;
}

RealValue* SyntaticAnalysis::procReal()
{
    std::string tmp = m_current.token;
    eat(TKN_REAL);
    float value = std::stof(tmp);
    RealValue* rv = new RealValue(value);
    return rv;
}

StringValue* SyntaticAnalysis::procString()
{
    std::string tmp = m_current.token;
    //remover aspas simples da string
    tmp = tmp.substr(1,tmp.length()-2);
    eat(TKN_STRING);
    StringValue* sv = new StringValue(tmp);
    return sv; 
}
