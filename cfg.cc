#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <typeinfo>
#include <algorithm>

using namespace std;

#include "common-classes.hh"
#include "error-display.hh"
#include "cfg.hh"
#include "reg-alloc.hh"
#include "symbol-table.hh"
#include "icode.hh"
#include "program.hh"


/*************************** Class Basic_Block *****************************/
typedef long long int lli;

int Basic_Block::block_counter=0;

lli get_operand_id(Ics_Opd* p){
	lli id=0;
	if(typeid(*p)==typeid(Register_Addr_Opd)) {
		if(((Register_Addr_Opd*)p)->get_reg()!=0  && typeid(*((Register_Addr_Opd*)p)->get_reg())==typeid(Register_Descriptor))
			id=(((((Register_Addr_Opd*)p)->get_reg())->get_register())<<2)+1;		//id=...01
	} else if(typeid(*p)==typeid(Mem_Addr_Opd)) {
		id=(lli)&((Mem_Addr_Opd*)p)->get_symbol_entry();				//id=...00 word aligned structures/classes
	} 
	//id=0 for constants and null rhs-opd-register(in case of Uminus)
	
	return id;
}

void print_by_id(lli i){
	if(i%4!=0){
		cout<<machine_desc_object.spim_register_table[(Spim_Register)(i/4)]->get_name()<<" ";
	} else if(i!=0) {
		cout<<((Symbol_Table_Entry*)i)->get_variable_name()<<" ";
	}
}

list<Icode_Stmt*>& Basic_Block::get_icode_list(){
	return sa_block_icode_list;
}

void Basic_Block::push_back(Icode_Stmt *stmt) {
	sa_block_icode_list.push_back(stmt);	
}

set<lli>& Basic_Block::get_kill_set(){
	return kill;
}
set<lli>& Basic_Block::get_gen_set(){
	return gen;
}
set<lli>& Basic_Block::get_in_set(){
	return in;
}
set<lli>& Basic_Block::get_out_set(){
	return out;
}


int Basic_Block::get_block_id()							  {	return block_id;	}
void Basic_Block::set_block_id(int id)					  { block_id=id; }

void Basic_Block::add_child(Basic_Block*child) {
	child_blocks.push_back(child);
}

list<Basic_Block*>& Basic_Block::get_child_blocks(){
	return child_blocks; 
}

void Basic_Block::update_next_ic_block(Basic_Block* new_block) {
	if(child_blocks.size()<=1) return;			// for j stmt
	child_blocks.front()=new_block;
}

void Basic_Block::set_seq_next_block(Basic_Block* next){
	seq_next_block=next;
}
Basic_Block* Basic_Block::get_seq_next_block(){
	return seq_next_block;
}


/*
 Iterate over the block from top to down
 for assignment stmt, conditional stmt and compute ops, add all the opds not in current-kill set(containing vars/regs with l-value access)
 to the gen set.
(kill==l_value)
*/
void Basic_Block::compute_kill_and_gen() {
	set<lli> &l_values=kill;
	l_values.insert(0);					//0 for constants, which are l_values only
	lli opd1,opd2;
	for(auto it:sa_block_icode_list){
		if(typeid(*it)==typeid(Move_IC_Stmt)){
			// cout<<"hahaha...\n";
				opd1= get_operand_id( ((Move_IC_Stmt*)it)->get_opd1() );	
				if(l_values.find(opd1)==l_values.end())
					gen.insert(opd1);
				l_values.insert(get_operand_id( ((Move_IC_Stmt*)it)->get_result() ));
		} else if(typeid(*it)==typeid(Compute_IC_Stmt)){
			// cout<<"hehehe...\n";
				opd1= get_operand_id( ((Compute_IC_Stmt*)it)->get_opd1() );	
				opd2= get_operand_id( ((Compute_IC_Stmt*)it)->get_opd2() );	
				if(l_values.find(opd1)==l_values.end())
					gen.insert(opd1);
				if(l_values.find(opd2)==l_values.end())
					gen.insert(opd2);
				l_values.insert(get_operand_id( ((Compute_IC_Stmt*)it)->get_result() ));
		} else if(typeid(*it)==typeid(Control_Flow_IC_Stmt)){
			// cout<<"hohoho...\n";
				if(((Compute_IC_Stmt*)it)->get_opd1()!=NULL){
					opd1= get_operand_id( ((Compute_IC_Stmt*)it)->get_opd1() );	
					// lli opd2= get_operand_id( ((Compute_IC_Stmt*)*it)->get_opd2() );	
					if(l_values.find(opd1)==l_values.end())
						gen.insert(opd1);
					//no kill
				}	
		} 
		// cout<<"done\n";
				// else if(typeid(*it)==typeid(Label_IC_Stmt)){
		// 	cout<<"...hohohop....\n";	
		// 		//no gen, no kill
		// }
	}
		// cout<<"done\n";
}

/*
compute in out sets for this block
called in Control_Flow_Graph::eliminate_dead_code for all the nodes in a post-order-traversal over the cfg

out=U(in-of-successors)
in=(out-kill) U gen 			//out-kill=out_kill
if in or out get updated return true to indicate this change =>!saturation
*/
bool Basic_Block::compute_in_and_out(){
	bool changed=false;
	// cout<<"Block "<<block_id<<"....\n";
	set<lli> out_next;
	if(child_blocks.size()==0){
		list<Symbol_Table_Entry*>& gst = program_object.get_global_symbol_table().get_variable_table_list();
		set<lli>::iterator it2=out_next.begin();		
		for(auto v:gst){
			it2=out_next.insert(it2,(lli)v);			//value of get_operand_id(opd(v))
		}
	}
	for(auto c_node:child_blocks){
		//union with in[c_node]
		set<lli>::iterator it2=out_next.begin();		
		// cout<<c_node->get_block_id()<<" ";
		for(auto v:c_node->get_in_set()){
			it2=out_next.insert(it2,v);
			// print_by_id(v);
		}
		// cout<<"\n";
	}
	changed=!(out_next==out);
	if(changed){
		out=out_next;
	}
	// cout<<"...\n";
	
	vector<lli> out_kill,in_nextv;

	//compute out_next[node]-kill[node]
	out_kill.resize(out.size());	
	vector<lli>::iterator it=set_difference(out.begin(),out.end(),kill.begin(),kill.end(),out_kill.begin());
	out_kill.resize(it-out_kill.begin());

	// cout<<"out-next "<<block_id<<' ';
	// for(auto i : out) 	print_by_id(i);
	// cout<<" ... kill "<<block_id<<' ';
	// for(auto i : kill) 					print_by_id(i);
	// cout<<"... out-kill "<<block_id<<' ';
	// for(auto i : out_kill)			 	print_by_id(i);

	//compute union of above (diff)set and gen set
	in_nextv.resize(gen.size()+out_kill.size());
	it=set_union(gen.begin(),gen.end(),out_kill.begin(),out_kill.end(),in_nextv.begin());
	in_nextv.resize(it-in_nextv.begin());

	// cout<<" ... gen "<<block_id<<' ';
	// for(auto i : gen) 					print_by_id(i);
	// cout<<" ... in-next "<<block_id<<' ';
	// for(auto i : in_nextv) 					print_by_id(i);
	// cout<<" ... kill "<<block_id<<' ';
	// for(auto i : kill) 					print_by_id(i);

	set<lli> in_next;	
	set<lli>::iterator it2=in_next.begin();
	for(auto v:in_nextv){
		it2=in_next.insert(it2,v);
	}
	if(!(in_next==in)){
		in=in_next;
		changed=true;
	}
	// cout<<" ... in-next "<<block_id<<' ';
	// for(auto i : in_next[block_id]) 					print_by_id(i);

	return changed;
}


/*
Called over all nodes in Control_Flow_Graph::eliminate_dead_code()
begin with out and do a bottom up traversal in current block
on encountering lvalue check if the lvalue-variable is live if not this assignment is dead code
remove this lvalue from the current-live(initially to out and maintained as out below) set and add rvalue occurances to this current-live set
(Finally, though later, we'll use the list of icode generated at the end of this function to generate final icode stmt list :) )
return true if somthing was deleted => !saturation 
*/
bool Basic_Block::eliminate_dead_code(){
	bool changed=false;
	lli opd1, opd2,result;
	for(list<Icode_Stmt*>::iterator it=sa_block_icode_list.end();it!=sa_block_icode_list.begin();){
		it--;
		if(typeid(**it)==typeid(Move_IC_Stmt)){
				opd1= get_operand_id( ((Move_IC_Stmt*)*it)->get_opd1() );	
				result = get_operand_id(((Move_IC_Stmt*)*it)->get_result());
				if(out.find(result)==out.end()){
					delete *it;
					it=sa_block_icode_list.erase(it);
					changed=true;
				} else {
					out.erase(result);
					out.insert(opd1);
				}
		} else if(typeid(**it)==typeid(Compute_IC_Stmt)){
				opd1= get_operand_id( ((Compute_IC_Stmt*)*it)->get_opd1() );	
				opd2= get_operand_id( ((Compute_IC_Stmt*)*it)->get_opd2() );	
				result = get_operand_id(((Compute_IC_Stmt*)*it)->get_result());
				if(out.find(result)==out.end()){
					delete *it;
					it=sa_block_icode_list.erase(it);
					changed=true;
				} else {
					out.erase(result);
					out.insert(opd1);
					out.insert(opd2);
				}
		} else if(typeid(**it)==typeid(Control_Flow_IC_Stmt)){
				if(((Compute_IC_Stmt*)*it)->get_opd1()!=NULL){
					opd1= get_operand_id( ((Compute_IC_Stmt*)*it)->get_opd1() );	
					out.insert(opd1);
				}	
		}
	}
	return changed;
}


/*************************** Class Control_Flow_Graph *****************************/

/*
Construct a CFG of the code.
Break points are jump/goto or branch stmt and label stmts

we maintain last_block so that if there is a label stmt current block becomes the already created label-block
hence we need to change the successor of the last_block. This only occurs for a branch stmt(not jump though). also we need to change the
successor node in the sequential-code-order-ignoring-control-flow sequence (Needed to get the listof icode back from ths cfg's blocks icode-list).

*/
Control_Flow_Graph::Control_Flow_Graph(list<Icode_Stmt*>&sa_icode_list){
	Basic_Block::block_counter=0;
	root=new Basic_Block;
	root->set_seq_next_block(NULL);
	Basic_Block* last_block=NULL, *current_block=root;
	current_block->set_block_id(Basic_Block::block_counter++);
	current_block->set_seq_next_block(NULL);
	bool is_empty=true;
	for(auto it:sa_icode_list) {
		if(typeid(*it)==typeid(Control_Flow_IC_Stmt)) {
				current_block->push_back(it);
				int label_no = atoi(((Control_Flow_IC_Stmt*)it)->get_Offset().substr(5).c_str());		//label#
				if(label_block_table.find(label_no)==label_block_table.end()){							//create a block for this label
					Basic_Block* new_block=new Basic_Block;
					new_block->set_block_id(Basic_Block::block_counter++);			
					// cout<<"made label "<<Basic_Block::block_counter-1<<" "<<label_no<<endl;
					new_block->set_seq_next_block(NULL);
					label_block_table.insert(pair<int,Basic_Block*>(label_no,new_block));
				}
				Basic_Block* new_block=new Basic_Block;					// create new block for next set of statements
				new_block->set_block_id(Basic_Block::block_counter++);
				// cout<<"made label "<<Basic_Block::block_counter-1<<" "<<label_no<<endl;
				new_block->set_seq_next_block(NULL);
				if(((Control_Flow_IC_Stmt*)it)->get_inst_op_of_ics().get_op()!=j)
					current_block->add_child(new_block);				// set next sequential block as child for bne/beq(not j)
				current_block->add_child(label_block_table[label_no]);	// set label block as other child pointer 
				current_block->set_seq_next_block(new_block);
				last_block=current_block;								// current block becomes last block
				current_block=new_block;								// set current block as new block
				is_empty=true;											// current block is empty
		} else if(typeid(*it)==typeid(Label_IC_Stmt)) {
				if(is_empty && last_block){
					Basic_Block::block_counter--;	// last new block to be deleted update the label_counter now	
				}
				Basic_Block* new_block;
				int label_no = atoi(((Label_IC_Stmt*)it)->get_offset().substr(5).c_str());		//label#
				if(label_block_table.find(label_no)==label_block_table.end()){					// make new block for label	
					new_block = new Basic_Block;											
					new_block->set_block_id(Basic_Block::block_counter++);
					// cout<<"made label "<<Basic_Block::block_counter-1<<" "<<label_no<<endl;
					new_block->set_seq_next_block(NULL);
					label_block_table.insert(pair<int,Basic_Block*>(label_no,new_block));
				} else {
					new_block = label_block_table[label_no];
				}

				new_block->push_back(it);
				if(is_empty){										  // don't want a basic block to be empty(although no harm in keeping it to be such :P)	
					if(last_block){									  // if there exist last block	
						last_block->update_next_ic_block(new_block);  // update parents child to correct label block
						last_block->set_seq_next_block(new_block);		
						delete current_block;
						// cout<<"delete label "<<Basic_Block::block_counter-1<<" "<<label_no<<endl;
						// it->print_icode(cout);
					} else if(current_block==root) {	//root was empty
						// cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
						delete root;
						root=new_block;
					}
				} else {
					current_block->add_child(new_block);				//if last block is not empty and hence not deletedset successors
					current_block->set_seq_next_block(new_block);
				}
				// cout<<"next for label: "<<new_block->get_seq_next_block()<<endl;
				new_block->set_seq_next_block(NULL);
				last_block=NULL;						// don't need last block, current block is non empty won't be deleted
				is_empty=false;							// current block has label stmt => !empty
				current_block = new_block;
		} else {										//compute or move statements
				current_block->push_back(it);
				last_block=NULL;
				is_empty=false;
		}
	}

	// print(false,false);
}

/*
Print the cfg in this format
	Block id 
	Child Blocks id id2 id3...
	--first-stmt-of-block--
	...
	--last-stmt-of-block--
	kill-gen:
	--kill-set-with-var-names--
	--gen-set-with-var-names--
	in-out:
	--in-set-with-var-names--
	--out-set-with-var-names--
*/
void Control_Flow_Graph::print(bool killgen, bool inout){
	//printinf the cfg
	vector<bool> seen(Basic_Block::block_counter,false);
	stack<Basic_Block*> dfs_stack;
	dfs_stack.push(root);
	seen[root->get_block_id()]=true;
	while(!dfs_stack.empty()){
		Basic_Block* node=dfs_stack.top();
		dfs_stack.pop();
		cout<<"Block "<<node->get_block_id()<<"\n\t Child Blocks";
		for(auto c_node:node->get_child_blocks()){
			// cout<<"1...\n";
			cout<<" "<<c_node->get_block_id();
			// cout<<"2...\n";
			if(!seen[c_node->get_block_id()]){
				seen[c_node->get_block_id()]=true;
				dfs_stack.push(c_node);
			}
		}
		// cout<<"\nhere\n"<<node<<endl;
		if(node->get_icode_list().size()>0)
			node->get_icode_list().front()->print_icode(cout);
		cout<<"\t...\n";
		if(node->get_icode_list().size()>0)
			node->get_icode_list().back()->print_icode(cout);
		if(killgen){
			cout<<"kill-gen:\n";
			for(auto i : node->get_kill_set()) print_by_id(i);
			cout<<'\n';
			for(auto i : node->get_gen_set()) print_by_id(i);
			cout<<'\n';
		}
		if(inout){
			cout<<"in-out:\n";
			for(auto i : node->get_in_set()) print_by_id(i);
			cout<<'\n';
			for(auto i : node->get_out_set()) print_by_id(i);
			cout<<'\n';
		}
			
	}
}

/*
Simple post order traversal for one iteration of the fixed point method
*/
bool post_order_traversal(Basic_Block* node,vector<bool>&seen){
	bool changed=false;
	//do dfs over the cfg and update the in, out sets until convergence
	seen[node->get_block_id()]=true;
	for(auto c_node:node->get_child_blocks()){
		if(!seen[c_node->get_block_id()]){
			changed = post_order_traversal(c_node,seen) or changed;
		}
	}
	changed = node->compute_in_and_out() or changed;
	return changed;
}

/*
1. compute kill gen sets
2. compute in out sets using fixed point method
3. do dead code elimination based on above sets on all the blocks locally
*/
bool Control_Flow_Graph::eliminate_dead_code(){
	//do dfs over the cfg and compute kill gen sets for all blocks	
	Basic_Block* node=root;
	// cout<<"edc ...\n";
	// cout<<Basic_Block::block_counter<<endl;
	while(node!=NULL){
		// cout<<"starting...\n";
		node->compute_kill_and_gen();
		// cout<<"block "<<node->get_block_id()<<" "<<endl;
		// if(node->get_icode_list().size()==0)
		// 	cout<<"error at "<<node->get_block_id();
		// else	
		// 	node->get_icode_list().front()->print_icode(cout);
		node=node->get_seq_next_block();
		// cout<<"ending...\n";
	}
	
	// cout<<"ahead "<<endl;
	bool changed=false;
	vector<bool> seen(Basic_Block::block_counter,false);
	do {
		for(int i=0;i<Basic_Block::block_counter;i++)  seen[i]=false;
		changed=post_order_traversal(root,seen);
	} while(changed);
	// print(true,true);

	changed=false;
	node=root;
	while(node!=NULL){
		changed=node->eliminate_dead_code() or changed;
		// cout<<changed;
		node->get_in_set().clear();
		node->get_out_set().clear();
		node->get_kill_set().clear();
		node->get_gen_set().clear();
		node=node->get_seq_next_block();
	}
	return changed;
}

/*
construct the icode list by patching the icode stmts of the basic block after dead code elimination 
in the in the order of the original program(use seq_next_block)
*/
void Control_Flow_Graph::make_icode_list(list<Icode_Stmt*>&sa_icode_list){
	sa_icode_list.clear();
	Basic_Block* node=root;
	while(node!=NULL){
		sa_icode_list.splice(sa_icode_list.end(),node->get_icode_list());
		node=node->get_seq_next_block();
	}
}
