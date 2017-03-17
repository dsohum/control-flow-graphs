%scanner scanner.h
%scanner-token-function d_scanner.lex()
%filenames parser
%parsefun-source parser.cc

%union 
{
	pair<Data_Type, list<string>* > * decl;
	Symbol_Table * symbol_table;
	list<Symbol_Table_Entry*> * symbol_entry;
	Procedure * procedure;
	//ADD CODE HERE
	int integer_value;
	double double_value;
	std::string* string_value;
	Sequence_Ast * sequence_ast;
	Ast * ast;
	Arithmetic_Expr_Ast* arith_exp_ast;
	list<string>* names;
};

//ADD TOKENS HERE
%token <integer_value> INTEGER_NUMBER
%token <double_value> DOUBLE_NUMBER
%token <string_value> NAME
%token INTEGER FLOAT 
%token ASSIGN VOID  
%token WHILE
%token DO
%token IF


%nonassoc THEN
%nonassoc ELSE

%left OR
%left AND
%nonassoc EQ NE
%nonassoc LE LT GE GT
%left '+' '-'
%left '*' '/'
%right UMINUS NOT
%nonassoc '('



%type <symbol_table> optional_variable_declaration_list
%type <symbol_table> variable_declaration_list
%type <symbol_entry> variable_declaration
%type <decl> declaration
%type <names> name_list 
//ADD CODE HERE
%type <sequence_ast> statement_list
%type <ast> statement
%type <ast> other_than_selection_statement
%type <ast> iteration_statement
%type <ast> assignment_statement
%type <ast> arith_expression
%type <ast> operand
%type <ast> bool_expression_term
%type <ast> relational_expression
%type <ast> boolean_expression
%type <ast> expression_term
%type <ast> variable
%type <ast> constant

%start program

%%

program:
	declaration_list procedure_definition
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT((current_procedure != NULL), "Current procedure cannot be null");

		program_object.set_procedure(current_procedure, get_line_number());
		program_object.global_list_in_proc_check();
	}
	}
;

declaration_list:
	procedure_declaration
	{
	if (NOT_ONLY_PARSE)
	{
		Symbol_Table * global_table = new Symbol_Table();
		program_object.set_global_table(*global_table);
	}
	}
|
	variable_declaration_list
	procedure_declaration
	{
	if (NOT_ONLY_PARSE)
	{
		Symbol_Table * global_table = $1;

		CHECK_INVARIANT((global_table != NULL), "Global declarations cannot be null");

		program_object.set_global_table(*global_table);
	}
	}
|
	procedure_declaration
	variable_declaration_list
	{
	if (NOT_ONLY_PARSE)
	{
		Symbol_Table * global_table = $2;

		CHECK_INVARIANT((global_table != NULL), "Global declarations cannot be null");

		program_object.set_global_table(*global_table);
	}
	}
;

procedure_declaration:
	VOID NAME '(' ')' ';'
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "Procedure name cannot be null");
		CHECK_INVARIANT((*$2 == "main"), "Procedure name must be main in declaration");
	}
	}
;

procedure_definition:
	NAME '(' ')'
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "Procedure name cannot be null");
		CHECK_INVARIANT((*$1 == "main"), "Procedure name must be main");

		string proc_name = *$1;

		current_procedure = new Procedure(void_data_type, proc_name, get_line_number());

		CHECK_INPUT ((program_object.variable_in_symbol_list_check(proc_name) == false),
			"Procedure name cannot be same as global variable", get_line_number());
	}
	}

	'{' optional_variable_declaration_list
	{
	if (NOT_ONLY_PARSE)
	{

		CHECK_INVARIANT((current_procedure != NULL), "Current procedure cannot be null");

		Symbol_Table * local_table = $6;

		if (local_table == NULL)
			local_table = new Symbol_Table();

		current_procedure->set_local_list(*local_table);
	}
	}

	statement_list '}'
	{
	if (NOT_ONLY_PARSE)
	{
		Sequence_Ast* seq = $8;

		CHECK_INVARIANT((current_procedure != NULL), "Current procedure cannot be null");
		CHECK_INVARIANT((seq != NULL), "statement list cannot be null");
		//seq->check_ast();
		current_procedure->set_sequence_ast(*seq);
	}
	}
;

optional_variable_declaration_list:
	{
	if (NOT_ONLY_PARSE)
	{
		$$ = NULL;
	}
	}
|
	variable_declaration_list
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "Declaration statement list cannot be null here");

		$$ = $1;
	}
	}
;

variable_declaration_list:
	variable_declaration
	{
	if (NOT_ONLY_PARSE)
	{
		list<Symbol_Table_Entry*> * decl_stmt = $1;

		CHECK_INVARIANT((decl_stmt != NULL), "Non-terminal declaration statement cannot be null");

		Symbol_Table * decl_list = new Symbol_Table();
		for(list<Symbol_Table_Entry*>::iterator it=decl_stmt->begin();it!=decl_stmt->end();it++ ){
			string decl_name = (*it)->get_variable_name();
			CHECK_INPUT ((program_object.variable_proc_name_check(decl_name) == false),
				"Variable name cannot be same as the procedure name", get_line_number());

			if(current_procedure != NULL)
			{
				CHECK_INPUT((current_procedure->get_proc_name() != decl_name),
					"Variable name cannot be same as procedure name", get_line_number());
			}
			CHECK_INPUT((decl_list->variable_in_symbol_list_check(decl_name) == false), 
					"Variable is declared twice", get_line_number());

			decl_list->push_symbol(*it);
		}
		$$ = decl_list;
	}
	}
|
	variable_declaration_list variable_declaration
	{
	if (NOT_ONLY_PARSE)
	{
		// if declaration is local then no need to check in global list
		// if declaration is global then this list is global list

		list<Symbol_Table_Entry*> * decl_stmt = $2;
		Symbol_Table * decl_list = $1;

		CHECK_INVARIANT((decl_stmt != NULL), "The declaration statement cannot be null");
		CHECK_INVARIANT((decl_list != NULL), "The declaration statement list cannot be null");

		for(list<Symbol_Table_Entry*>::iterator it=decl_stmt->begin();it!=decl_stmt->end();it++){
			string decl_name = (*it)->get_variable_name();
			CHECK_INPUT((program_object.variable_proc_name_check(decl_name) == false),
				"Procedure name cannot be same as the variable name", get_line_number());
			if(current_procedure != NULL)
			{
				CHECK_INPUT((current_procedure->get_proc_name() != decl_name),
					"Variable name cannot be same as procedure name", get_line_number());
			}

			CHECK_INPUT((decl_list->variable_in_symbol_list_check(decl_name) == false), 
					"Variable is declared twice", get_line_number());

			decl_list->push_symbol(*it);
		}
		$$ = decl_list;
	}
	}
;

variable_declaration:
	declaration ';'
	{
	if (NOT_ONLY_PARSE)
	{
		pair<Data_Type, list<string>*> * decl = $1;

		CHECK_INVARIANT((decl != NULL), "Declaration cannot be null");

		Data_Type type = decl->first;
		list<string> *decl_name_list = decl->second;

		list<Symbol_Table_Entry*>* decl_entry_list=new list<Symbol_Table_Entry*>();
		for(list<string>::iterator it=decl_name_list->begin(); it!=decl_name_list->end();it++){
			Symbol_Table_Entry * decl_entry = new Symbol_Table_Entry(*it, type, get_line_number());
			decl_entry_list->push_front(decl_entry);
		}

		$$ = decl_entry_list;

	}
	}
;

declaration:
	INTEGER name_list
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE
		CHECK_INVARIANT(($2 != NULL), "Name list cannot be null");

		Data_Type type = int_data_type;

		pair<Data_Type, list<string>* > * declar = new pair<Data_Type, list<string>*>(type, $2);

		$$ = declar;
	}
	}
|
	FLOAT name_list
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "Name list cannot be null");

		Data_Type type = double_data_type;

		pair<Data_Type, list<string>*> * declar = new pair<Data_Type, list<string>*>(type, $2);

		$$ = declar;
	}
	}
;

name_list:
	NAME ',' name_list
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "Name cannot be null");
		CHECK_INVARIANT(($3 != NULL), "Name list cannot be null");

		string name = *$1;
		delete $1;

		list<string> * declar = $3;
		declar->push_back(name);
		$$ = declar;
	}
	}
|
	NAME
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "Name cannot be null");

		string name = *$1;
		delete $1;

		list<string> *declar = new list<string>();
		declar->push_back(name);
		$$ = declar;
	}
	}
;


statement_list:
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE ok
		Sequence_Ast* stmt_list=new Sequence_Ast(get_line_number());
		$$ = stmt_list;
	}
	}
|
	statement_list statement
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE ok
		Sequence_Ast* stmt_list = $1;
		Ast* stmt = $2;

		CHECK_INVARIANT((stmt_list!=NULL),"stmt list can't be null");
		CHECK_INVARIANT((stmt!=NULL),"assign stmt can't be null");

		stmt_list->ast_push_back(stmt);
		$$ = stmt_list;
	}
	}
;

///////////////////////////////////////////////////////////////
statement:
	IF '(' bool_expression_term ')' statement ELSE statement
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($3 != NULL), "condition-statement cannot be null");	
		CHECK_INVARIANT(($5 != NULL), "matched-selection-statement cannot be null");	
		CHECK_INVARIANT(($7 != NULL), "matched-selection-statement cannot be null");
		Selection_Statement_Ast* if_then_else = new Selection_Statement_Ast($3,$5,$7,get_line_number());
		//if_then_else->check_ast();
		$$ = (Ast*)if_then_else;
	}
	}
|
	IF '(' bool_expression_term ')' statement 		%prec THEN
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($3 != NULL), "condition-statement cannot be null");	
		CHECK_INVARIANT(($5 != NULL), "selection-statement cannot be null");	
		Sequence_Ast* empty_else_ast = new Sequence_Ast(get_line_number());
		Selection_Statement_Ast* if_then = new Selection_Statement_Ast($3,$5,empty_else_ast,get_line_number());
		//if_then->check_ast();
		$$ = (Ast*)if_then;
	}
	}
|
	other_than_selection_statement
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "statement cannot be null");
		//$1->check_ast();
		$$ = $1;
	}	
	}
;

other_than_selection_statement:
	assignment_statement
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "assignment-statement cannot be null");	
		$$ = (Ast*)$1;
	}
	}
|	
	iteration_statement
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "iteration-statement cannot be null");	
		$$ = $1;
	}
	}
|
	'{' statement_list '}'
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "block-statement cannot be null");	
		$$ = $2;
	}
	}
;

///////////////////////////////////////////////////////////////
iteration_statement:
	WHILE '(' bool_expression_term ')' statement
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($3 != NULL), "bool-expression cannot be null");	
		CHECK_INVARIANT(($5 != NULL), "while-statement cannot be null");
		Iteration_Statement_Ast* while_stmt = new Iteration_Statement_Ast($3,$5,get_line_number(),false);
		$$ = (Ast*)while_stmt;
	}
	}
|
	DO statement WHILE '(' bool_expression_term ')' ';'
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "body-expression cannot be null");	
		CHECK_INVARIANT(($5 != NULL), "condition-expression cannot be null");	
		Iteration_Statement_Ast* do_while_stmt = new Iteration_Statement_Ast($5,$2,get_line_number(),true);
		$$ = (Ast*)do_while_stmt;
	}
	}	
;

///////////////////////////////////////////////////////////////
// Make sure to call check_ast in assignment_statement and arith_expression
// Refer to error_display.hh for displaying semantic errors if any
assignment_statement:
	variable ASSIGN arith_expression ';'
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE
		Ast* arith_exp_ast = $3;
		Ast* var_ast = $1;
		CHECK_INVARIANT(((var_ast != NULL) && (arith_exp_ast != NULL)), "lhs/rhs cannot be null");
		Assignment_Ast *assign_ast = new Assignment_Ast(var_ast,arith_exp_ast,get_line_number());
		//assign_ast->check_ast();
		$$=assign_ast;	
	}
	}
;
///////////////////////////////////////////////////////////////
arith_expression:
		//ADD RELEVANT CODE ALONG WITH GRAMMAR RULES HERE
                // SUPPORT binary +, -, *, / operations, unary -, and allow parenthesization
                // i.e. E -> (E)
                // Connect the rules with the remaining rules given below
	operand '+' operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL)&&($3 != NULL), "operand-term cannot be null");	

		Plus_Ast *plus_ast = new Plus_Ast($1,$3,get_line_number());
		$$ = (Ast*)plus_ast;
		$$->check_ast();
	}
	}
|
	operand '-' operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL)&&($3 != NULL), "operand-term cannot be null");	

		Minus_Ast *minus_ast = new Minus_Ast($1,$3,get_line_number());
		$$ = (Ast*)minus_ast;
		$$->check_ast();
	}
	}
|
	operand '*' operand	
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL)&&($3 != NULL), "operand-term cannot be null");	

		Mult_Ast *mult_ast = new Mult_Ast($1,$3,get_line_number());
		$$ = (Ast*)mult_ast;
		$$->check_ast();
	}
	}
|
	operand '/' operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL)&&($3 != NULL), "operand-term cannot be null");	

		Divide_Ast *divide_ast = new Divide_Ast($1,$3,get_line_number());
		$$ = (Ast*)divide_ast;	
		$$->check_ast();
	}
	}
|
	'-' operand %prec UMINUS
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "operand-term cannot be null");	

		UMinus_Ast *uminus_ast = new UMinus_Ast($2,NULL,get_line_number());
		$$ = (Ast*)uminus_ast;
		// $$->check_ast();
	}
	}
|
	'(' operand ')'
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "operand-term cannot be null");	
		$$ = $2;
	}
	}
|
	expression_term 
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "exp-term cannot be null");	
		$$ = $1;
	}
	}
|
	bool_expression_term '?' operand ':' operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "bool-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "arith-expression-lhs cannot be null");	
		CHECK_INVARIANT(($5 != NULL), "arith-expression-rhs cannot be null");	
		Conditional_Operator_Ast* cond_op_ast = new Conditional_Operator_Ast($1,$3,$5,get_line_number()); 
		$$ = cond_op_ast;
	}
	}	
;

///////////////////////////////////////////////////////////

bool_expression_term:
	boolean_expression
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "bool-expression cannot be null");	
		$$ = $1;
	}
	}
|
	relational_expression
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "relational-expression cannot be null");	
		$$ = $1;
	}
	}
|
	'(' bool_expression_term ')'
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "bool-expression cannot be null");	
		$$ = $2;
	}
	}
;

//////////////////////////////////////////////////////////

boolean_expression:
	bool_expression_term AND bool_expression_term	
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "bool-expression-lhs cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "bool-expression-lhs cannot be null");	
		Boolean_Expr_Ast* bool_exp = new Boolean_Expr_Ast($1,boolean_and,$3,get_line_number());
		// bool_exp->check_ast;
		$$ = bool_exp;
	}
	}
|
	bool_expression_term OR bool_expression_term	
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "bool-expression-lhs cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "bool-expression-lhs cannot be null");	
		Boolean_Expr_Ast* bool_exp = new Boolean_Expr_Ast($1,boolean_or,$3,get_line_number());
		// bool_exp->check_ast;
		$$ = bool_exp;
	}
	}
|
	NOT bool_expression_term
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($2 != NULL), "bool-expression cannot be null");	
		Boolean_Expr_Ast* bool_exp = new Boolean_Expr_Ast(NULL,boolean_not,$2,get_line_number());
		// bool_exp->check_ast;
		$$ = bool_exp;
	}
	}
;

//////////////////////////////////////////////////////////
relational_expression:
	operand LE operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "lhs of rel-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "rhs of rel-expression cannot be null");	
		
		Relational_Expr_Ast* rel_exp = new Relational_Expr_Ast($1,less_equalto,$3,get_line_number());
		$$ = rel_exp;
	}
	}
|
	operand LT operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "lhs of rel-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "rhs of rel-expression cannot be null");	
		
		Relational_Expr_Ast* rel_exp = new Relational_Expr_Ast($1,less_than,$3,get_line_number());
		// rel_exp->check_ast();
		$$ = rel_exp;
	}
	}
|
	operand EQ operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "lhs of rel-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "rhs of rel-expression cannot be null");	
		
		Relational_Expr_Ast* rel_exp = new Relational_Expr_Ast($1,equalto,$3,get_line_number());
		// rel_exp->check_ast();
		$$ = rel_exp;
	}
	}
|
	operand NE operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "lhs of rel-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "rhs of rel-expression cannot be null");	
		
		Relational_Expr_Ast* rel_exp = new Relational_Expr_Ast($1,not_equalto,$3,get_line_number());
		// rel_exp->check_ast();
		$$ = rel_exp;
	}
	}
|
	operand GE operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "lhs of rel-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "rhs of rel-expression cannot be null");	
		
		Relational_Expr_Ast* rel_exp = new Relational_Expr_Ast($1,greater_equalto,$3,get_line_number());
		// rel_exp->check_ast();
		$$ = rel_exp;
	}
	}
|
	operand GT operand
	{
	if (NOT_ONLY_PARSE)
	{
		CHECK_INVARIANT(($1 != NULL), "lhs of rel-expression cannot be null");	
		CHECK_INVARIANT(($3 != NULL), "rhs of rel-expression cannot be null");	
		
		Relational_Expr_Ast* rel_exp = new Relational_Expr_Ast($1,greater_than,$3,get_line_number());
		// rel_exp->check_ast();
		$$ = rel_exp;
	}
	}
;
//////////////////////////////////////////////////////////
operand:
	arith_expression
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE
		CHECK_INVARIANT(($1 != NULL), "arith-exp cannot be null");	
		$$=$1;
	}
	}
;

expression_term:
	variable
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE ok
		CHECK_INVARIANT(($1 != NULL), "Variable (name_node) cannot be null");
		$$ = (Ast*)$1;	
	}
	}
|
	constant
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE ok
		CHECK_INVARIANT(($1 != NULL), "Constant (name_node) cannot be null");
		$$ = (Ast*)$1;	
	}
	}
;

variable:
	NAME
	{
	if (NOT_ONLY_PARSE)
	{
		Symbol_Table_Entry * var_table_entry;

		CHECK_INVARIANT(($1 != NULL), "Variable name cannot be null");

		if (current_procedure->variable_in_symbol_list_check(*$1))
			 var_table_entry = &(current_procedure->get_symbol_table_entry(*$1));

		else if (program_object.variable_in_symbol_list_check(*$1))
			var_table_entry = &(program_object.get_symbol_table_entry(*$1));

		else
			CHECK_INPUT_AND_ABORT(CONTROL_SHOULD_NOT_REACH, "Variable has not been declared", get_line_number());

		$$ = new Name_Ast(*$1, *var_table_entry, get_line_number());
		delete $1;
	}
	}
;

constant:
	INTEGER_NUMBER
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE ok
		int int_val = $1;	
		Ast* number_ast = new Number_Ast<int>(int_val, int_data_type, get_line_number());

		$$ = number_ast;
	}
	}
|
	DOUBLE_NUMBER
	{
	if (NOT_ONLY_PARSE)
	{
		//ADD CODE HERE ok
		double double_val = $1;	
		Ast* number_ast = new Number_Ast<double>(double_val, double_data_type, get_line_number());

		$$ = number_ast;
	}
	}
;
