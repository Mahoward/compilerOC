
#include <string>
using namespace std;

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "stringset.h"
#include "yyparse.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

string yyin_cpp_command;
string cpp_flag = "";

void yyin_cpp_popen(const char* filename){
   yyin_cpp_command = CPP;
   yyin_cpp_command += " ";
   yyin_cpp_command += cpp_flag;
   yyin_cpp_command += " ";
   yyin_cpp_command += filename;
   yyin = popen (yyin_cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (yyin_cpp_command.c_str());
      exit (get_exitstatus());
   }
}

void get_tok(){
   for (;;) {
      int token = yylex();
      //if (yy_flex_debug) fflush (NULL);
      switch (token) {
         case YYEOF:
            printf ("END OF FILE\n");
            return;
         case TOK_VOID:

         case TOK_BOOL:

         case TOK_CHAR:

         case TOK_INT:

         case TOK_STRING:

         case TOK_IF:

         case TOK_ELSE:

         case TOK_WHILE:

         case TOK_RETURN:

         case TOK_STRUCT:

         case TOK_FALSE:

         case TOK_TRUE:

         case TOK_NULL:

         case TOK_NEW:

         case TOK_ARRAY:

         case TOK_EQ:

         case TOK_NE:

         case TOK_LT:

         case TOK_LE:

         case TOK_GT:

         case TOK_GE:

         case TOK_IDENT:

         case TOK_INTCON:

         case TOK_CHARCON:

         case TOK_STRINGCON:

         case TOK_ORD:

         case TOK_CHR:

         case TOK_INTCON:
            printf ("\"%s\"\n", yytext);
            break;
         default:
            printf ("");
      }
   }
}


void yyin_cpp_pclose(void){
   int pclose_rc = pclose (yyin);
   eprint_status (yyin_cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

void print_str(char* filename){
   int len = strlen(filename);
   char stringset_file[len];
   strncpy(stringset_file, strtok(filename, "."), len);
   strcat(stringset_file,".str");
   FILE *out = fopen(stringset_file, "w+");
   dump_stringset(out);
}

//set_opts modeled by:
//http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
int set_opts(int argc, char **argv){
   int c;
   opterr = 0;
   while ((c = getopt (argc, argv, "ly@:D:")) != -1)
      switch (c){
         case 'l': yy_flex_debug = 1;
         case 'y': yydebug = 1;
      case 'D':
         cpp_flag.append("-D");
         cpp_flag.append(optarg);

      case '@':
         char debug_arg[LINESIZE];
         debug_arg[0] = '@';
         strncat(debug_arg, optarg, strlen(optarg));
         set_debugflags(debug_arg);

      case '?':
         if (optopt == 'D' || optopt == '@')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
         else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
            fprintf (stderr,
                  "Unknown option character `\\x%x'.\n",
                  optopt);
         return -1;
      default:
         return -1;
      }
   return optind;
}

int main (int argc, char** argv) {
   yydebug = 0;
   yy_flex_debug = 0;
   set_execname (argv[0]);
   int argi = set_opts(argc, argv);
   if(argi == -1)return 1;
   if(argi+1 < argc){
      printf("Input Error: Multiple Input Files Not Allowed\n");
      return 1;
   }else if(argi == argc){
      printf("Input Error: No File Specified\n");
      return 1;
   }
   char *filename = argv[argi];
   char *is_oc = strrchr(filename,'o');
   if(is_oc == NULL || strcmp(is_oc,"oc") != 0){
      printf("Input Error: .oc files only\n");
      return 1;
   }

   yyin_cpp_popen(filename);

   get_tok();

   yyin_cpp_pclose();

   //print_str(filename);

   return get_exitstatus();
}
