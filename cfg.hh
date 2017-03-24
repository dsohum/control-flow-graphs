#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std;

#include "icode.hh"

typedef long long int lli;

class Basic_Block {
	int block_id;
	list<Icode_Stmt*> sa_block_icode_list;
	list<Basic_Block*> child_blocks;
  Basic_Block* seq_next_block;
	set<lli> gen,kill,in,out;
  public:
	static int block_counter;
  	Basic_Block(){}
  	void push_back(Icode_Stmt*);
  	int get_block_id();
  	void set_block_id(int id);
    void set_seq_next_block(Basic_Block*);
    Basic_Block* get_seq_next_block();
  	void add_child(Basic_Block*);
  	void update_next_ic_block(Basic_Block*);
  	bool compute_in_and_out();
  	void compute_kill_and_gen();
    bool eliminate_dead_code();
  	list<Basic_Block*>& get_child_blocks();
    list<Icode_Stmt*>& get_icode_list();
    set<lli>& get_kill_set();
    set<lli>& get_gen_set();
    set<lli>& get_in_set();
    set<lli>& get_out_set();
};

class Control_Flow_Graph {
	Basic_Block* root;
	map<int,Basic_Block*> label_block_table;
  public:
  	Control_Flow_Graph(list<Icode_Stmt*>&);
  	bool eliminate_dead_code();
    void print(bool,bool);
    void make_icode_list(list<Icode_Stmt*>&);
};