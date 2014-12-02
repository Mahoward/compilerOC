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
#include "symtable.h"
#include "yyparse.h"

symbol_table struct_table;
symbol_table global_table;
stack <symbol_table> sym_stack;
int depth = 0;


void populate_fields(astree* root, symbol_table* fields){}

string *populate_struct_sym(astree* root, symbol* struct_sym){
  string *key = NULL;
  for(size_t i = 0; i < root->children.size(); i++){
      if(root->children[i]->symbol == TOK_TYPEID){
        key = (string *)root->children[i]->lexinfo;
        struct_sym->filenr = root->children[i]->filenr;
        struct_sym->linenr = root->children[i]->linenr;
        struct_sym->offset = root->children[i]->offset;
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
    printf("Key:%s\n", key->c_str());
    printf("struct_sym: %ld, %ld, %ld, %ld\n",
            struct_sym->filenr,struct_sym->linenr,
            struct_sym->offset,struct_sym->blocknr);
    struct_table.insert({key, struct_sym});
  }
}

void visit(astree* root){
    printf("%s", tok_base_name(get_yytname (root->symbol)));
    switch(root->symbol){
      case TOK_STRUCT:
        insert_struct(root);
        break;
      default:
        for(size_t i = 0; i < root->children.size(); i++){
            visit(root->children[i]);
        }
    }
}
