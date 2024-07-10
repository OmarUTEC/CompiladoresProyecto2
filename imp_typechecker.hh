#ifndef IMP_TYPECHECKER
#define IMP_TYPECHECKER

#include <unordered_map>

#include "imp.hh"
#include "type_visitor.hh"
#include "environment.hh"

using namespace std;

class FEntry {
public:
  string fname;
  ImpType ftype;
  int mem_locals, max_stack;
};


class ImpTypeChecker : public TypeVisitor {
public:
  ImpTypeChecker();
  // compartido con codegen
  Environment<FEntry> ftable;  
private:
  Environment<ImpType> env;
  ImpType booltype;
  ImpType inttype;
  ImpType voidtype;
  ImpType maintype;
  void add_fundec(FunDec* fd);

  // variables y metodos sugeridos
  vector<string> fnames;
  int sp, max_sp;
  int dir, max_dir;
  void sp_decr(int n);
  void sp_incr(int n);

public:
  void typecheck(Program*);
  void visit(Program*);
  void visit(Body*);
  void visit(VarDecList*);
  void visit(VarDec*);
  void visit(FunDecList*);
  void visit(FunDec*);
  void visit(StatementList*);
  void visit(AssignStatement*);
  void visit(PrintStatement*);
  void visit(IfStatement*);
  void visit(WhileStatement*);
  void visit(ReturnStatement* e);
  
  ImpType visit(BinaryExp* e);
  ImpType visit(NumberExp* e);
  ImpType visit(TrueFalseExp* e);
  ImpType visit(IdExp* e);
  ImpType visit(ParenthExp* e);
  ImpType visit(CondExp* e);
  ImpType visit(FCallExp* e);

};


#endif

