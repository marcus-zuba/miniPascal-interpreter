#ifndef SYNTATIC_ANALYSIS_H
#define SYNTATIC_ANALYSIS_H

#include "../interpreter/util/Memory.h"
#include "../lexical/LexicalAnalysis.h"
#include "../interpreter/command/Command.h"
#include "../interpreter/command/AssignCommand.h"
#include "../interpreter/command/BlocksCommand.h"
#include "../interpreter/command/WriteCommand.h"
#include "../interpreter/command/ReadCommand.h"
#include "../interpreter/command/IfCommand.h"
#include "../interpreter/command/RepeatCommand.h"
#include "../interpreter/command/WhileCommand.h"
#include "../interpreter/command/ForCommand.h"
#include "../interpreter/command/CaseCommand.h"
#include "../interpreter/value/Type.h"
#include "../interpreter/value/Value.h"
#include "../interpreter/value/IntegerValue.h"
#include "../interpreter/value/RealValue.h"
#include "../interpreter/value/StringValue.h"
#include "../interpreter/expr/Expr.h"
#include "../interpreter/expr/BoolExpr.h"
#include "../interpreter/expr/NotBoolExpr.h"
#include "../interpreter/expr/BinaryExpr.h"
#include "../interpreter/expr/CompositeBoolExpr.h"
#include "../interpreter/expr/SingleBoolExpr.h"
#include "../interpreter/expr/ConstExpr.h"
#include "../interpreter/expr/Variable.h"
#include <map>

class Command;

class SyntaticAnalysis {
public:
    explicit SyntaticAnalysis(LexicalAnalysis& lex);
    virtual ~SyntaticAnalysis();

    Command* start();

private:
    LexicalAnalysis& m_lex;
    Lexeme m_current;

    void advance();
    void eat(enum TokenType type);
    void matchToken(enum TokenType type);
    void showError();

    Command* procProgram();
    void procConst();
    void procVar();
    BlocksCommand* procBlock();
    Command* procBody();
    Command* procCmd();
    AssignCommand* procAssign();
    IfCommand* procIf();
    CaseCommand* procCase();
    WhileCommand* procWhile();
    RepeatCommand* procRepeat();
    ForCommand* procFor();
    WriteCommand* procWrite();
    ReadCommand* procRead();
    BoolExpr* procBoolExpr();
    SingleBoolExpr* procCmpExpr();
    Expr* procExpr();
    Expr* procTerm();
    Expr* procFactor();
    Type* procValue();
    Variable* procId();
    IntegerValue* procInteger();
    RealValue* procReal();
    StringValue* procString();

};

#endif
