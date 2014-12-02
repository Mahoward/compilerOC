#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

//Added C libraries
#include <stdio.h>
#include <stdlib.h>

//Added C++ libraries
#include <stack>

//Added Project libraries
#include "astree.h"
#include "yyparse.h"


using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<string*,symbol*>;
using symbol_entry = pair<string*,symbol*>;

struct symbol {
   attr_bitset attributes;
   symbol_table* fields;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
};

symbol_table struct_table;
symbol_table global_table;
stack <symbol_table> sym_stack;
int depth = 0;


void populate_fields(astree* root, symbol_table* fields){
  for(vector<astree*>::iterator it = root->children.begin();
    it != root->children.end(); it++){}
}

string *populate_struct_sym(astree* root, symbol_table* struct_sym){
  string *key = NULL;
  for(vector<astree*>::iterator it = root->children.begin();
    it != root->children.end(); it++){
      if(*it->symbol == TOK_TYPEID){
        key = *it->lexinfo;
        struct_sym->filenr = *it->filenr;
        struct_sym->linenr = *it->linenr;
        struct_sym->offset = *it->offset;
        struct_sym->blocknr = 0;
        return key;
        break;
      }
    }
  return key;
}

void insert_struct(astree* root){
  symbol* struct_sym = new symbol();
  struct_sym->attributes.set(ATTR_struct);
  struct_sym->fields = new symbol_table();
  populate_fields(root, struct_sym->fields);
  string* key = populate_struct_sym(root, struct_sym);
  if(key == NULL){
    printf("KEY ERROR\n");
  }else{
    struct_table.insert({key, struct_sym});
  }
}

void visit(astree* root){
    switch(root->symbol){
      case TOK_STRUCT:
        insert_struct(root);
        break;
      default:
        insert_standard(root);
        for(vector<astree*>::iterator it = root->children.begin();
          it != root->children.end(); it++){
            visit(*it);
        }
    }

}
