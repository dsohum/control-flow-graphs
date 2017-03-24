#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <fstream>

using namespace std;

#include "common-classes.hh"
#include "error-display.hh"
#include "user-options.hh"
#include "symbol-table.hh"
#include "ast.hh"
#include "procedure.hh"
#include "program.hh"
#include "cfg.hh"

void Program::optimize(bool print_icode,string file_name){
	procedure->optimize(print_icode,file_name);
}

Symbol_Table& Program::get_global_symbol_table(){
	return global_symbol_table;
}

list<Symbol_Table_Entry*>& Symbol_Table::get_variable_table_list(){
	return variable_table;
}
v
oid Procedure::optimize(bool print_icode,string file_name){
	//construct the cfg for this program
	Control_Flow_Graph cfg(sequence_ast->get_icode_list());
	//elimoinate dead code untill convergence
	while(cfg.eliminate_dead_code());
	//get the filtered icode list
	cfg.make_icode_list(sequence_ast->get_icode_list());
	if(print_icode){
		cout<<"  Procedure: main\n  Intermediate Code Statements";
		sequence_ast->print_icode(cout);
	}
	//write to .dce file
	ofstream ofile;	
	ofile.open(file_name+".dce");
	ofile<<"  Procedure: main\n  Intermediate Code Statements\n";
	sequence_ast->print_icode(ofile);
	ofile.close();
}
