Instructions:
	run make to create the executable(do the relevan changes in make file for different versions of Ubuntu)

Idead:
	Make a CGF in Control_Flow_Graph class consisting of multiple Basic_Block objects connected together based on code structure.
	Kill and Gen sets are computed for each block in the CFG.
	In and Out sets are generated using the Iterative Fixed Point method discussed in class with post-order traversal.
	Liveness analysis is done for each basic block by checking liveness of variable for each assignment statement. Liveness at a point is propagated up from the bottom by starting with out set for that block and modifying it at each step as we go up(upward traversal). We repeat the above computations untill convergence for dead code elimination.
	Out-set of end block is initialized with all global symbol table entries.

	We have created cfg.cc and cfg.hh for the above computations.
	We have modified main.cc to do the dead code elimination and output the dce file.
	We have modified program.hh and procedure.hh for optimize() funtion. program.cc implements these functions.
	We have modified symbol-table.hh to accesss list of global-symbol-table entries. This function is also implemented in program.cc
	We have modified ast.hh to access sa_icode_list of Sequence_Ast.

________________________________________________________________________________________________________________________________

BONUS:
We do strong liveness analysis for dead code elimination handling global variables by putting them in out set of the end-block(as BL of slides)



RELEVANT GRAMMAR RULES USED:
################################

	iteration_statement:
		WHILE '(' bool_expression_term ')' statement
	|
		DO statement WHILE '(' bool_expression_term ')' ';'
	|
		FOR '(' assignment_list ';' bool_expression_term ';' assignment_list ')' statement
	;

	assignment_list:
		variable ASSIGN arith_expression
	|
		assignment_list ',' variable ASSIGN arith_expression
	;

################################

assignment_list and statement can be empty.

examples :
	int a,b;
	for(a=0,b=0;a<1;a=a+1){
		a=1;
	}

	for(;a<2;){
		a=a+1;
	}

	for(;a<3;a=a+1,b=a+1){
		a=a+1;
	}



How we did it :
	We modeled FOR into a  Sequence_Ast(read for_ast).
	The one-time executing assignment_list is pushed into a new Sequence_Ast

	Then another Sequence_Ast(read body_ast) is created which contains the body (statement)
	followed by the post iteration assignment_list.
	This Sequence_Ast(body_ast) along with bool_expression_term is passeed to create Iteration_Statement_Ast.

	This Iteration_Statement_Ast is then pushed into the Sequence_Ast(for_ast)

	Structure :
		Sequence_Ast:
			assignment_list
			Iteration_Statement_Ast:
				bool_expression_term
				Sequence_Ast:
					statement
					assignment_list


	The following substructure is similar to that of while:
		Iteration_Statement_Ast:
					bool_expression_term
					Sequence_Ast:
						statement
						assignment_list

	Therefore what FOR essentially becomes is:
		Sequence_Ast:
			assignment_list
			WHILE



#############################################################
RELEVANT GRAMMAR FOR SWITCH

switch_statement:
	SWITCH '(' arith_expression ')' '{' case_list '}'

case_list:
|
	CASE arith_expression ':' statement_list case_list
|
	DEFAULT ':' statement_list
;


examples:
	int a;
	switch ( a ) {
		case 1: a=1; break;
		case 2: a=2; break;
		default : a=3; 
	}
	switch ( a ) {
		default : a=3; 
	}
	switch ( a ) {
		case 1: a=1; break;
	}


	Modeled as series if-then-else clauses with equality checked with the passed arith_expression.
	We always expect break; after each case statement-list; 




