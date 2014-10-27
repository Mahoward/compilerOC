
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

#include "auxlib.h"
#include "stringset.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

int yy_flex_debug = 0;
int yydebug = 0;
string cpp_flag = "";

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}


void cpplines (FILE* yyin, char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, yyin);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      //printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, filename);
      if (sscanf_rc == 2) {
         //printf ("DIRECTIVE: line %d file \"%s\"\n", linenr, filename);
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
   //      printf ("token %d.%d: [%s]\n",
   //              linenr, tokenct, token);
         print_tok()
         intern_stringset(token);
      }
      ++linenr;
   }
}

//set_opts modeled by:
//http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
int set_opts(int argc, char **argv){
   int c;
   opterr = 0;
   while ((c = getopt (argc, argv, "ly@:D:")) != -1)
      switch (c){
         case 'l':
            yy_flex_debug = 1;
            break;
         case 'y':
            yydebug = 1;
            break;
      case 'D':
         cpp_flag.append("-D");
         cpp_flag.append(optarg);
         break;
      case '@':
         char debug_arg[LINESIZE];
         debug_arg[0] = '@';
         strncat(debug_arg, optarg, strlen(optarg));
         set_debugflags(debug_arg);
         break;
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
         abort ();
      }
   return optind;
}

int main (int argc, char** argv) {
   set_execname (argv[0]);
   int argi = set_opts(argc, argv);
   if(argi == -1)return 1;
   if(argi+1 < argc){
      printf("Input Error: Multiple Input Files Not Allowed\n");
      return 1;
   }
   char *filename = argv[argi];
   char *is_oc = strrchr(filename,'o');
   if(is_oc == NULL || strcmp(is_oc,"oc") != 0){
      printf("Input Error: .oc files only\n");
      return 1;
   }
   string command = CPP + " " + cpp_flag + " " + filename;
   yyin = popen (command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (command.c_str());
   }else {
      cpplines (yyin, filename);
      int pclose_rc = pclose (yyin);
      eprint_status (command.c_str(), pclose_rc);
      if (pclose_rc != 0) return EXIT_FAILURE;
   }
   int len = strlen(filename);
   char stringset_file[len];
   strncpy(stringset_file, strtok(filename, "."), len);
   strcat(stringset_file,".str");
   FILE *out = fopen(stringset_file, "w+");
   dump_stringset(out);
   return get_exitstatus();
}
