#include <stdio.h>
#include "kcommon.h"
#include "klog.h"
#include "event.h"


struct _Event
{
	Ktimestamp ts_t;	// Date de programmation de l'evenement
	EventType  type_e;
	void 		*data_p;	// Donnees associees a l'evenement
	void (*delete)(void *);
} ;

typedef struct _EventLibelle
{
	EventType type;
	char *libelle;
} EventLibelles;

static bool _eventPourri_b=false;


/* Pour instrumentalisation, on ajoute des libelles */

static EventLibelles _eventLibelles[]={
	{EVENT_AFFICHEUR,"AFFICHEUR"},
	{EVENT_ES,"ES"},
	{EVENT_THL,"THL"},
	{EVENT_DEMARRAGE,"DEMARRAGE"},
	{EVENT_ARRET,"ARRET"},
	{EVENT_AFFICHAGE,"AFFICHAGE"},
	{EVENT_ALLUMAGE,"ALLUMAGE"},
	{EVENT_ANIMATION,"ANIMATION"},
	{EVENT_TEST_PIXEL,"PIXEL"},
	{EVENT_STATUS,"STATUS"},
	{EVENT_TEST_ANIM,"ANIM"},
	{EVENT_REDEMARRAGE,"REDEMARRAGE"},
	{EVENT_SABORDAGE,"SABORDAGE"},
	{EVENT_TEST_ACTIVATION,"ACTIVATION"},
	{EVENT_REINIT     ,"REINIT"},
	{EVENT_MAX,"MAX"}
} ;

bool eventIsOk(Event *event_pt)
{
bool retour_b=false;
	if(NULL!=event_pt)
	{
	int indice=0;
	bool trouve_b=false;
		/* Controle du type */
		while((_eventLibelles[indice].type!=EVENT_MAX)&&(false==trouve_b))
		{
			if(eventGetType(event_pt)==_eventLibelles[indice].type)
			{
			Ktimestamp ts_t;
				/* Le type a ete trouve dans la liste des types */
				trouve_b=true;
				/* Si le type est correct, on regarde le timestamp
				 * en le comparant a la date courante. */
				ktsGetTime(&ts_t);
				switch(ktsGetDeltaMs(&ts_t,&event_pt->ts_t))
				{
				case INT32_MAX:
				case INT32_MIN:
					/* Il y a overflow, on sort en echec. */
					break;
				default:
					retour_b=true;
					break;
				}
			}
			indice++;
		}

	}
	return retour_b;
}

bool eventIsPourri(void)
{
	return _eventPourri_b;
}

void eventSetPourri(bool val_b)
{
	_eventPourri_b=val_b;
}

static char *_eventGetLibelle(EventType type)
{
int indice=0;
bool trouve_b=false;
	while((_eventLibelles[indice].type!=EVENT_MAX)&&(false==trouve_b))
	{
		if(_eventLibelles[indice].type==type)
		{
			trouve_b=true;
		}
		else
		{
			indice++;
		}
	}
	return _eventLibelles[indice].libelle;
}

void eventLog(Event *event_pt,KlogLevel level,char *chaine)
{
Ktimestamp *ts_pt=&event_pt->ts_t;
	klogPut(NULL,level,"Event %s : %s date %02ld/%02ld/%02ld %02ld:%02ld:%02ld.%03ld",
			chaine, _eventGetLibelle(event_pt->type_e),
			ts_pt->day_dw, ts_pt->month_dw, ts_pt->year_dw % 100,
			ts_pt->hour_dw, ts_pt->min_dw, ts_pt->sec_dw, ts_pt->ms_dw );
}

void eventPrint(Event *event_pt)
{
Ktimestamp *ts_pt=&event_pt->ts_t;
	printf("Event : %s date %02ld/%02ld/%02ld %02ld:%02ld:%02ld.%03ld\n",
			_eventGetLibelle(event_pt->type_e),
			ts_pt->day_dw, ts_pt->month_dw, ts_pt->year_dw % 100,
			ts_pt->hour_dw, ts_pt->min_dw, ts_pt->sec_dw, ts_pt->ms_dw );
}


Event *eventNew(Ktimestamp *date,EventType event_e,void *data_p,void (*delete)(void *))
{
	Event *event_pt=(Event *)kmmAlloc(NULL,sizeof(Event));
	if(NULL!=event_pt)
	{
		klogPut(NULL,LOG_INFO,"eventNew : allocation %#x",event_pt);
		event_pt->ts_t=*date;
		event_pt->type_e=event_e;
		event_pt->data_p=data_p;
		event_pt->delete=delete;
	}
	return event_pt;
}

int32 eventCompare(void *un_p,void *de_p)
{
	int32 retour=0;
	if(un_p!=de_p)
	{
		Event *un_pt=(Event *)un_p;
		Event *de_pt=(Event *)de_p;
		retour=ktsGetDeltaMs(&de_pt->ts_t,&un_pt->ts_t);
		switch(retour)
		{
		case INT32_MAX:
		case INT32_MIN:
			eventLog(de_pt,LOG_ERROR,"Ori ");
			eventLog(un_pt,LOG_ERROR,"New ");
			eventSetPourri(true);
			break;
		case 0:
			retour=1;
			break;
		default :
			break;
		}
	}
	return retour;
}

void eventDelete(void *event_p)
{
	if(NULL!=event_p)
	{
		Event *event_pt=(Event *)event_p;
		klogPut(NULL,LOG_INFO,"eventDelete : liberation %#x",event_pt);
		if(NULL!=event_pt->delete)
		{
			event_pt->delete(event_pt->data_p);
		}
		kmmFree(NULL, event_p);
	}
}

Ktimestamp *eventGetDate(Event *event_pt)
{
	return &event_pt->ts_t;
}

void eventSetDate(Event *event_pt,Ktimestamp *ts_pt)
{
	event_pt->ts_t=*ts_pt;
}
void eventSetDelta(Event *event_pt,long delta)
{
	Ktimestamp ts_t;
	ktsGetTime(&ts_t);
	kAddTime(&ts_t,0,delta);
	event_pt->ts_t=ts_t;
}

void eventAddDeltaSec(Event *event_pt,long delta)
{
	if(NULL!=event_pt)
	{
		kAddTime(&event_pt->ts_t,delta,0);
	}
}

void *eventGetData(Event *event_pt)
{
	return event_pt->data_p;
}

EventType eventGetType(Event *event_pt)
{
	return event_pt->type_e;
}
