#include "imp_typechecker.hh"

ImpTypeChecker::ImpTypeChecker():inttype(),booltype(),voidtype(),maintype() {
  inttype.set_basic_type("int");
  booltype.set_basic_type("bool");
  voidtype.set_basic_type("void");
  // maintype
  list<string> noparams;
  maintype.set_fun_type(noparams,"void");
}

// Metodos usados para el analisis de altura maxima de pila
void ImpTypeChecker::sp_incr(int n) {
  sp += n;
  if (sp > max_sp) max_sp = sp;
}

void ImpTypeChecker::sp_decr(int n) {
  sp -= n;
  if (sp < 0) {
    cout << "stack less than 0" << endl;
    exit(0);
  }
}

void ImpTypeChecker::typecheck(Program* p) {
  env.clear();
  p->accept(this);
  return;
}

void ImpTypeChecker::visit(Program* p) {
  env.add_level();
  ftable.add_level();  // nuevo
  p->var_decs->accept(this);
  p->fun_decs->accept(this);
  if (!env.check("main")) {
    cout << "Programa no tiene main" << endl;
    exit(0);
  }
  ImpType funtype = env.lookup("main");
  if (!funtype.match(maintype)) {
    cout << "Tipo incorrecto de main: " << funtype << endl;
    exit(0);
  }

  env.remove_level();
  cout << "EXITO" << endl;
  // Codigo usado para ver contenido de ftable
  cout << "Reporte ftable" << endl;
  for(int i = 0; i < fnames.size(); i++) {
    cout << "-- Function: " << fnames[i] << endl;
    FEntry fentry = ftable.lookup(fnames[i]);
    cout << fentry.fname << " : " << fentry.ftype << endl;
    cout << "max stack height: " << fentry.max_stack << endl;
    cout << "mem local variables: " << fentry.mem_locals << endl;
  }

  // no remover nivel de ftable porque sera usado por codegen.
  return;
}

void ImpTypeChecker::visit(Body* b) {
  // guardar direccion actual (dir)
  int temp = dir;
  env.add_level();
  b->var_decs->accept(this);
  b->slist->accept(this);
  env.remove_level();
  // restaurar direccion de entrada
  dir = temp;
  return;
}

void ImpTypeChecker::visit(VarDecList* decs) {
  list<VarDec*>::iterator it;
  for (it = decs->vdlist.begin(); it != decs->vdlist.end(); ++it) {
    (*it)->accept(this);
  }  
  return;
}

// Codigo completo
void ImpTypeChecker::visit(FunDecList* s) {
  list<FunDec*>::iterator it;
  for (it = s->fdlist.begin(); it != s->fdlist.end(); ++it) {
    add_fundec(*it);
  } 
  for (it = s->fdlist.begin(); it != s->fdlist.end(); ++it) {
    // inicializar valores de sp, max_sp, dir, max_dir
    sp = max_sp = 0;
    dir = max_dir = 0;
    (*it)->accept(this);
    FEntry fentry;
    string fname  = (*it)->fname;
    fentry.fname = fname;
    fentry.ftype = env.lookup(fname);
    fnames.push_back(fname);
    fentry.max_stack = max_sp;
    fentry.mem_locals = max_dir;
    ftable.add_var(fname, fentry);
  }
  return;
}

void ImpTypeChecker::add_fundec(FunDec* fd) {
  ImpType funtype;
  if (!funtype.set_fun_type(fd->types,fd->rtype)) {
    cout << "Tipo invalido en declaracion de funcion: " << fd->fname << endl;
    exit(0);
  }
  env.add_var(fd->fname,funtype);
  return;
}

// Los tipos de declaraciones de variables son
// agregados al environment
void ImpTypeChecker::visit(VarDec* vd) {
  ImpType type;  
  type.set_basic_type(vd->type);
  if (type.ttype==ImpType::NOTYPE || type.ttype==ImpType::VOID) {
    cout << "Tipo invalido: " << vd->type << endl;
    exit(0);
  }
  list<string>::iterator it;
  for (it = vd->vars.begin(); it != vd->vars.end(); ++it) {
    env.add_var(*it, type);
    // actualizar dir y max_dir
    dir += 1;
    if (dir > max_dir) max_dir = dir;
  }   
  return;
}

void ImpTypeChecker::visit(FunDec* fd) {
  env.add_level();
  ImpType funtype = env.lookup(fd->fname);
  ImpType rtype, ptype;
  rtype.set_basic_type(funtype.types.back());
  list<string>::iterator it;
  int i=0;
  for (it = fd->vars.begin(); it != fd->vars.end(); ++it, i++) {
    ptype.set_basic_type(funtype.types[i]);
    env.add_var(*it,ptype);
  } 
  env.add_var("return", rtype);
  fd->body->accept(this);
  env.remove_level();
  return;
}



 
void ImpTypeChecker::visit(StatementList* s) {
  list<Stm*>::iterator it;
  for (it = s->slist.begin(); it != s->slist.end(); ++it) {
    (*it)->accept(this);
   }
  return;
}

void ImpTypeChecker::visit(AssignStatement* s) {
  ImpType type = s->rhs->accept(this);
  if (!env.check(s->id)) {
    cout << "Variable " << s->id << " undefined" << endl;
    exit(0);
  }
  // que hacer con sp?
  sp_decr(1);
  ImpType var_type = env.lookup(s->id);  
  if (!type.match(var_type)) {
    cout << "Tipo incorrecto en Assign a " << s->id << endl;
    exit(0);
  }
  return;
}

void ImpTypeChecker::visit(PrintStatement* s) {
  s->e->accept(this);
  // que hacer con sp?
  sp_decr(1);
  return;
}

void ImpTypeChecker::visit(IfStatement* s) {
  if (!s->cond->accept(this).match(booltype)) {
    cout << "Expresion conditional en IF debe de ser bool" << endl;
    exit(0);
  }
  // que hacer con sp?
  sp_decr(1);
  s->tbody->accept(this);
  if (s->fbody != NULL)
    s->fbody->accept(this);
  return;
}

void ImpTypeChecker::visit(WhileStatement* s) {
  if (!s->cond->accept(this).match(booltype)) {
    cout << "Expresion conditional en IF debe de ser bool" << endl;
    exit(0);
  }
  // que hacer con sp?
  sp_decr(1);
  s->body->accept(this);
 return;
}

void ImpTypeChecker::visit(ReturnStatement* s) {
 ImpType rtype = env.lookup("return");
  ImpType etype;
  if (s->e != NULL){
    etype = s->e->accept(this);// que hacer con sp?
    sp_decr(1);
  }
  else
    etype = voidtype;
  if (!rtype.match(etype)) {
    cout << "Return type mismatch: " << rtype << "<->" << etype << endl;
    exit(0);
  }
  return;
}

void ImpTypeChecker::visit(FCallStatement* e) {
  e->e->accept(this);
  return;
}

ImpType ImpTypeChecker::visit(BinaryExp* e) {
  ImpType t1 = e->left->accept(this);
  ImpType t2 = e->right->accept(this);
  if (!t1.match(inttype) || !t2.match(inttype)) {
    cout << "Tipos en BinExp deben de ser int" << endl;
    exit(0);
  }
  ImpType result;
  switch(e->op) {
  case PLUS: 
  case MINUS:
  case MULT:
  case DIV:
  case EXP:
    result = inttype;
    break;
  case LT: 
  case LTEQ:
  case EQ:
    result = booltype;
    break;
  }
  // que hacer con sp?
  sp_decr(1);
  return result;
}

ImpType ImpTypeChecker::visit(NumberExp* e) {
  // que hacer con sp?
  sp_incr(1);
  return inttype;
}

ImpType ImpTypeChecker::visit(TrueFalseExp* e) {
  // que hacer con sp?
  sp_incr(1);
  return booltype;
}

ImpType ImpTypeChecker::visit(IdExp* e) {
  // que hacer con sp?
  sp_incr(1);
  if (env.check(e->id))
    return env.lookup(e->id);
  else {
    cout << "Variable indefinida: " << e->id << endl;
    exit(0);
  }
}

ImpType ImpTypeChecker::visit(ParenthExp* ep) {
  return ep->e->accept(this);
}

ImpType ImpTypeChecker::visit(CondExp* e) {
  if (!e->cond->accept(this).match(booltype)) {
    cout << "Tipo en ifexp debe de ser bool" << endl;
    exit(0);
  }
  // que hacer con sp?
  sp_decr(1);
  int sp_start = sp;
  ImpType ttype =  e->etrue->accept(this);
  // que hacer con sp?
  sp = sp_start;
  if (!ttype.match(e->efalse->accept(this))) {
    cout << "Tipos en ifexp deben de ser iguales" << endl;
    exit(0);
  }
  return ttype;
}

ImpType ImpTypeChecker::visit(FCallExp* e) {
  if (!env.check(e->fname)) {
    cout << "(Function call): " << e->fname <<  " no existe" << endl;
    exit(0);
  }
  ImpType funtype = env.lookup(e->fname);
  if (funtype.ttype != ImpType::FUN) {
    cout << "(Function call): " << e->fname <<  " no es una funcion" << endl;
    exit(0);
  }

  // check args
  int num_fun_args = funtype.types.size()-1;
  int num_fcall_args = e->args.size();
  ImpType rtype;
  rtype.set_basic_type(funtype.types[num_fun_args]);

  // que hacer con sp y el valor de retorno?
  if(!voidtype.match(rtype)) sp_incr(1);
  if (num_fun_args != num_fcall_args) {
    cout << "(Function call) Numero de argumentos no corresponde a declaracion de: " << e->fname << endl;
    exit(0);
  }
  ImpType argtype;
  list<Exp*>::iterator it;
  int i = 0;
  for (it = e->args.begin(); it != e->args.end(); ++it) {
    argtype = (*it)->accept(this);
    if (argtype.ttype != funtype.types[i]) {
      cout << "(Function call) Tipo de argumento no corresponde a tipo de parametro en fcall de: " << e->fname << endl;
      exit(0);
    }
    i++;
  }

  
  return rtype;
}
