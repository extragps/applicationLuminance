/***************************************************************************/
/**                                                                       **/
/*    DECLARATION DES DONNEES PUBLIQUES A TOUTES LES TACHES               **/
/**                                                                       **/
/***************************************************************************/
/************************************************/
/*  pour la trace                               */
/************************************************/
struct trace_wdg
{
  ULONG date;
  UINT type_restart;
  UINT ssr;
};
/*************************************************************************/
/*  pour la gestion de la simulation                                     */
/*************************************************************************/
struct vct_simulation
{
  UINT8 phase;
  UINT compteur;
  UINT compteur_debit;
  UINT relais;
};


/*************************************************************************/
/*  pour la gestion de la simulation                                     */
/*************************************************************************/
GLOBALREF struct vct_simulation vct_simulation;
