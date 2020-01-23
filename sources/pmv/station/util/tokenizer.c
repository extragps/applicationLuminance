/**********************************************************************************************************************
 * fichier     : TokenizerDirif.c
 * description : Fonctions de decoupage de chaines de caracteres
 * prefixe     : tkz
 * --------------------------------------------------------------------------------------------------------------------
 * Auteur      : SG
 * Date        : 26/09/2006
 * OS          : Linux/ucLinux
 * --------------------------------------------------------------------------------------------------------------------
 * $Log: TokenizerDirif.c,v $
 * Revision 1.2  2018/06/04 08:38:44  xg
 * Passage en version V122
 *
 * Revision 1.1  2008/09/12 15:01:00  xag
 * Archivage de printemps
 *
 * Revision 1.1  2008/03/07 17:25:36  xag
 * *** empty log message ***
 *
 * Revision 1.13  2007/12/07 17:57:08  sgronchi
 * *** empty log message ***
 *
 * Revision 1.12  2007/12/05 19:25:12  sgronchi
 * ajout des fonctions tkzGetFirstInt() et tkzGetNextInt()
 *
 * Revision 1.11  2007/10/19 08:53:24  sgronchi
 * Acceptation des commandes commencant par une identification par parametre (ex. : ID=SIAT DT)
 *
 * Revision 1.10  2007/03/27 10:18:29  sgronchi
 * Ajout de  la fonction tkzGetSubstring() utilisee pour le relayage X
 *
 * Revision 1.9  2007/03/21 16:19:42  sgronchi
 * Suppression printf, ajout cartouche
 *
 *********************************************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kcommon.h>
#include <klog.h>
#include "tokenizerDirif.h"

/**------------------------------------------------------------------------------------------------
 * Decoupage d'un message en sous messages separes par le caractere donne
 * @param tkz_pt (S) : structure contenant les donnees decoupees
 * @param str_pc (E) : chaine a decouper (terminee par le caractere de fin de chaine \0)
 * @param separator_c (E) : caractere de decoupage
 * @return : true si ok, false si erreur
 */
bool tokenizeDirif( TokenizerDirif * tkz_pt, const char * str_pc, char separator_c )
{
	bool error_b = false;
	int32 index_dw = 0;
	int32 strLength_dw = strlen( str_pc ); /// Attention: chaine LCR uniquement?
	//int32 nbTokens_dw = 0;
	

	tkz_pt->nbTokens_dw = 0;
	tkz_pt->index_dw = -1;
	tkz_pt->str_pc = str_pc; /// Attention, pas de copie...

	index_dw = 0;
	tkz_pt->tokenStart_adw[tkz_pt->nbTokens_dw] = index_dw;
	
	while( !error_b && index_dw < strLength_dw && tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
		if( str_pc[index_dw] == separator_c ) {
			if( tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
				tkz_pt->tokenMasked_b[tkz_pt->nbTokens_dw] = false;
				tkz_pt->tokenStop_adw[tkz_pt->nbTokens_dw++] = index_dw;
				if( tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
					tkz_pt->tokenStart_adw[tkz_pt->nbTokens_dw] = index_dw + 1;
				}
			} else {
				error_b = true;
			}
		}
		index_dw++;
	}

	if( !error_b && tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
		tkz_pt->tokenMasked_b[tkz_pt->nbTokens_dw] = false;
		tkz_pt->tokenStop_adw[tkz_pt->nbTokens_dw++] = strLength_dw;
	} else {
		error_b = true;
	}
	
	return (error_b == false);
}

static bool _tokenizeIsSeparator(const char info,char *separators_pt)
{
bool retour_b=false;
int longueur=strlen(separators_pt);
int indice=0;
	for(indice=0;indice<longueur;indice++)
	{
		if( info == separators_pt[indice] ) {
			retour_b=true;
			break;
		}
	}
	return retour_b;
}	

bool tokenizeExtended( TokenizerDirif * tkz_pt, const char * str_pc, char *separators_pt )
{
	bool error_b = false;
	int32 index_dw = 0;
	int32 strLength_dw = strlen( str_pc ); /// Attention: chaine LCR uniquement?
	//int32 nbTokens_dw = 0;
	
	tkz_pt->nbTokens_dw = 0;
	tkz_pt->index_dw = -1;
	tkz_pt->str_pc = str_pc; /// Attention, pas de copie...

	index_dw = 0;
	tkz_pt->tokenStart_adw[tkz_pt->nbTokens_dw] = index_dw;
	
	while( !error_b && index_dw < strLength_dw && tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
		if( _tokenizeIsSeparator(str_pc[index_dw],separators_pt) ) {
			if( tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
				tkz_pt->tokenMasked_b[tkz_pt->nbTokens_dw] = false;
				tkz_pt->tokenStop_adw[tkz_pt->nbTokens_dw++] = index_dw;
				if( tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
					tkz_pt->tokenStart_adw[tkz_pt->nbTokens_dw] = index_dw + 1;
				}
			} else {
				error_b = true;
			}
		}
		index_dw++;
	}

	if( !error_b && tkz_pt->nbTokens_dw < TKZ_MAXNBTOKENS ) {
		tkz_pt->tokenMasked_b[tkz_pt->nbTokens_dw] = false;
		tkz_pt->tokenStop_adw[tkz_pt->nbTokens_dw++] = strLength_dw;
	} else {
		error_b = true;
	}
	
	return (error_b == false);
}

/**------------------------------------------------------------------------------------------------
 * Remise a zero de l'index de lecture des tokens
 * @param tkz_pt (E/S) : TokenizerDirif
 */
void tkzDirifReinit( TokenizerDirif * tkz_pt )
{
	tkz_pt->index_dw = -1;
}

/**------------------------------------------------------------------------------------------------
 * Recuperation de l'index valide (non masque) suivant
 */
static int32 _tkzNextIndex( TokenizerDirif * tkz_pt )
{
	int32 index_dw = tkz_pt->index_dw + 1;
	
	while( index_dw < tkz_pt->nbTokens_dw && tkz_pt->tokenMasked_b[index_dw] ) {
		index_dw++;
	}
	
	return index_dw;
}

/**------------------------------------------------------------------------------------------------
 * 
 */
static int32 _tkzPreviousIndex( TokenizerDirif * tkz_pt )
{
	int32 index_dw = tkz_pt->index_dw - 1;
	
	while( index_dw >= 0 && tkz_pt->tokenMasked_b[index_dw] ) {
		index_dw--;
	}
	
	return index_dw;
}

/**------------------------------------------------------------------------------------------------
 * Recuperation du token courant (fonction privee)
 */
static bool _tkzGet( TokenizerDirif * tkz_pt, char * token_pc, int32 sizeMax_dw )
{
	bool success_b = false;
	int32 start_dw;
	int32 end_dw;
	
	if( tkz_pt->index_dw < tkz_pt->nbTokens_dw ) {
		if( token_pc != null && sizeMax_dw > 0 ) {
			start_dw = tkz_pt->tokenStart_adw[tkz_pt->index_dw];
			end_dw = tkz_pt->tokenStop_adw[tkz_pt->index_dw];
			if( start_dw == end_dw ) {
				token_pc[0] = 0;
				success_b = true;
			} else if( end_dw - start_dw < sizeMax_dw ) {
				strncpy( token_pc, tkz_pt->str_pc + start_dw, end_dw - start_dw );
				token_pc[end_dw - start_dw] = 0;
				success_b = true;
			}
		} else if( token_pc == null ) {
			success_b = true;
		}
	}

	return success_b;
}

/**------------------------------------------------------------------------------------------------
 * Verification de l'existence du token suivant
 * @param tkz_pt (E) : TokenizerDirif
 */
bool tkzDirifHasNext( TokenizerDirif * tkz_pt )
{
	return ( _tkzNextIndex(tkz_pt) < tkz_pt->nbTokens_dw );
}

/**------------------------------------------------------------------------------------------------
 * Recuperation du premier token
 * @param tkz_pt (E/S): TokenizerDirif
 * @param token_pc (S) : chaine dans laquelle sera stocke le token
 * @param size_dw (E) : taille de la chaine token_pc
 * @return flag de succes de l'operation
 */
bool tkzDirifGetFirst( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw )
{
	bool success_b = false;
	
	tkz_pt->index_dw = 0;
	while( tkz_pt->index_dw < tkz_pt->nbTokens_dw && tkz_pt->tokenMasked_b[tkz_pt->index_dw] ) {
		tkz_pt->index_dw++;
	}

	success_b = _tkzGet( tkz_pt, token_pc, size_dw );
	
	return success_b;
}

/**------------------------------------------------------------------------------------------------
 * Recuperation du token courant (repetition)
 * @param tkz_pt (E) : TokenizerDirif
 * @param token_pc (S) : chaine de stockage du token
 * @param size_dw (E) : taille max de la chaine de stockage
 * @return : true si ok, false si erreur
 */
bool tkzDirifGetCurrent( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw )
{
	bool success_b = false;
	
	success_b = _tkzGet( tkz_pt, token_pc, size_dw );
	
	return success_b;
}

/**--------------------------------------------------------------------------------------------------------------------
 * Recuperation de la sous-chaine suivant la position courante
 * @param tkz_pt (E) : TokenizerDirif
 * @return : sous-chaine
 */
char * tkzDirifGetSubstring( TokenizerDirif * tkz_pt )
{
	const char * substring_pc;
	int32 index_dw = tkz_pt->index_dw + 1;
	int32 size_dw;
	
	if( index_dw >= 0 && index_dw < tkz_pt->nbTokens_dw ) {
		substring_pc = &tkz_pt->str_pc[tkz_pt->tokenStart_adw[index_dw]];
	} else {
		size_dw = strlen(tkz_pt->str_pc);
		substring_pc = &tkz_pt->str_pc[size_dw-1];
	}
	
	return (char*) substring_pc;
}

/**------------------------------------------------------------------------------------------------
 * Recuperation du token suivant
 * @param tkz_pt (E/S): TokenizerDirif
 * @param token_pc (S) : chaine dans laquelle sera stocke le token
 * @param size_dw (E) : taille de la chaine token_pc
 * @return flag de succes de l'operation
 */
bool tkzDirifGetNext( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw )
{
	bool success_b = false;
	
	tkz_pt->index_dw = _tkzNextIndex( tkz_pt );
	success_b = _tkzGet( tkz_pt, token_pc, size_dw );
	
	return success_b;
}

/**------------------------------------------------------------------------------------------------
 * Recuperation du token precedent
 * @param tkz_pt (E/S): TokenizerDirif
 * @param token_pc (S) : chaine dans laquelle sera stocke le token (ou null)
 * @param size_dw (E) : taille de la chaine token_pc
 * @return flag de succes de l'operation
 */
bool tkzDirifGetPrevious( TokenizerDirif * tkz_pt, char * token_pc, int32 size_dw )
{
	bool success_b = false;
	tkz_pt->index_dw = _tkzPreviousIndex( tkz_pt );
	success_b = _tkzGet( tkz_pt, token_pc, size_dw );
	
	return success_b;
}

/**------------------------------------------------------------------------------------------------
 * Recuperation du nombre de tokens valides (non masques)
 * @param tkz_pt (E) : TokenizerDirif
 * @return : nombre de tokens
 */
int32 tkzDirifGetSize( TokenizerDirif * tkz_pt )
{
	int32 nbTokens_dw = tkz_pt->nbTokens_dw;
	int32 k_dw;
	
	for( k_dw = 0; k_dw < tkz_pt->nbTokens_dw; k_dw++ ) {
		if( tkz_pt->tokenMasked_b[k_dw] ) {
			nbTokens_dw--;
		}
	}
	
	return nbTokens_dw;
}

/**------------------------------------------------------------------------------------------------
 * Masquage d'un token. Une fois masque, un token ne sera plus accessible lors d'un parcours avec
 * tkzGetFirst(), tkzGetNext() et tkzGetCurrent()
 * @param tkz_pt (E/S) : TokenizerDirif
 * @return : true si ok, false si erreur (index courant incorrect)
 */
bool tkzDirifMaskCurrent( TokenizerDirif * tkz_pt )
{
	bool success_b = false;
	
	if( tkz_pt->index_dw < tkz_pt->nbTokens_dw ) {
		tkz_pt->tokenMasked_b[tkz_pt->index_dw] = true;
		success_b = true;
	}
	
	return success_b;
}

/**------------------------------------------------------------------------------------------------
 * Reinitialisation des masques de tokens
 * @param tkz_pt (E/S) : TokenizerDirif
 */ 
void tkzDirifMaskClear( TokenizerDirif * tkz_pt )
{
	int32 k_dw;
	
	for( k_dw = 0; k_dw < tkz_pt->nbTokens_dw; k_dw++ ) {
		tkz_pt->tokenMasked_b[k_dw] = false;
	}
}

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
bool tkzDirifGetFirstInt( TokenizerDirif * tkz_pt, int32 * value_pdw, int32 min_dw, int32 max_dw )
{
	char str_ac[12];
	
	return ( tkzDirifGetFirst( tkz_pt, str_ac, 12 ) && str2intBorned( value_pdw, str_ac, min_dw, max_dw ) );
}

/**--------------------------------------------------------------------------------------------------------------------
 * 
 */
bool tkzDirifGetNextInt( TokenizerDirif * tkz_pt, int32 * value_pdw, int32 min_dw, int32 max_dw )
{
	char str_ac[12];
	
	return ( tkzDirifGetNext( tkz_pt, str_ac, 12 ) && str2intBorned( value_pdw, str_ac, min_dw, max_dw ) );
}
