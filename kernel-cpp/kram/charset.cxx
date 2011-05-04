#include "precomp.h"
//#define MODULE_NAME "charset"
//#define COPYRIGHT   "(c) SOS GmbH Berlin"
//#define AUTHOR      "J�rg Schwiemann, Joacim Zschimmer"

#include "sos.h"
#include "charset.h"

using namespace std;
namespace sos {


const Char_set_table_entry char_set_table [] =
{
    { "ISO 8859-1"   , "ascii",     "[[" "\\\\" "]]" "{{" "||" "}}" "~~" "@@" "$$" "``" "^^" },  
    { "belgisch"     , "belgian",   "[�" "\\�"  "]�" "{�" "|�" "}�"      "@�"                },
    { "d�nisch"      , "danish",    "[�" "\\�"  "]�" "{�" "|�" "}�" "~�"                "^�" },
    { "deutsch"      , "german",    "[�" "\\�"  "]�" "{�" "|�" "}�" "~�" "@�"                },
    { "franz�sisch"  , "french",    "[�" "\\�"  "]�" "{�" "|�" "}�" "~�" "@�"      "``"      },
    { "schwedisch"   , "swedish",   "[�" "\\�"  "]�" "{�" "|�" "}�" "~�" "@�"      "`�" "^�" }
};

const int char_set_table_count() 
{
  return NO_OF( char_set_table );
};

} //namespace sos
