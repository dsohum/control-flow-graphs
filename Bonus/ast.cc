#include<iostream>
#include<fstream>
#include<typeinfo>

using namespace std;

#include"common-classes.hh"
#include"error-display.hh"
#include"user-options.hh"
#include"local-environment.hh"
#include"symbol-table.hh"
#include"ast.hh"
#include"procedure.hh"
#include"program.hh"

Ast::Ast()
{
}
int Ast::labelCounter=0;

Ast::~Ast()
{}

bool Ast::check_ast()
{
	print(std::cout);
	stringstream msg;
	msg << "No check_ast() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

Data_Type Ast::get_data_type()
{
	stringstream msg;
	msg << "No get_data_type() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

Symbol_Table_Entry & Ast::get_symbol_entry()
{
	stringstream msg;
	msg << "No get_symbol_entry() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

bool Ast::is_value_zero()
{
	stringstream msg;
	msg << "No is_value_zero() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

void Ast::set_data_type(Data_Type dt)
{
	stringstream msg;
	msg << "No set_data_type() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

////////////////////////////////////////////////////////////////

Assignment_Ast::Assignment_Ast(Ast * temp_lhs, Ast * temp_rhs, int line)
{
	//ADD CODE HERE ok
	lhs=temp_lhs;
	rhs=temp_rhs;
	lineno = line;
	if(check_ast()){
		this->node_data_type = lhs->get_data_type();
	}
	else{
		this->node_data_type = void_data_type;
	}
}

Assignment_Ast::~Assignment_Ast()
{
	//ADD CODE HERE ok
	delete lhs;
	delete rhs;
}


bool Assignment_Ast::check_ast()
{
	CHECK_INVARIANT((rhs != NULL), "Rhs of Assignment_Ast cannot be null");
	CHECK_INVARIANT((lhs != NULL), "Lhs of Assignment_Ast cannot be null");

	// use typeid(), get_data_type()
	//ADD CODE HERE
	// if(typeid(*rhs)!=typeid(Name_Ast)&&typeid(*rhs)!=typeid(Number_Ast<double>)&&typeid(*rhs)!=typeid(Number_Ast<int>))
	// 	rhs->check_ast();		
	if(lhs->get_data_type()==rhs->get_data_type()){
		return true;
	}
	/* else if(typeid(*rhs)==typeid(Number_Ast<int>)||typeid(*rhs)==typeid(Number_Ast<double>)){
		if(rhs->is_value_zero()){
			node_data_type=lhs->get_data_type();
			return true;
		}
	}*/
	//node_data_type=(Data_Type)-1;
	CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, 
		"Assignment statement data type not compatible", lineno);
	return false;
}

void Assignment_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE ok
	file_buffer<<"\n         Asgn:\n            LHS (";
	lhs->print(file_buffer);
	file_buffer<<")\n            RHS (";
	rhs->print(file_buffer);	
	file_buffer<<")";
}

/////////////////////////////////////////////////////////////////

Name_Ast::Name_Ast(string & name, Symbol_Table_Entry & var_entry, int line)
{

	variable_symbol_entry= &var_entry;
	CHECK_INVARIANT((variable_symbol_entry->get_variable_name() == name),
		"Variable's symbol entry is not matching its name");
	//ADD CODE HERE
	lineno = line;
	node_data_type = variable_symbol_entry->get_data_type();
}

Name_Ast::~Name_Ast()
{	
	delete variable_symbol_entry;
}
// delete ?

Data_Type Name_Ast::get_data_type()
{
	// refer to functions for Symbol_Table_Entry
	//ADD CODE HERE ok
	return variable_symbol_entry->get_data_type();
}

Symbol_Table_Entry & Name_Ast::get_symbol_entry()
{
	//ADD CODE HERE ok
	return 	*variable_symbol_entry;
}

void Name_Ast::set_data_type(Data_Type dt)
{
	//ADD CODE HERE ok
	variable_symbol_entry->set_data_type(dt);
}

void Name_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"Name : "<<(variable_symbol_entry->get_variable_name());
}

///////////////////////////////////////////////////////////////////////////////

template <class DATA_TYPE>
Number_Ast<DATA_TYPE>::Number_Ast(DATA_TYPE number, Data_Type constant_data_type, int line)
{
	// use Ast_arity from ast.hh
	//ADD CODE HERE
	constant=number;
	ast_num_child=zero_arity;
	node_data_type=constant_data_type;
	lineno = line;
}

template <class DATA_TYPE>
Number_Ast<DATA_TYPE>::~Number_Ast()
{}

template <class DATA_TYPE>
Data_Type Number_Ast<DATA_TYPE>::get_data_type()
{
	//ADD CODE HERE ok
	return node_data_type;
}

template <class DATA_TYPE>
void Number_Ast<DATA_TYPE>::set_data_type(Data_Type dt)
{
	//ADD CODE HERE ok
	node_data_type = dt;
}

template <class DATA_TYPE>
bool Number_Ast<DATA_TYPE>::is_value_zero()
{
	//ADD CODE HERE ok
	return (constant==0);
}

template <class DATA_TYPE>
void Number_Ast<DATA_TYPE>::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"Num : "<<constant;
}

///////////////////////////////////////////////////////////////////////////////
Relational_Expr_Ast::Relational_Expr_Ast(Ast * lhs, Relational_Op rop, Ast * rhs, int line){
	lhs_condition=lhs;
	rel_op=rop;
	rhs_condition=rhs;
	lineno=line;
	if(check_ast())
		node_data_type=int_data_type;
	else
		node_data_type=int_data_type;  //---------------- maybe node_data_type=int_data_type
	ast_num_child=binary_arity;
}

Data_Type Relational_Expr_Ast::get_data_type(){
	return node_data_type;
}
void Relational_Expr_Ast::set_data_type(Data_Type dt){
	node_data_type = dt;
}

bool Relational_Expr_Ast::check_ast(){
	//ADD CODE HERE
	// if(typeid(*lhs_condition)!=typeid(Name_Ast)&&typeid(*lhs_condition)!=typeid(Number_Ast<double>)&&typeid(*lhs_condition)!=typeid(Number_Ast<int>))
	// 	lhs_condition->check_ast();
	// if(typeid(*rhs_condition)!=typeid(Name_Ast)&&typeid(*rhs_condition)!=typeid(Number_Ast<double>)&&typeid(*rhs_condition)!=typeid(Number_Ast<int>))
	// 	rhs_condition->check_ast();
	CHECK_INVARIANT((rhs_condition != NULL), "Rhs of Relational_Expr_Ast cannot be null");
	CHECK_INVARIANT((lhs_condition != NULL), "Lhs of Relational_Expr_Ast cannot be null");

	// use typeid(), get_data_type()
	//ADD CODE HERE
	if(lhs_condition->get_data_type() == rhs_condition->get_data_type())
	{
		return true;
	}
	// if(lhs_condition->get_data_type()==rhs_condition->get_data_type()){
	// 	return true;
	// }
	// print(std::cout);
	CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, "Relational statement data type not compatible", lineno);
	return false;
}

void Relational_Expr_Ast::print(ostream & file_buffer){
	file_buffer<<"\n            Condition: ";
	switch(rel_op){
		case less_equalto:		file_buffer<<"LE"; break;
		case less_than:			file_buffer<<"LT"; break;
		case greater_than:		file_buffer<<"GT"; break;
		case greater_equalto:	file_buffer<<"GE"; break;
		case equalto:			file_buffer<<"EQ"; break;
		case not_equalto:		file_buffer<<"NE"; break;
	}
	file_buffer<<"\n               LHS (";
	lhs_condition->print(file_buffer);
	if(rhs_condition!=NULL){
		file_buffer<<")\n               RHS (";
		rhs_condition->print(file_buffer);
	}
	file_buffer<<")";
}

Relational_Expr_Ast::~Relational_Expr_Ast(){
	delete lhs_condition;
	delete rhs_condition;
}

///////////////////////////////////////////////////////////////////////

Boolean_Expr_Ast::Boolean_Expr_Ast(Ast * lhs, Boolean_Op bop, Ast * rhs, int line){
	lhs_op=lhs;
	bool_op=bop;
	rhs_op=rhs;
	lineno=line;
	// node_data_type=lhs->get_data_type();
	ast_num_child=(bop==boolean_not)?unary_arity:binary_arity;

	if(check_ast()){
		this->node_data_type = int_data_type;
	}
	else{
		this->node_data_type = void_data_type;
	}
}

Data_Type Boolean_Expr_Ast::get_data_type(){
	return node_data_type;
}
void Boolean_Expr_Ast::set_data_type(Data_Type dt){
	node_data_type=dt;
}

bool Boolean_Expr_Ast::check_ast(){
	//ADD CODE HERE ok
	CHECK_INVARIANT((rhs_op != NULL), "Rhs of Boolean_Expr_Ast cannot be null");
	CHECK_INVARIANT((lhs_op != NULL || ast_num_child == unary_arity), "Lhs of Boolean_Expr_Ast cannot be null");

	if(ast_num_child == unary_arity ){
		if(rhs_op->get_data_type() == int_data_type)
			return true;
	}
	else if (rhs_op->get_data_type() == int_data_type && lhs_op->get_data_type()==rhs_op->get_data_type())
		return true;
	// lhs_op->check_ast();
	// if(rhs_op==NULL)
	// 	return true;
	// rhs_op->check_ast();	
	// if(lhs_op->get_data_type()==rhs_op->get_data_type()){
	// 	node_data_type=lhs_op->get_data_type();
	// 	return true;
	// }
	CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, "Boolean statement data type not compatible", lineno);
	return false;
	// return true;
}

void Boolean_Expr_Ast::print(ostream & file_buffer){
	file_buffer<<"\n            Condition: ";
	switch(bool_op){
		case boolean_not:	file_buffer<<"NOT"; break;
		case boolean_and:	file_buffer<<"AND"; break;
		case boolean_or:	file_buffer<<"OR"; break;
	}
	if(lhs_op!=NULL){
		file_buffer<<"\n               LHS (";
		lhs_op->print(file_buffer);
		file_buffer<<")";
	}
	file_buffer<<"\n               RHS (";
	rhs_op->print(file_buffer);
	file_buffer<<")";
}

Boolean_Expr_Ast::~Boolean_Expr_Ast(){
	delete lhs_op;
	delete rhs_op;
}

////////////////////////////////////////////////////////////////

Selection_Statement_Ast::Selection_Statement_Ast(Ast * cond,Ast* then_part, Ast* else_part, int line){
	this->cond=cond;
	this->then_part=then_part;
	this->else_part=else_part;
	lineno=line;
	ast_num_child=ternary_arity;
	if(check_ast()){
		this->node_data_type = int_data_type;
	}
	else{
		this->node_data_type = void_data_type;
	}
}
Selection_Statement_Ast::~Selection_Statement_Ast(){
	delete cond;
	delete then_part;
	delete else_part;
}

Data_Type Selection_Statement_Ast::get_data_type(){
	return node_data_type;
}
void Selection_Statement_Ast::set_data_type(Data_Type dt){
	node_data_type=dt;
}

bool Selection_Statement_Ast::check_ast(){
	//ADD CODE HERE ok
	// cond->check_ast();
	// then_part->check_ast();
	// if(else_part!=NULL)
	// 	else_part->check_ast();
	// return true;
	CHECK_INVARIANT((cond != NULL), "Condition of Selection_Statement_Ast cannot be null");
	CHECK_INVARIANT((then_part != NULL), "If part of Selection_Statement_Ast cannot be null");
}

void Selection_Statement_Ast::print(ostream & file_buffer){
	file_buffer<<"\n         IF : \n         CONDITION (";
	cond->print(file_buffer);
	file_buffer<<")\n         THEN (";
	then_part->print(file_buffer);
	//if(else_part!=NULL){
	file_buffer<<")\n         ELSE (";
	else_part->print(file_buffer);
	//}
	file_buffer<<")";
}

////////////////////////////////////////////////////////////////

Iteration_Statement_Ast::Iteration_Statement_Ast(Ast * cond, Ast* body, int line,bool do_form){
	this->cond=cond;
	this->body=body;
	is_do_form=do_form;
	node_data_type=cond->get_data_type();
	lineno=line;
}
Iteration_Statement_Ast::~Iteration_Statement_Ast(){
	delete cond;
	delete body;
}

Data_Type Iteration_Statement_Ast::get_data_type(){
	return node_data_type;
}
void Iteration_Statement_Ast::set_data_type(Data_Type dt){
	node_data_type=dt;
}

bool Iteration_Statement_Ast::check_ast(){
	//ADD CODE HERE
	// cond->check_ast();
	// body->check_ast();
	CHECK_INVARIANT((cond != NULL), "Condition of Selection_Statement_Ast cannot be null");
	CHECK_INVARIANT((body != NULL), "If part of Selection_Statement_Ast cannot be null");
	return true;
}

void Iteration_Statement_Ast::print(ostream & file_buffer){
	if(is_do_form){
		file_buffer<<"\n         DO (";
		body->print(file_buffer);
		file_buffer<<")";
		file_buffer<<"\n         WHILE CONDITION (";
		cond->print(file_buffer);
		file_buffer<<")";
	} else {
		file_buffer<<"\n         WHILE : \n         CONDITION (";
		cond->print(file_buffer);
		file_buffer<<")\n         BODY (";
		body->print(file_buffer);
		file_buffer<<")";
	}
}

///////////////////////////////////////////////////////////////////////

Data_Type Arithmetic_Expr_Ast::get_data_type()
{
	//ADD CODE HERE ok
	return node_data_type;
}

void Arithmetic_Expr_Ast::set_data_type(Data_Type dt)
{
	//ADD CODE HERE ok
	node_data_type=dt;
}

bool Arithmetic_Expr_Ast::check_ast()
{
	// use get_data_type(), typeid()
	//ADD CODE HERE
	// if(typeid(*this)==typeid(UMinus_Ast)){
	// 	if(typeid(*lhs)!=typeid(Name_Ast)&&typeid(*lhs)!=typeid(Number_Ast<double>)&&typeid(*lhs)!=typeid(Number_Ast<int>))
	// 		lhs->check_ast();
	// 	return true;
	// }
	// if(typeid(*lhs)!=typeid(Name_Ast)&&typeid(*lhs)!=typeid(Number_Ast<double>)&&typeid(*lhs)!=typeid(Number_Ast<int>))
	// 	lhs->check_ast();
	// if(typeid(*rhs)!=typeid(Name_Ast)&&typeid(*rhs)!=typeid(Number_Ast<double>)&&typeid(*rhs)!=typeid(Number_Ast<int>)) 
	// 	rhs->check_ast();
	if (lhs->get_data_type()==rhs->get_data_type()){
		// node_data_type = lhs->get_data_type();
		return true;
	}
	node_data_type=void_data_type;
	CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, "Arithmetic statement data type not compatible", lineno);
	return false;
}

Arithmetic_Expr_Ast::~Arithmetic_Expr_Ast(){
	delete lhs;
	delete rhs;
}

/////////////////////////////////////////////////////////////////////

Plus_Ast::Plus_Ast(Ast * l, Ast * r, int line)
{
	// set arity and data type
	//ADD CODE HERE ok
	lhs = l;
	rhs = r;
	lineno = line;
	ast_num_child=binary_arity;
	if(l->get_data_type()==r->get_data_type()){
		this->node_data_type = l->get_data_type();
	}
	else{
		this->node_data_type = void_data_type;
	} 
}

void Plus_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"\n            Arith: PLUS\n               LHS (";
	lhs->print(file_buffer);
	file_buffer<<")\n               RHS (";
	rhs->print(file_buffer);	
	file_buffer<<")";
}

Plus_Ast::~Plus_Ast(){
	delete lhs;
	delete rhs;
}

/////////////////////////////////////////////////////////////////

Minus_Ast::Minus_Ast(Ast * l, Ast * r, int line)
{
	//ADD CODE HERE ok
	lhs = l;
	rhs = r;
	lineno = line;
	if(l->get_data_type()==r->get_data_type()){
		this->node_data_type = l->get_data_type();
	}
	else{
		this->node_data_type = void_data_type;
	}
}

void Minus_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"\n            Arith: MINUS\n               LHS (";
	lhs->print(file_buffer);
	file_buffer<<")\n               RHS (";
	rhs->print(file_buffer);	
	file_buffer<<")";
}

Minus_Ast::~Minus_Ast(){
	delete lhs;
	delete rhs;
}

//////////////////////////////////////////////////////////////////

Mult_Ast::Mult_Ast(Ast * l, Ast * r, int line)
{
	//ADD CODE HERE ok
	lhs = l;
	rhs = r;
	lineno = line;
	ast_num_child=binary_arity;
	if(l->get_data_type()==r->get_data_type()){
		this->node_data_type = l->get_data_type();
	}
	else{
		this->node_data_type = void_data_type;
	} 
}

void Mult_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"\n            Arith: MULT\n               LHS (";
	lhs->print(file_buffer);
	file_buffer<<")\n               RHS (";
	rhs->print(file_buffer);	
	file_buffer<<")";
}

Mult_Ast::~Mult_Ast(){
	delete lhs;
	delete rhs;
}

////////////////////////////////////////////////////////////////////

Divide_Ast::Divide_Ast(Ast * l, Ast * r, int line)
{
	//ADD CODE HERE ok
	lhs = l;
	rhs = r;
	lineno = line;
	ast_num_child=binary_arity;
	if(l->get_data_type()==r->get_data_type()){
		this->node_data_type = l->get_data_type();
	}
	else{
		this->node_data_type = void_data_type;
	}
}

void Divide_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"\n            Arith: DIV\n               LHS (";
	lhs->print(file_buffer);
	file_buffer<<")\n               RHS (";
	rhs->print(file_buffer);	
	file_buffer<<")";
}

Divide_Ast::~Divide_Ast(){
	delete lhs;
	delete rhs;
}

//////////////////////////////////////////////////////////////////////

Conditional_Operator_Ast::Conditional_Operator_Ast(Ast* cond, Ast* l, Ast* r, int line){
	this->cond = cond;
	lhs = l;
	rhs = r;
	lineno = line;
	ast_num_child = ternary_arity;
	if(lhs->get_data_type()==rhs->get_data_type()){
		this->node_data_type = lhs->get_data_type();
	}
	else{
		this->node_data_type = lhs->get_data_type();
		CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, "Arithmetic statement data type not compatible", lineno);
	}
}

void Conditional_Operator_Ast::print(ostream & file_buffer){
	file_buffer<<"\n            Arith: Conditional\n               COND (";
	cond->print(file_buffer);
	file_buffer<<")\n               LHS (";
	lhs->print(file_buffer);
	file_buffer<<")\n               RHS (";
	rhs->print(file_buffer);	
	file_buffer<<")";
}

// bool Conditional_Operator_Ast::check_ast(){
// 	cond->check_ast();	
// 	if(typeid(*rhs)!=typeid(Name_Ast)&&typeid(*rhs)!=typeid(Number_Ast<double>)&&typeid(*rhs)!=typeid(Number_Ast<int>)) 
// 		rhs->check_ast();
// 	if(typeid(*lhs)!=typeid(Name_Ast)&&typeid(*lhs)!=typeid(Number_Ast<double>)&&typeid(*lhs)!=typeid(Number_Ast<int>))
// 		lhs->check_ast();

// 	if(lhs->get_data_type()==rhs->get_data_type()&&cond->get_data_type()==void_data_type){
// 		node_data_type=lhs->get_data_type();
// 		return true;
// 	}
// 	node_data_type=lhs->get_data_type();
// 	CHECK_INPUT(CONTROL_SHOULD_NOT_REACH, "Arithmetic statement odata type not compatible", lineno);
// 	return false;
// }

Conditional_Operator_Ast::~Conditional_Operator_Ast(){
	delete lhs;
	delete rhs;
	delete cond;
}
//////////////////////////////////////////////////////////////////////

UMinus_Ast::UMinus_Ast(Ast * l, Ast * r, int line)
{
	//ADD CODE HERE ok
	lhs = l;
	rhs = NULL;
	lineno = line;
	ast_num_child=unary_arity;
	node_data_type=l->get_data_type(); 
}

void UMinus_Ast::print(ostream & file_buffer)
{
	//ADD CODE HERE
	file_buffer<<"\n            Arith: UMINUS\n               LHS (";
	lhs->print(file_buffer);
	file_buffer<<")";
}

UMinus_Ast::~UMinus_Ast(){
	delete lhs;
}

Sequence_Ast::Sequence_Ast(int line){
	lineno=line;
}

void Sequence_Ast::ast_push_back(Ast * ast){
	statement_list.push_back(ast);
}
void Sequence_Ast::print(ostream & file_buffer){
	file_buffer<<"\n      Sequence Ast:\n";
	for(list<Ast*>::iterator it=statement_list.begin();it!=statement_list.end();it++){
		(*it)->print(file_buffer);
	}
}
// Code_For_Ast & Sequence_Ast::compile(){
// }
Sequence_Ast::~Sequence_Ast() {
	for(list<Ast*>::iterator it=statement_list.begin();it!=statement_list.end();it++){
		delete *it;
	}
}

/*bool Sequence_Ast::check_ast(){
	for(list<Ast*>::iterator it=statement_list.begin();it!=statement_list.end();it++) {
		(*it)->check_ast();
	}
	return true;
}*/
// void Sequence_Ast::print_assembly(ostream & file_buffer);
// void Sequence_Ast::print_icode(ostream & file_buffer);


template class Number_Ast<double>;
template class Number_Ast<int>;
