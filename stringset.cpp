#include "stringset.h"

using stringset = unordered_set<string>;
using stringset_citor = stringset::const_iterator;

stringset set;

const string* intern_stringset (const char* string) {
   pair<stringset_citor,bool> handle = set.insert (string);
   return &*handle.first;
}

void dump_stringset (FILE* out) {
   size_t max_bucket_size = 0;
   for (size_t bucket = 0; bucket < set.bucket_count(); ++bucket) {
      bool need_index = true;
      size_t curr_size = set.bucket_size (bucket);
      if (max_bucket_size < curr_size) max_bucket_size = curr_size;
      for (auto itor = set.cbegin (bucket);
           itor != set.cend (bucket); ++itor) {
         if (need_index) fprintf (out, "stringset[%4lu]: ", bucket);
                    else fprintf (out, "          %4s   ", "");
         need_index = false;
         const string* str = &*itor;
         fprintf (out, "%22lu %p->\"%s\"\n", set.hash_function()(*str),
                  str, str->c_str());
      }
   }
   fprintf (out, "load_factor = %.3f\n", set.load_factor());
   fprintf (out, "bucket_count = %lu\n", set.bucket_count());
   fprintf (out, "max_bucket_size = %lu\n", max_bucket_size);
}
