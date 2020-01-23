
#include "fdm.h"
#include "fdmtime.h"
#include "usr_fip.h"


void fdm_generic_time_give_value(FDM_GENERIC_TIME_VALUE *heure)
{
	printf("Signifiance : %d\n",heure->Signifiance);
	printf("   Secondes : %d\n",heure->Number_of_second);
	printf("   Nanos    : %d\n",heure->Number_of_nanoseconde);
	
}

FDM_GENERIC_TIME_VALUE fdm_generic_time_get_value(void)
{
FDM_GENERIC_TIME_VALUE retour;
	return retour;
}

