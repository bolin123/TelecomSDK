#include "PPrivate.h"

_ptag PrivateCtx_t *PPrivateCreate(void)
{
    PrivateCtx_t *pri = (PrivateCtx_t *)malloc(sizeof(PrivateCtx_t));

    if(pri)
    {
        memset(pri, 0, sizeof(PrivateCtx_t));
        PListInit(&pri->properties);
        PListInit(&pri->resources);
        PListInit(&pri->subDevices);
        return pri;
    }
    return PNULL;
}

_ptag void PPrivateInitialize(void)
{
}

_ptag void PPrivatePoll(void)
{
}


