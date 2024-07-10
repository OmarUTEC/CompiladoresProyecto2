CC=g++

SRC_IMP=test_imp_fun.cpp imp.cpp imp_parser.cpp imp_printer.cpp imp_interpreter.cpp imp_value.cpp imp_typechecker.cpp imp_type.cpp imp_codegen.cpp

SRC_SVM=svm_run.cpp svm_parser.cpp svm.cpp


compiler: $(SRC_IMP)
	$(CC) -o compile  $(SRC_IMP)

svm: $(SRC_SVM)
	$(CC) -o svm  $(SRC_SVM)
