/*********************************************************************
 *
 * Copyright:
 *	1998-1999 MOTOROLA, INC. All Rights Reserved. /SE The AMD29GL256M FLASH devices.  These devices are 1M x 16 devices.
 */
#ifndef _AMD_H
#define _AMD_H

/* Nouvelle structure ajoutee par XG pour realiser le claquage en flash en
 * copiant les routines en RAM. */
typedef struct AMD_FUNCT
{
  void (*start) (void);
  void (*read) (struct AMD_FUNCT *, void *);
  int (*sector_erase) (struct AMD_FUNCT *, void *, int);
  int (*id) (struct AMD_FUNCT *, void *);
  int (*write) (struct AMD_FUNCT *, void *, void *, unsigned int,
                int, void (*)(void));
  void *flashAdrs;
  long  sectorSize;
  BOOL verrou;
} AMD_FUNCT;

typedef struct AMD_INFO
{

	void *(*get_address)(void *,int);
	void *(*get_address_fin)(void *,int);
	long   (*get_size)(int);
  	void *flashAdrs;
  	long  sectorSize;
}	AMD_INFO;

/*****************************************************************/

#endif /* _AMD_H */
