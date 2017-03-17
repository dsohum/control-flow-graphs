#include <iostream>
#include <fstream>
#include <typeinfo>

using namespace std;

#include "common-classes.hh"
#include "error-display.hh"
#include "user-options.hh"
#include "icode.hh"
#include "reg-alloc.hh"
#include "symbol-table.hh"
#include "ast.hh"
#include "procedure.hh"
#include "program.hh"

Code_For_Ast & get_code_for_ast(Ast* rhs_op,Tgt_Op op,Register_Use_Category dt,const char *ast_type);
Code_For_Ast & get_code_for_ast(Ast* lhs_op, Ast* rhs_op,Tgt_Op op,Register_Use_Category dt,const char *ast_type);

Code_For_Ast & Ast::create_store_stmt(Register_Descriptor * store_register)
{
	stringstream msg;
	msg << "No create_store_stmt() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

void Ast::print_assembly()
{
	stringstream msg;
	msg << "No print_assembly() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

void Ast::print_icode()
{
	stringstream msg;
	msg << "No print_icode() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

////////////////////////////////////////////////////////////////

Code_For_Ast & Assignment_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null in Assignment_Ast");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null in Assignment_Ast");

	Code_For_Ast & load_stmt = rhs->compile();

	Register_Descriptor * load_register = load_stmt.get_reg();
	CHECK_INVARIANT(load_register, "Load register cannot be null in Assignment_Ast");
	load_register->set_use_for_expr_result();

	Code_For_Ast store_stmt = lhs->create_store_stmt(load_register);

	CHECK_INVARIANT((load_register != NULL), "Load register cannot be null in Assignment_Ast");
	load_register->reset_use_for_expr_result();

	// Store the statement in ic_list

	// list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;
	list<Icode_Stmt *>  ic_list ;

	if (load_stmt.get_icode_list().empty() == false)
		ic_list = load_stmt.get_icode_list();

	if (store_stmt.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), store_stmt.get_icode_list());

	Code_For_Ast * assign_stmt;
	if (ic_list.empty() == false)
		assign_stmt = new Code_For_Ast(ic_list, load_register);
	return *assign_stmt;
}


/////////////////////////////////////////////////////////////////

Code_For_Ast & Name_Ast::compile()
{
	CHECK_INVARIANT((variable_symbol_entry != NULL), "Variable symbol entry cannot be null in Name_Ast");

	//create move statement r<-m
	Mem_Addr_Opd* var = new Mem_Addr_Opd(*variable_symbol_entry);
	Register_Descriptor* newtemp;
	Move_IC_Stmt *mv_stmt;
	//watch out for :gp_data vs float_reg
	
	if(node_data_type==int_data_type){
		newtemp = machine_desc_object.get_new_register<gp_data>();
		Register_Addr_Opd* result = new Register_Addr_Opd(newtemp);
		mv_stmt = new Move_IC_Stmt(load,var,result);
	} else if(double_data_type){
		newtemp = machine_desc_object.get_new_register<float_reg>();
		Register_Addr_Opd* result = new Register_Addr_Opd(newtemp);
		mv_stmt = new Move_IC_Stmt(load_d,var,result);
	}

	list<Icode_Stmt *> ic_list;
	ic_list.push_back(mv_stmt);
	Code_For_Ast * name_ast = new Code_For_Ast(ic_list,newtemp);
	return *name_ast;
}

Code_For_Ast & Name_Ast::create_store_stmt(Register_Descriptor * store_register)
{

	CHECK_INVARIANT((variable_symbol_entry != NULL), "Variable symbol entry cannot be null in Name_Ast");

	//create move statement m<-r
	Mem_Addr_Opd* result = new Mem_Addr_Opd(*variable_symbol_entry);
	Register_Addr_Opd *opd1 = new Register_Addr_Opd(store_register);
	Move_IC_Stmt *mv_stmt;
	switch(node_data_type){
	case int_data_type : 	mv_stmt = new Move_IC_Stmt(store,opd1,result);  	break;
	case double_data_type : mv_stmt = new Move_IC_Stmt(store_d,opd1,result);
	}

	list<Icode_Stmt *> ic_l;
	ic_l.push_back(mv_stmt);
	Code_For_Ast *store_stmt = new Code_For_Ast(ic_l,store_register);
	return *store_stmt;
}


///////////////////////////////////////////////////////////////////////////////

template <class DATA_TYPE>
Code_For_Ast & Number_Ast<DATA_TYPE>::compile()
{
	//create move statement r<-c
	Const_Opd<DATA_TYPE>* const_opd = new Const_Opd<DATA_TYPE>(constant);
	//watch out for: gp_data vs float_reg
	Register_Descriptor* newtemp;
	Move_IC_Stmt *mv_stmt;
	if(node_data_type==int_data_type){
	 	newtemp = machine_desc_object.get_new_register<gp_data>();
		Register_Addr_Opd* result = new Register_Addr_Opd(newtemp);
		mv_stmt = new Move_IC_Stmt(imm_load,const_opd,result);
	} else if (node_data_type==double_data_type) {
		newtemp = machine_desc_object.get_new_register<float_reg>(); 
		Register_Addr_Opd* result = new Register_Addr_Opd(newtemp);
		mv_stmt = new Move_IC_Stmt(imm_load_d,const_opd,result);
	}

	list<Icode_Stmt *>  ic_list;
	ic_list.push_back(mv_stmt);
	Code_For_Ast * number_ast = new Code_For_Ast(ic_list,newtemp);
	return *number_ast;
}

///////////////////////////////////////////////////////////////////////////////

Code_For_Ast & Relational_Expr_Ast::compile()
{
	CHECK_INVARIANT((lhs_condition != NULL), "Lhs condition cannot be null in Relational_Expr_Ast");
	CHECK_INVARIANT((rhs_condition != NULL), "Rhs condition cannot be null in Relational_Expr_Ast");

	Tgt_Op op;
	switch(rel_op){
	case less_equalto:		op=sle; break;
	case less_than:			op=slt; break;
	case greater_than:		op=sgt; break;
	case greater_equalto:	op=sge; break;
	case equalto:			op=seq; break;
	case not_equalto:		op=sne; break;	
	}
	//watch out for: fixed_reg vs float_reg
	Code_For_Ast & arith_expr = get_code_for_ast(lhs_condition,rhs_condition,op,gp_data,"Relational_Expr_Ast");
	return arith_expr;			

}

//////////////////////////////////////////////////////////////////////

Code_For_Ast & Boolean_Expr_Ast::compile()
{
	CHECK_INVARIANT((lhs_op != NULL)||(ast_num_child==unary_arity), "Lhs op cannot be null in Boolean_Expr_Ast");
	CHECK_INVARIANT((rhs_op != NULL), "Rhs op cannot be null in Boolean_Expr_Ast");

	Tgt_Op op;
	Code_For_Ast* boolean_expr;
	//watch out for: fixed_reg vs float_reg
	Register_Use_Category reg_use_cat=gp_data;//(node_data_type==int_data_type)?gp_data:float_reg;
	switch(bool_op){
		case boolean_and: 
			op=and_t;
			boolean_expr= &get_code_for_ast(lhs_op,rhs_op,op,reg_use_cat,"Boolean_Expr_Ast");
			break;
		case boolean_or : 
			op=or_t;  
			boolean_expr= &get_code_for_ast(lhs_op,rhs_op,op,reg_use_cat,"Boolean_Expr_Ast"); 
			break;
		case boolean_not: 
			op=not_t;
			Number_Ast<int>* rhs = new Number_Ast<int>(1,int_data_type,lineno);
			boolean_expr= &get_code_for_ast(rhs_op,rhs,op,reg_use_cat,"Boolean_Expr_Ast");
	 		break;
	}
	 
	return *boolean_expr;		
}
///////////////////////////////////////////////////////////////////////

Code_For_Ast & Selection_Statement_Ast::compile()
{
	CHECK_INVARIANT((cond != NULL), "Condition cannot be null in Selection_Statement_Ast");
	CHECK_INVARIANT((then_part != NULL), "Then part cannot be null in Selection_Statement_Ast");
	CHECK_INVARIANT((else_part != NULL), "Else part cannot be null in Selection_Statement_Ast");

	//get labels to else-start and if-else-end points
	int else_label,end_label;
	Code_For_Ast & cond_cfa = cond->compile();			//compile condition
	cond_cfa.get_reg()->set_use_for_expr_result();		//set cond register for use
	Code_For_Ast & then_part_cfa = then_part->compile();//compile then part	
	Code_For_Ast & else_part_cfa = else_part->compile();//compile else part
	else_label = Ast::labelCounter++;					//get else point label
	end_label = Ast::labelCounter++;					//get end point label
	
	CHECK_INVARIANT((cond_cfa.get_reg()!=NULL),"Condition register cannot be null in Selection_Statement_Ast");
	Register_Addr_Opd * opd1 = new Register_Addr_Opd(cond_cfa.get_reg());
	Register_Addr_Opd * zero_reg = new Register_Addr_Opd(machine_desc_object.spim_register_table[zero]);
	Control_Flow_IC_Stmt* if_part =  new Control_Flow_IC_Stmt(beq, opd1, zero_reg, string("label")+to_string(else_label));	//branch to else
	cond_cfa.get_reg()->reset_use_for_expr_result();		//free cond register
	
	Label_IC_Stmt* else_label_stmt = new Label_IC_Stmt( label, NULL , string("label")+to_string(else_label));			//else label
	Label_IC_Stmt* end_label_stmt = new Label_IC_Stmt( label, NULL , string("label")+to_string(end_label));				//end label	
	Label_IC_Stmt* goto_stmt = new Label_IC_Stmt( j, NULL , string("label")+to_string(end_label));						//goto end-label(for then)
	
	//put together statements in a list in order
	list<Icode_Stmt*> ic_list;
	if(cond_cfa.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(),cond_cfa.get_icode_list());		//condition
	ic_list.push_back(if_part);											//branch statement
	if(then_part_cfa.get_icode_list().empty() == false)					
		ic_list.splice(ic_list.end(),then_part_cfa.get_icode_list());	//then_part body
	ic_list.push_back(goto_stmt);										//goto end(for then part)
	ic_list.push_back(else_label_stmt);									//else label
	if(else_part_cfa.get_icode_list().empty() == false)					
		ic_list.splice(ic_list.end(),else_part_cfa.get_icode_list());	//else part body
	ic_list.push_back(end_label_stmt);									//end label
	Code_For_Ast* selection_stmt = new Code_For_Ast(ic_list,NULL);
	return *selection_stmt;
}

///////////////////////////////////////////////////////////////////////////////////////////

Code_For_Ast & Iteration_Statement_Ast::compile()
{
	CHECK_INVARIANT((cond != NULL), "Condition part cannot be null in Iteration_Statement_Ast");
	CHECK_INVARIANT((body != NULL), "Body part cannot be null in Iteration_Statement_Ast");

	//compile body and condition in order and free the result of cond-code-for-ast-register
	//[nothing in between branch check and value computation;   also, branch does not create a register]
	Code_For_Ast & cond_cfa = cond->compile();	
	cond_cfa.get_reg()->set_use_for_expr_result();
	//get labels for loop-body-start and loop-body-end
	int start_label, end_label;	
	start_label = Ast::labelCounter++;
	end_label = Ast::labelCounter++;
	Code_For_Ast & body_cfa = body->compile();	
	
	//make branch statement
	Register_Addr_Opd * opd1 = new Register_Addr_Opd(cond_cfa.get_reg());
	Register_Addr_Opd * zero_reg = new Register_Addr_Opd(machine_desc_object.spim_register_table[zero]);
	Control_Flow_IC_Stmt* branch_stmt =  new Control_Flow_IC_Stmt(bne, opd1, zero_reg, string("label")+to_string(start_label));  //branch to start label
	cond_cfa.get_reg()->reset_use_for_expr_result();

	//make all the labels and goto statements
	Label_IC_Stmt* start_label_stmt = new Label_IC_Stmt( label, NULL , string("label")+to_string(start_label));	//loop body start label
	Label_IC_Stmt* end_label_stmt = new Label_IC_Stmt( label, NULL , string("label")+to_string(end_label));		//loop body end label
	Label_IC_Stmt* begin_goto_stmt;																				//goto statement for while
	if(!is_do_form)
		begin_goto_stmt = new Label_IC_Stmt( j, NULL , string("label")+to_string(end_label));					//goto loop-body end to check condition first
	
	//get all the statements in a list in order
	list<Icode_Stmt*> ic_list;
	if(!is_do_form)
		ic_list.push_back(begin_goto_stmt);
	ic_list.push_back(start_label_stmt);							//loop-start-label
	if(body_cfa.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(),body_cfa.get_icode_list());	//loop-body
	ic_list.push_back(end_label_stmt);								//loop-end-label
	if(cond_cfa.get_icode_list().empty() == false)					
		ic_list.splice(ic_list.end(),cond_cfa.get_icode_list());	//condition
	ic_list.push_back(branch_stmt);									//branch to start
	Code_For_Ast* iteration_stmt = new Code_For_Ast(ic_list,NULL);
	return *iteration_stmt;	
}

///////////////////////////////////////////////////////////////////////////////////////////

Code_For_Ast & Plus_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs op cannot be null in Plus_Ast");
	CHECK_INVARIANT((rhs != NULL), "Rhs op cannot be null in Plus_Ast");

	//watch out for: fixed_reg vs float_reg
	Register_Use_Category reg_use_cat;
	Tgt_Op op;
	switch(node_data_type){
	case int_data_type    :  reg_use_cat=gp_data;   op=add;   break;
	case double_data_type :  reg_use_cat=float_reg; op=add_d; break;
	}
	Code_For_Ast & arith_expr = get_code_for_ast(lhs,rhs,op,reg_use_cat,"Plus_Ast");
	return arith_expr;			
}

/////////////////////////////////////////////////////////////////

Code_For_Ast & Minus_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs op cannot be null in Minus_Ast");
	CHECK_INVARIANT((rhs != NULL), "Rhs op cannot be null in Minus_Ast");

	//watch out for: fixed_reg vs float_reg
	Register_Use_Category reg_use_cat;
	Tgt_Op op;
	switch(node_data_type){
	case int_data_type    :  reg_use_cat=gp_data;   op=sub;   break;
	case double_data_type :  reg_use_cat=float_reg; op=sub_d; break;
	}
	Code_For_Ast & arith_expr = get_code_for_ast(lhs,rhs,op,reg_use_cat,"Minus_Ast");
	return arith_expr;			
}

//////////////////////////////////////////////////////////////////

Code_For_Ast & Mult_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs op cannot be null in Mult_Ast");
	CHECK_INVARIANT((rhs != NULL), "Rhs op cannot be null in Mult_Ast");

	//watch out for: fixed_reg vs float_reg
	Register_Use_Category reg_use_cat;
	Tgt_Op op;
	switch(node_data_type){
	case int_data_type    :  reg_use_cat=gp_data;   op=mult;   break;
	case double_data_type :  reg_use_cat=float_reg; op=mult_d; break;
	}
	Code_For_Ast & arith_expr = get_code_for_ast(lhs,rhs,op,reg_use_cat,"Mult_Ast");
	return arith_expr;				
}

////////////////////////////////////////////////////////////////////

Code_For_Ast & Conditional_Operator_Ast::compile()
{
	CHECK_INVARIANT((cond != NULL), "Condition cannot be null in Conditional_Operator_Ast");
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null in Conditional_Operator_Ast");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null in Conditional_Operator_Ast");

	//get labels to else-start and if-else-end points
	int else_label,end_label;	
	Code_For_Ast & cond_cfa = cond->compile();			//compile condition
	CHECK_INVARIANT((cond_cfa.get_reg()!=NULL),"Condition register cannot be null in Selection_Statement_Ast");
	cond_cfa.get_reg()->set_use_for_expr_result();	//save register of condition
	
	Code_For_Ast & lhs_cfa = lhs->compile();			//compile lhs part	
	CHECK_INVARIANT((lhs_cfa.get_reg()!=NULL),"Lhs register cannot be null in Selection_Statement_Ast");
	lhs_cfa.get_reg()->set_use_for_expr_result();	//save register of condition
	else_label = Ast::labelCounter++;					//get else point label

	Code_For_Ast & rhs_cfa = rhs->compile();			//compile rhs part
	CHECK_INVARIANT((rhs_cfa.get_reg()!=NULL),"Rhs register cannot be null in Selection_Statement_Ast");
	rhs_cfa.get_reg()->set_use_for_expr_result();	//save register of condition
	end_label = Ast::labelCounter++;					//get end point label
	
	Register_Descriptor * newtemp;
	switch(node_data_type){
	case int_data_type:		newtemp = machine_desc_object.get_new_register<gp_data>();  break;
	case double_data_type:  newtemp = machine_desc_object.get_new_register<float_reg>();
	}
	
	Register_Addr_Opd * opd1 = new Register_Addr_Opd(cond_cfa.get_reg());
	Register_Addr_Opd * zero_reg = new Register_Addr_Opd(machine_desc_object.spim_register_table[zero]);
	Control_Flow_IC_Stmt* if_part =  new Control_Flow_IC_Stmt(beq, opd1, zero_reg, string("label")+to_string(else_label));	//branch to rhs/else
	cond_cfa.get_reg()->reset_use_for_expr_result();	//free cond register 

	Register_Addr_Opd * result = new Register_Addr_Opd(newtemp);
	opd1 = new Register_Addr_Opd(lhs_cfa.get_reg());
	Compute_IC_Stmt *lhs_or_stmt = new Compute_IC_Stmt(or_t,result,opd1,zero_reg);
	lhs_cfa.get_reg()->reset_use_for_expr_result();		//free lhs register 
	opd1 = new Register_Addr_Opd(rhs_cfa.get_reg());
	Compute_IC_Stmt *rhs_or_stmt = new Compute_IC_Stmt(or_t,result,opd1,zero_reg);
	rhs_cfa.get_reg()->reset_use_for_expr_result();		//free rhs register 

	Label_IC_Stmt* else_label_stmt = new Label_IC_Stmt( label, NULL , string("label")+to_string(else_label));			//else label
	Label_IC_Stmt* end_label_stmt = new Label_IC_Stmt( label, NULL , string("label")+to_string(end_label));				//end label	
	Label_IC_Stmt* goto_stmt = new Label_IC_Stmt( j, NULL , string("label")+to_string(end_label));						//goto end-label(for then)
	
	//put together statements in a list in order
	list<Icode_Stmt*> ic_list;
	if(cond_cfa.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(),cond_cfa.get_icode_list());		//condition
	ic_list.push_back(if_part);											//branch statement
	if(lhs_cfa.get_icode_list().empty() == false)					
		ic_list.splice(ic_list.end(),lhs_cfa.get_icode_list());	//lhs body
	ic_list.push_back(lhs_or_stmt);
	ic_list.push_back(goto_stmt);										//goto end(for then part)
	ic_list.push_back(else_label_stmt);									//else label
	if(rhs_cfa.get_icode_list().empty() == false)					
		ic_list.splice(ic_list.end(),rhs_cfa.get_icode_list());	//rhs body
	ic_list.push_back(rhs_or_stmt);
	ic_list.push_back(end_label_stmt);									//end label
	Code_For_Ast* selection_stmt = new Code_For_Ast(ic_list,newtemp);
	return *selection_stmt;

}


////////////////////////////////////////////////////////////////////

Code_For_Ast & Divide_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs op cannot be null in Mult_Ast");
	CHECK_INVARIANT((rhs != NULL), "Rhs op cannot be null in Mult_Ast");

	//watch out for: fixed_reg vs float_reg
		Register_Use_Category reg_use_cat;
	Tgt_Op op;
	switch(node_data_type){
	case int_data_type    :  reg_use_cat=gp_data;   op=divd;   break;
	case double_data_type :  reg_use_cat=float_reg; op=div_d; break;
	}
	Code_For_Ast & arith_expr = get_code_for_ast(lhs,rhs,op,reg_use_cat,"Divide_Ast");
	return arith_expr;					
}


//////////////////////////////////////////////////////////////////////

Code_For_Ast & UMinus_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs op cannot be null in Mult_Ast");

	//watch out for: fixed_reg vs float_reg
		Register_Use_Category reg_use_cat;
	Tgt_Op op;
	switch(node_data_type){
	case int_data_type    :  reg_use_cat=gp_data;   op=uminus;   break;
	case double_data_type :  reg_use_cat=float_reg; op=uminus_d; break;
	}
	Code_For_Ast & arith_expr = get_code_for_ast(lhs,op,reg_use_cat,"reg_use_cat");
	return arith_expr;						
}

//////////////////////////////////////////////////////////////////////////////

Code_For_Ast & Sequence_Ast::compile()
{
	for(list<Ast*>::iterator it=statement_list.begin();it!=statement_list.end();it++){
		Code_For_Ast& code = (*it)->compile();
		//cout<<code.get_icode_list().size()<<" "<<sa_icode_list.size()<<";";
		if(code.get_icode_list().empty() == false)
			sa_icode_list.splice(sa_icode_list.end(),code.get_icode_list());
	}
	list<Icode_Stmt*> ic_list = sa_icode_list;
	Code_For_Ast* sa = new Code_For_Ast(ic_list,NULL);
	return *sa;
}

void Sequence_Ast::print_assembly(ostream & file_buffer)
{
	//cout<<sa_icode_list.size();
	for(list<Icode_Stmt*>::iterator it = sa_icode_list.begin();it!=sa_icode_list.end();it++){
		(*it)->print_assembly(file_buffer);
	}	
}

void Sequence_Ast::print_icode(ostream & file_buffer)
{
	for(list<Icode_Stmt*>::iterator it = sa_icode_list.begin();it!=sa_icode_list.end();it++){
		(*it)->print_icode(file_buffer);
	}
}

//////////////////////////////////////////////////////////////////////////////

template class Number_Ast<double>;
template class Number_Ast<int>;

//helper functions to set up Code_FOr-_Ast for expressoins
Code_For_Ast & get_code_for_ast(Ast* lhs_op, Ast* rhs_op,Tgt_Op op,const Register_Use_Category dt,const char *ast_type){


	Code_For_Ast *lhs_expr, *rhs_expr;
	Register_Descriptor *lhs_expr_register, *rhs_expr_register;

	if(op!=not_t){
		//compile lhs node
		lhs_expr = &lhs_op->compile();
		lhs_expr_register = lhs_expr->get_reg();
		CHECK_INVARIANT(lhs_expr_register, string("Lhs register cannot be null in ")+ast_type);
		lhs_expr_register->set_use_for_expr_result();

		//compile rhs node
		rhs_expr = &rhs_op->compile();
		rhs_expr_register = rhs_expr->get_reg();
		CHECK_INVARIANT(rhs_expr_register, string("Rhs register cannot be null in ")+ast_type);
		rhs_expr_register->set_use_for_expr_result();
	} else {
		//compile rhs node
		rhs_expr = &rhs_op->compile();
		rhs_expr_register = rhs_expr->get_reg();
		CHECK_INVARIANT(rhs_expr_register, string("Rhs register cannot be null in ")+ast_type);
		rhs_expr_register->set_use_for_expr_result();

		//compile lhs node
		lhs_expr = &lhs_op->compile();
		lhs_expr_register = lhs_expr->get_reg();
		CHECK_INVARIANT(lhs_expr_register, string("Lhs register cannot be null in ")+ast_type);
		lhs_expr_register->set_use_for_expr_result();
	}

	//create a compute statement with corresponding registers and operation
	Register_Descriptor* newtemp;
	switch(dt){
	case gp_data:	newtemp = machine_desc_object.get_new_register<gp_data>();		break;
	case float_reg: newtemp = machine_desc_object.get_new_register<float_reg>();	
	}
	Register_Addr_Opd* result = new Register_Addr_Opd(newtemp);				//newtemp()
	Register_Addr_Opd* opd1 = new Register_Addr_Opd(lhs_expr_register);		//opd1
	Register_Addr_Opd* opd2 = new Register_Addr_Opd(rhs_expr_register);		//opd2
	Compute_IC_Stmt* stmt = new Compute_IC_Stmt(op,result,opd1,opd2);		//compute statement for op on opd1 and opd2 and result into newtemp
	lhs_expr_register->reset_use_for_expr_result();							//free left register (used only in parent,i.e. here)
	rhs_expr_register->reset_use_for_expr_result();							//free the right expr (used only in parent,i.e. here)

	// Store the statement in ic_list
	list<Icode_Stmt *> ic_list;
	if(op!=not_t){
		if (lhs_expr->get_icode_list().empty() == false)
			ic_list.splice(ic_list.end(), lhs_expr->get_icode_list());			//lhs body
		if (rhs_expr->get_icode_list().empty() == false)
			ic_list.splice(ic_list.end(),rhs_expr->get_icode_list());			//rhs body
	} else {
		if (rhs_expr->get_icode_list().empty() == false)
			ic_list.splice(ic_list.end(),rhs_expr->get_icode_list());			//rhs body		
		if (lhs_expr->get_icode_list().empty() == false)
			ic_list.splice(ic_list.end(), lhs_expr->get_icode_list());			//lhs body
	}
	ic_list.push_back(stmt);												//compute statement

	Code_For_Ast * expr = new Code_For_Ast(ic_list, newtemp);
	return *expr;		
}

Code_For_Ast & get_code_for_ast(Ast* rhs_op,Tgt_Op op,const Register_Use_Category dt,const char *ast_type){

	//compile rhs node
	Code_For_Ast & rhs_expr = rhs_op->compile();
	Register_Descriptor * rhs_expr_register = rhs_expr.get_reg();
	CHECK_INVARIANT(rhs_expr_register, string("Rhs register cannot be null in ")+ast_type);
	rhs_expr_register->set_use_for_expr_result();

	//create a compute statement with corresponding registers and operation
	Register_Descriptor* newtemp;
	switch(dt){
	case gp_data:	newtemp = machine_desc_object.get_new_register<gp_data>();	 break;
	case float_reg: newtemp = machine_desc_object.get_new_register<float_reg>();
	} 
	Register_Addr_Opd* result = new Register_Addr_Opd(newtemp);			//newtemp()
	Register_Addr_Opd* opd1 = new Register_Addr_Opd(rhs_expr_register);	//opd1
	Register_Addr_Opd* opd2 = new Register_Addr_Opd(NULL);	//opd2
	Compute_IC_Stmt* stmt = new Compute_IC_Stmt(op,result,opd1,opd2);   //compute statement for op on opd1 and result into newtemp
	rhs_expr_register->reset_use_for_expr_result();						//free the right expr (used only in parent,i.e. here)

	// Store the statement in ic_list
	list<Icode_Stmt *>  ic_list;
	if (rhs_expr.get_icode_list().empty() == false)					
		ic_list.splice(ic_list.end(),rhs_expr.get_icode_list());		//rhs body
	ic_list.push_back(stmt);											//compute statement	

	Code_For_Ast * expr = new Code_For_Ast(ic_list, newtemp);
	return *expr;		
}