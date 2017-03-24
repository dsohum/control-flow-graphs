#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

using namespace std;

#include "common-classes.hh"
#include "error-display.hh"
#include "user-options.hh"
#include "symbol-table.hh"
#include "ast.hh"
#include "procedure.hh"
#include "program.hh"
#include "cfg.hh"

void Program::optimize(){
	procedure->optimize();
}

void Procedure::optimize(){
	Control_Flow_Graph cfg(sequence_ast->get_icode_list());
	while(cfg.eliminate_dead_code());
	cfg.make_icode_list(sequence_ast->get_icode_list());
	sequence_ast->print_icode(cout);
}
