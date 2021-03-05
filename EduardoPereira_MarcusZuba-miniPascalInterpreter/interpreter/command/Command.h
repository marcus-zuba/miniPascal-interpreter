#ifndef COMMAND_H
#define COMMAND_H

class Command {
public:
    virtual ~Command() {}

    int getLine() const { return line; }
    virtual void Execute() = 0;

protected:
    explicit Command(int line) : line(line) {}

private:
    int line;

};

#endif
