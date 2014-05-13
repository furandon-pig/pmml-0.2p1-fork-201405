/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -D -p -t -G  */
#include "keyword.h"
struct keyword { char *name;  int  val; };

#define TOTAL_KEYWORDS 94
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 15
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 233
/* maximum key range = 233, duplicates = 3 */

static unsigned int
hash (str, len)
     register char *str;
     register int unsigned len;
{
  static unsigned char asso_values[] =
    {
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
      10, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
     234, 234, 234, 234, 234, 234, 234,  70,  40,  30,
     112,  95,  35,  50,  85, 105, 234,  75,  25,   0,
      75, 110,  95, 234,  30,   0,   0,  25,  35,  60,
       5,   5, 234, 234, 234, 234, 234, 234,
    };
  return len + asso_values[str[len - 1]] + asso_values[str[0]];
}

struct keyword wordlist[] =
{
      {"",}, 
      {"t",  T_T},
      {"text",  T_TEXT},
      {"shift",  T_SHIFT},
      {"set_eff_chs",  T_SET_EFF_CHS},
      {"set_thru_chs",  T_SET_THRU_CHS},
      {"set_eff_etypes",  T_SET_EFF_ETYPES},
      {"set_thru_etypes",  T_SET_THRU_ETYPES},
      {"shl",  T_SHL},
      {"signal",  T_SIGNAL},
      {"rt",  T_RT},
      {"shr",  T_SHR},
      {"reject",  T_REJECT},
      {"repeat",  T_REPEAT},
      {"ctrl_pt",  T_CTRL_PT},
      {"xor",  T_XOR},
      {"tb",  T_TB},
      {"ctrl_any",  T_CTRL_ANY},
      {"l",  T_L},
      {"gt",  T_DU},
      {"local",  T_LOCAL},
      {"timesig",  T_TIMESIG},
      {"ctrl",  T_CTRL},
      {"cpr",  T_CPR},
      {"wait",  T_WAIT},
      {"undef",  T_UNDEF},
      {"for",  T_FOR},
      {"v",  T_V},
      {"alt",  T_ALT},
      {"meta",  T_META},
      {"arbit",  T_ARBIT},
      {"tk",  T_TK},
      {"add_eff_chs",  T_ADD_EFF_CHS},
      {"gr",  T_DR},
      {"key",  T_KEY},
      {"add_eff_etypes",  T_ADD_EFF_ETYPES},
      {"sh",  T_SH},
      {"switch",  T_SWITCH},
      {"tp",  T_TP},
      {"all",  T_ALL},
      {"smpte",  T_SMPTE},
      {"ac",  T_AC},
      {"null",  T_NULL},
      {"loadtrk",  T_LOADTRK},
      {"init",  T_INIT},
      {"excl2",  T_EXCL2},
      {"insert",  T_INSERT},
      {"nv",  T_NV},
      {"dt",  T_DT},
      {"seqno",  T_SEQNO},
      {"tempo",  T_TEMPO},
      {"ch",  T_CH},
      {"note_off",  T_NOTE_OFF},
      {"default",  T_DEFAULT},
      {"break",  T_BREAK},
      {"del_eff_chs",  T_DEL_EFF_CHS},
      {"eval",  T_EVAL},
      {"excl",  T_EXCL},
      {"del_eff_etypes",  T_DEL_EFF_ETYPES},
      {"foreach",  T_FOREACH},
      {"case",  T_CASE},
      {"close",  T_CLOSE},
      {"keysig",  T_KEYSIG},
      {"evalstr",  T_EVALSTR},
      {"edef",  T_EDEF},
      {"elsif",  T_ELSIF},
      {"du",  T_DU},
      {"load",  T_LOAD},
      {"if",  T_IF},
      {"dr",  T_DR},
      {"ctrl_to",  T_CTRL_TO},
      {"ctrl_cto",  T_CTRL_CTO},
      {"prog",  T_PROG},
      {"def",  T_DEF},
      {"n",  T_N},
      {"defeff",  T_DEFEFF},
      {"bend",  T_BEND},
      {"note_on",  T_NOTE_ON},
      {"wrap",  T_WRAP},
      {"while",  T_WHILE},
      {"attach",  T_ATTACH},
      {"kp",  T_KP},
      {"note",  T_NOTE},
      {"append",  T_APPEND},
      {"else",  T_ELSE},
      {"ecode",  T_ECODE},
      {"enable",  T_ENABLE},
      {"detach",  T_DETACH},
      {"include",  T_INCLUDE},
      {"dp",  T_DP},
      {"end",  T_END},
      {"disable",  T_DISABLE},
      {"o",  T_O},
      {"do",  T_DO},
      {"defthread",  T_DEFTHREAD},
};

static short lookup[] =
{
        -1,   1,  -1,  -1,   2,   3,  -1,  -1,  -1,  -1,  -1,   4,   5,  -1,
         6,   7,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
         8,  -1,  -1,   9,  10,  11,  -1,  -1, 236,  14,  15, -12,  -2,  -1,
        16,  17,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  18,  19,  -1,  -1,  20,
        -1,  21,  -1,  22,  -1,  -1,  -1,  23,  24,  25,  -1,  -1,  26,  -1,
        -1,  27,  -1,  28,  29,  30,  -1,  31,  -1,  -1,  -1,  32,  33,  34,
        35,  -1,  -1,  36,  -1,  -1,  -1,  37,  -1,  -1,  -1,  -1,  -1,  38,
        39,  -1,  40,  -1,  41,  -1,  42,  -1,  -1,  43,  -1,  44,  45,  46,
        47,  -1,  48, 239,  -1,  51,  52,  53,  54, -49,  -2,  55, 245,  -1,
        58,  59,  -1,  60,  61,  62,  63,  -1,  64,  65, -56,  -2,  -1,  66,
        -1,  67,  68,  -1,  69,  -1,  -1,  70,  71,  72,  73,  74,  -1,  75,
        -1,  -1,  76,  77,  -1,  78,  79,  80,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  81,  -1,  82,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  83,  -1,  -1,  -1,  -1,  -1,  84,  85,
        86,  -1,  -1,  -1,  -1,  -1,  -1,  87,  -1,  -1,  -1,  88,  -1,  89,
        90,  -1,  -1,  -1,  91,  -1,  -1,  -1,  -1,  -1,  -1,  92,  -1,  -1,
        93,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  94,
};

struct keyword *
in_word_set (str, len)
     register char *str;
     register unsigned int len;
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0 && index < MAX_HASH_VALUE)
            {
              register char *s = wordlist[index].name;

              if (*s == *str && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
          else if (index < 0 && index >= -MAX_HASH_VALUE)
            return 0;
          else
            {
              register int offset = key + index + (index > 0 ? -MAX_HASH_VALUE : MAX_HASH_VALUE);
              register struct keyword *base = &wordlist[-lookup[offset]];
              register struct keyword *ptr = base + -lookup[offset + 1];

              while (--ptr >= base)
                if (*str == *ptr->name && !strcmp (str + 1, ptr->name + 1))
                  return ptr;
            }
        }
    }
  return 0;
}
