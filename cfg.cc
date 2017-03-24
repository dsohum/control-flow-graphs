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
		// cout<<"here "<<id<<endl;
		// cout<<p<<endl;
		// cout<<((Register_Addr_Opd*)p)->get_reg()<<endl;
		if(((Register_Addr_Opd*)p)->get_reg()!=0  && typeid(*((Register_Addr_Opd*)p)->get_reg())==typeid(Register_Descriptor))
			id=(((((Register_Addr_Opd*)p)->get_reg())->get_register())<<2)+1;		//id=...01
		// cout<<"here "<<id<<endl;
	} else if(typeid(*p)==typeid(Mem_Addr_Opd)) {
		// cout<<"here instead!\n";
		id=(lli)&((Mem_Addr_Opd*)p)->get_symbol_entry();				//id=...00 word aligned structures/classes
		// cout<<"here instead!\n";
	} 
	//id=0 for constants
	
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

bool Basic_Block::update_next_ic_block(Basic_Block* new_block) {
	if(child_blocks.size()<=1) return false;			// for j stmt
	child_blocks.front()=new_block;
	return true;
}

void Basic_Block::set_seq_next_block(Basic_Block* next){
	seq_next_block=next;
}
Basic_Block* Basic_Block::get_seq_next_block(){
	return seq_next_block;
}

void Basic_Block::compute_kill_and_gen() {
	set<lli> &l_values=kill;
	l_values.insert(0);					//0 for constants, which are l_values only
	lli opd1,opd2;
	for(auto it:sa_block_icode_list){
		// cout<<"...hohoho....\n";	
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

bool Basic_Block::compute_in_and_out(){
	bool changed=false;
	cout<<"Block "<<block_id<<' ';
	set<lli> out_next;
	for(auto c_node:child_blocks){
		//union with in[c_node]
		set<lli>::iterator it2=out_next.begin();		
		for(auto v:c_node->get_in_set()){
			it2=out_next.insert(it2,v);
			print_by_id(v);
		}
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

Control_Flow_Graph::Control_Flow_Graph(list<Icode_Stmt*>&sa_icode_list){
	Basic_Block::block_counter=0;
	root=new Basic_Block;
	root->set_seq_next_block(NULL);
	Basic_Block* last_block=NULL, *current_block=root;
	current_block->set_block_id(Basic_Block::block_counter++);
	bool is_empty=false;
	for(auto it:sa_icode_list) {
		if(typeid(*it)==typeid(Control_Flow_IC_Stmt)) {
				current_block->push_back(it);
				int label_no = ((Control_Flow_IC_Stmt*)it)->get_Offset().back()-'0';
				if(label_block_table.find(label_no)==label_block_table.end()){
					Basic_Block* new_block=new Basic_Block;
					new_block->set_block_id(Basic_Block::block_counter++);
					new_block->set_seq_next_block(NULL);
					label_block_table.insert(pair<int,Basic_Block*>(label_no,new_block));
				}
				Basic_Block* new_block=new Basic_Block;					// create new block
				new_block->set_block_id(Basic_Block::block_counter++);
				new_block->set_seq_next_block(NULL);
				if(((Control_Flow_IC_Stmt*)it)->get_inst_op_of_ics().get_op()!=j)
					current_block->add_child(new_block);				// set next sequential block as child pointer for bne/beq(not j)
				current_block->add_child(label_block_table[label_no]);	// set label block as other child pointer 
				current_block->set_seq_next_block(new_block);
				last_block=current_block;								// current block becomes last block
				current_block=new_block;								// set current block as new block
				is_empty=true;											// current block is empty
		} else if(typeid(*it)==typeid(Label_IC_Stmt)) {
				Basic_Block* new_block;
				int label_no = atoi(((Label_IC_Stmt*)it)->get_offset().substr(5).c_str());		//label#
				if(label_block_table.find(label_no)==label_block_table.end()){
					new_block = new Basic_Block;											// make new block for label
					new_block->set_block_id(Basic_Block::block_counter++);
					new_block->set_seq_next_block(NULL);
					label_block_table.insert(pair<int,Basic_Block*>(label_no,new_block));
				} else {
					new_block = label_block_table[label_no];
				}

				new_block->push_back(it);
				if(is_empty){
					last_block->update_next_ic_block(new_block);  // update parents child to correct label block,
					last_block->set_seq_next_block(new_block);
					delete current_block;
					Basic_Block::block_counter--;	// last new block was deleted update the label_counter
				} else {
					current_block->add_child(new_block);
					current_block->set_seq_next_block(new_block);
				}
				// cout<<"next for label: "<<new_block->get_seq_next_block()<<endl;
				new_block->set_seq_next_block(NULL);
				last_block=NULL;
				is_empty=false;
				current_block = new_block;
		} else {
				current_block->push_back(it);
				last_block=NULL;
				is_empty=false;
		}
	}

	// print(false,false);
}

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

bool Control_Flow_Graph::eliminate_dead_code(){
	//do dfs over the cfg and compute kill gen sets for all blocks	
	Basic_Block* node=root;
	// cout<<"edc ...\n";
	// cout<<Basic_Block::block_counter<<endl;
	while(node!=NULL){
		// cout<<"starting...\n";
		node->compute_kill_and_gen();
		// cout<<"block "<<node->get_block_id()<<endl;
		node=node->get_seq_next_block();
		// cout<<"ending...\n";
	}
	
	// cout<<"ahead "<<endl;
	bool changed=false;
	vector<bool> seen(Basic_Block::block_counter,false);
	do {
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

void Control_Flow_Graph::make_icode_list(list<Icode_Stmt*>&sa_icode_list){
	sa_icode_list.clear();
	Basic_Block* node=root;
	while(node!=NULL){
		sa_icode_list.splice(sa_icode_list.end(),node->get_icode_list());
		node=node->get_seq_next_block();
	}
}
