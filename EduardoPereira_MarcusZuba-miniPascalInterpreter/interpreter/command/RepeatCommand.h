#ifndef REPEAT_COMMAND_H
#define REPEAT_COMMAND_H

#include <list>
#include "Command.h"
#include "../expr/BoolExpr.h"

class RepeatCommand:public Command{

public:
  RepeatCommand(int line, std::list<Command*> cmds, BoolExpr* cond);

  virtual ~RepeatCommand();

  virtual void Execute();

private:
  BoolExpr* cond;
  std::list<Command*> cmds;

};

#endif