/**********************************************************************************************************************
 * fichier     : tokenizer.h
 * description : Fonctions de decoupage de chaines de caracteres
 * prefixe     : tkz
 * --------------------------------------------------------------------------------------------------------------------
 * Auteur      : SG
 * Date        : 26/09/2006
 * OS          : Linux/ucLinux
 * --------------------------------------------------------------------------------------------------------------------
 * $Log: tokenizer.h,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/09/12 14:56:42  xag
 * Archivage de printemps
 *
 * Revision 1.1  2008/03/07 17:25:35  xag
 * *** empty log message ***
 *
 * Revision 1.9  2007/12/05 19:25:12  sgronchi
 * ajout des fonctions tkzDirifGetFirst() et tkzDirifGetNextInt()
 *
 * Revision 1.8  2007/03/27 10:18:29  sgronchi
 * Ajout de  la fonction tkzDirifGetSubstring() utilisee pour le relayage X
 *
 * Revision 1.7  2007/03/21 16:19:43  sgronchi
 * Suppression printf, ajout cartouche
 *
 *********************************************************************************************************************/
#ifndef TOKENIZER_DIRIF_H_
#define TOKENIZER_DIRIF_H_

#define TKZ2_MAXNBTOKENS 50

/**------------------------------------------------------------------------------------------------
 * Structure tokenizer
 */
typedef struct _TokenizerDirif {
	const char * str_pc;
	int32 tokenStart_adw[TKZ2_MAXNBTOKENS];
	int32 tokenStop_adw[TKZ2_MAXNBTOKENS];
	bool tokenMasked_b[TKZ2_MAXNBTOKENS];
	int32 nbTokens_dw;
	int32 index_dw;
} TokenizerDirif;

/**------------------------------------------------------------------------------------------------
 * Decoupage d'un message en sous messages separes par le caractere donne
 * @param tkz_pt (S) : structure contenant les donnees decoupees
 * @param str_pc (E) : chaine a decouper (terminee par le caractere de fin de chaine \0)
 * @param separator_c (E) : caractere de decoupage
 * @return : true si ok, false si erreur
 */
bool tokenizeDirif( TokenizerDirif * tkz_pt, const char * str_pc, char separator_c );
bool tokenizeExtended( TokenizerDirif * tkz_pt, const char * str_pc, char *separators_pt );

/**------------------------------------------------------------------------------------------------
 * Recuperation du nombre de tokens valides (non masques)
 * @param tkz_pt (E) : tokenizer
 * @return : nombre de tokens
 */
int32 tkzDirifGetSize( TokenizerDirif * tkz_pt );

/**------------------------------------------------------------------------------------------------
 * Remise a zero de l'index de lecture des tokens
 * @param tkz_pt (E/S) : tokenizer
 */
void tkzDirifRenit( TokenizerDirif * tkz_pt );

/**------------------------------------------------------------------------------------------------
 * Verification de l'existence du token suivant
 * @param tkz_pt (E) : tokenizer
 */
bool tkzDirifHasNext( TokenizerDirif * tkz_pt );

/**------------------------------------------------------------------------------------------------
 * Recuperation du premier token
 * @param tkz_pt (E/S): tokenizer
 * @param token_pc (S) : chaine dans laquelle sera stocke le token
 * @param size_dw (E) : taille de la chaine token_pc
 * @return flag de succes de l'operation
 */
bool tkzDirifGetFirst( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw );

/**------------------------------------------------------------------------------------------------
 * Recuperation du token courant (repetition)
 * @param tkz_pt (E) : tokenizer
 * @param token_pc (S) : chaine de stockage du token
 * @param size_dw (E) : taille max de la chaine de stockage
 * @return : true si ok, false si erreur
 */
bool tkzDirifGetCurrent( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw );

/**--------------------------------------------------------------------------------------------------------------------
 * Recuperation de la sous-chaine suivant la position courante
 * @param tkz_pt (E) : tokenizer
 * @return : sous-chaine
 */
char * tkzDirifGetSubstring( TokenizerDirif * tkz_pt );

/**------------------------------------------------------------------------------------------------
 * Recuperation du token suivant
 * @param tkz_pt (E/S): tokenizer
 * @param token_pc (S) : chaine dans laquelle sera stocke le token
 * @param size_dw (E) : taille de la chaine token_pc
 * @return flag de succes de l'operation
 */
bool tkzDirifGetNext( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw );

/**------------------------------------------------------------------------------------------------
 * Recuperation du token precedent
 * @param tkz_pt (E/S): tokenizer
 * @param token_pc (S) : chaine dans laquelle sera stocke le token (ou null)
 * @param size_dw (E) : taille de la chaine token_pc
 * @return flag de succes de l'operation
 */
bool tkzDirifGetPrevious( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw );

/**------------------------------------------------------------------------------------------------
 * Masquage d'un token. Une fois masque, un token ne sera plus accessible lors d'un parcours avec
 * tkzDirifGetFirst(), tkzDirifGetNext() et tkzDirifGetCurrent()
 * @param tkz_pt (E/S) : tokenizer
 * @return : true si ok, false si erreur (index courant incorrect)
 */
bool tkzDirifMaskCurrent( TokenizerDirif * tkz_pt );

/**------------------------------------------------------------------------------------------------
 * Reinitialisation des masques de tokens
 * @param tkz_pt (E/S) : tokenizer
 */ 
void tkzDirifMaskClear( TokenizerDirif * tkz_pt );

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
bool tkzDirifGetFirstInt( TokenizerDirif * tkz_pt, int32 * value_pdw, int32 min_dw, int32 max_dw );

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
bool tkzDirifGetNextInt( TokenizerDirif * tkz_pt, int32 * value_pdw, int32 min_dw, int32 max_dw );

#endif /*TOKENIZER_H_*/
