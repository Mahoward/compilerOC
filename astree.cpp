
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

astree* new_astree (int symbol, int filenr, int linenr, int offset,
              const char* lexinfo) {
  astree* tree = new astree();
  tree->symbol = symbol;
  tree->filenr = filenr;
  tree->linenr = linenr;
  tree->offset = offset;
  tree->lexinfo = intern_stringset (lexinfo);
  DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
        tree, tree->filenr, tree->linenr, tree->offset,
        get_yytname (tree->symbol), tree->lexinfo->c_str());
  return tree;
}


astree* adopt1 (astree* root, astree* child) {
  root->children.push_back (child);
  DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
        root, root->lexinfo->c_str(),
        child, child->lexinfo->c_str());
  return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
  adopt1 (root, left);
  adopt1 (root, right);
  return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
  root = adopt1 (root, child);
  root->symbol = symbol;
  return root;
}

astree* create_funct_p(astree* val, astree* param, astree* block){
  astree *root;
  if(strcmp(";", const_cast<char*>(block->lexinfo->c_str())) == 0){
    root = new_astree(TOK_PROTOTYPE,
                       val->filenr, val->linenr, val->offset, "");
  }else{
    root = new_astree(TOK_FUNCTION,
                       val->filenr, val->linenr, val->offset, "");
  }

  adopt2(root, val, param);
  adopt1(root, block);
  return root;
}

astree* create_funct_e(astree* val, astree* block){
  astree *root = new_astree(TOK_FUNCTION,
                     val->filenr, val->linenr, val->offset, "");
  if(strcmp(";", const_cast<char*>(block->lexinfo->c_str())) == 0){

  }else{
    root = new_astree(TOK_FUNCTION,
                       val->filenr, val->linenr, val->offset, "");
  }

  return adopt2(root, val, block);
}

const char *tok_base_name(const char *tname){
  if (strstr (tname, "TOK_") == tname) tname += 4;
  return tname;
}

static void dump_node (FILE* outfile, astree* node) {
  fprintf (outfile, "%s \"%s\" %ld.%ld.%ld",
            tok_base_name(get_yytname (node->symbol)), node->lexinfo->c_str(),
            node->filenr, node->linenr, node->offset);
  /*bool need_space = false;
  for (size_t child = 0; child < node->children.size(); ++child) {
    if (need_space) fprintf (outfile, " ");
    need_space = true;
    fprintf (outfile, "%p", node->children.at(child));
  }
  fprintf (outfile, "]}");
*/
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
  if (root == NULL) return;
  //fprintf (outfile, "%*s%s ", depth * 3, "", root->lexinfo->c_str());
  for(int i = 0; i < depth; i++){
    fprintf(outfile, "|%*s", 3, "");
  }
  dump_node (outfile, root);
  fprintf (outfile, "\n");
  for (size_t child = 0; child < root->children.size(); ++child) {
    dump_astree_rec (outfile, root->children[child], depth + 1);
  }
}

void dump_astree (FILE* outfile, astree* root) {
  dump_astree_rec (outfile, root, 0);
  fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep){
  DEBUGF ('f', "toknum = %d, yyvaluep = %p\n", toknum, yyvaluep);
  if (is_defined_token (toknum)) {
    dump_node (outfile, yyvaluep);
  }else {
    fprintf (outfile, "%s(%d)\n", get_yytname (toknum), toknum);
  }
  fflush (NULL);
}


void free_ast (astree* root) {
  while (not root->children.empty()) {
    astree* child = root->children.back();
    root->children.pop_back();
    free_ast (child);
  }
  DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: \"%s\")\n",
        (uintptr_t) root, root->filenr, root->linenr, root->offset,
        get_yytname (root->symbol), root->lexinfo->c_str());
  delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
  free_ast (tree1);
  free_ast (tree2);
}
