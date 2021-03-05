#ifndef BINARY_EXPR_H
#define BINARY_EXPR_H

#include "Expr.h"
#include "../value/IntegerValue.h"
#include "../value/RealValue.h"
#include "../value/StringValue.h"

class BinaryExpr:public Expr{

public:

  enum BinaryOp{
    AddOp,
    SubOp,
    MulOp,
    DivOp,
    ModOp
  };

  BinaryExpr(int line, Expr* left, enum BinaryOp op, Expr* right);

  virtual ~BinaryExpr();

  virtual Type* expr();

private:

  Expr* left;
  enum BinaryOp op;
  Expr* right;
  
};




#endif