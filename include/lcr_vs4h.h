
#ifndef _lcr_vs4_h
#define _lcr_vs4_h

/* --------------------------------
 * E_vs4Cat
 * ========
 * Liste des categories.
 * --------------------------------	*/

typedef enum E_vs4Cat
{
  VS4_ALL,
  VS4_LC,
  VS4_VC,
  VS4_KC,
  VS4_PC,
  VS4_EC,
  VS4_TC,
  VS4_MAX
} E_vs4Cat;



GLOBALREF VOID vs4_fva (STRING, INT, INT, STRING, struct usr_ztf *);

#endif
