#ifndef _EVENT_PMV_H_
#define _EVENT_PMV_H_

typedef enum _EventType
{
	EVENT_AFFICHEUR,
	EVENT_ES,
	EVENT_THL,
	EVENT_DEMARRAGE,
	EVENT_ARRET,
	EVENT_AFFICHAGE,
	EVENT_ALLUMAGE,
	EVENT_ANIMATION,
	EVENT_TEST_PIXEL,
	EVENT_STATUS,
	EVENT_TEST_ANIM,
	EVENT_REDEMARRAGE,
	EVENT_SABORDAGE,
	EVENT_TEST_ACTIVATION,
	EVENT_REINIT,
	EVENT_MAX
} EventType;

typedef struct _Event Event;


Event *eventNew(Ktimestamp *date,EventType type_e,void *data_p,void (*delete)(void *));
void eventDelete(void *);
Ktimestamp *eventGetDate(Event *);
void *eventGetData(Event *);
EventType eventGetType(Event *);
void eventSetDate(Event *,Ktimestamp *);
void eventSetDelta(Event *event_pt,long delta);
void eventAddDeltaSec(Event *event_pt,long delta);
int32 eventCompare(void *un_p,void *de_p);
void eventLog(Event *event_pt,KlogLevel level,char *chaine);
bool eventIsPourri(void);
void eventSetPourri(bool val_b);
bool eventIsOk(Event *event_pt);
void eventPrint(Event *event_pt);

#endif /*EVENT_H_*/
