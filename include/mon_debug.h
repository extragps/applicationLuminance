#ifndef __INCmon_debugh
#define __INCmon_debugh

/* ********************************
 * DEFINITION DES CONSTANTES
 * ********************************	*/

typedef enum E_monDebugModule
{
  MON_DEBUG_ACK,
  MON_DEBUG_INT,
  MON_DEBUG_DET_PROT,
  MON_DEBUG_DET_DIAL,
  MON_DEBUG_PERF_LCR,
  MON_DEBUG_PERF_FIP,
  MON_DEBUG_PERF_SUP,
  MON_DEBUG_CAC,
  MON_DEBUG_EQU,
  MON_DEBUG_MAX_MODULE
} E_monDebugModule;

/* ********************************
 * PROTOTYPES DES FONCTIONS DU MODULE
 * ********************************	*/

void monDebugInit (void);
void monDebugSet (E_monDebugModule, int);
int monDebugGet (E_monDebugModule);
int monPrintDebug (const char *, ...);
int monPrintDebugBis (int,const char *, ...);

#endif
