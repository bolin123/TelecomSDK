#include "PPrivate.h"

_ptag void PPrivateEventEmit(PrivateCtx_t *ctx, PlatformEvent_t event, void *args)
{
    if(ctx->eventHandle)
    {
        ctx->eventHandle(event, args);
    }
}

_ptag PMSubDevice_t *PPrivateGetSubDevice(PrivateCtx_t *ctx, const char *subDid)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(&ctx->subDevices, subDev)
    {
        if(strcmp(subDev->did, subDid) == 0)
        {
            return subDev;
        }
    }
    return PNULL;
}

_ptag int PPrivateSubDeviceHeartbeat(PrivateCtx_t *ctx, const char *did)
{
    PMSubDevice_t *subDev = PPrivateGetSubDevice(ctx, did);
    if(subDev)
    {
        subDev->hbTime = PUtcTime();
        return 0;
    }

    return -1;
}

_ptag int PPrivateSubDeviceRSSIValue(PrivateCtx_t *ctx, const char *did, int rssi)
{
    PMSubDevice_t *subDev = PPrivateGetSubDevice(ctx, did);
    if(subDev)
    {
        subDev->rssi = rssi;
        return 0;
    }

    return -1;
}

_ptag int PPrivateSubDeviceBatteryRemain(PrivateCtx_t *ctx, const char *did, int remain)
{
    PMSubDevice_t *subDev = PPrivateGetSubDevice(ctx, did);
    if(subDev)
    {
        subDev->battery = remain;
        return 0;
    }

    return -1;
}

_ptag int PPrivateSubDeviceDel(PrivateCtx_t *ctx, const char *did)
{
//    puint16_t i;
    PMSubDevice_t *subDev = PNULL;
    ModulesVersion_t *module;
    PMProperty_t *property;
    ResourceInfo_t *resource;
    ResourceItems_t *item;
    ResourceNode_t *node;

    PListForeach(&ctx->subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            PListDel(subDev);
            PListForeach(&subDev->modules, module)
            {
                PListDel(module);
                if(module->name)
                {
                    free(module->name);
                }
                free(module);
            }

            PListForeach(&subDev->property, property)
            {
                PListDel(property);
                if(property->name)
                {
                    free(property->name);
                }
                if(property->type == PROPERTY_TYPE_TEXT && property->value.text)
                {
                    free(property->value.text);
                }
                free(property);
            }

            PListForeach(&subDev->resource, resource)
            {
                PListDel(resource);
                if(resource->rscName)
                {
                    free(resource->rscName);
                }
                if(resource->idName)
                {
                    free(resource->idName);
                }

                PListForeach(&resource->items, item)
                {
                    PListDel(item);
                    PListForeach(&item->node, node)
                    {
                        PListDel(node);
                        if(node->type == PROPERTY_TYPE_TEXT)
                        {
                            if(node->value.text)
                            {
                                free(node->value.text);
                            }
                        }
                        free(node->name);
                        free(node);
                    }
                    free(item);
                }
                free(resource);
            }

            free(subDev);
            return 0;
        }
    }
    return -1;
}

_ptag int PPrivateSubDeviceRegister(PrivateCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version, const char *factoryName)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(&ctx->subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            plog("%s already register!", did);
            return 0;
        }
    }

    subDev = (PMSubDevice_t *)malloc(sizeof(PMSubDevice_t));
    if(subDev)
    {
        memset(subDev, 0, sizeof(PMSubDevice_t));
        subDev->online = pfalse;
        subDev->needPost = pfalse;
        subDev->authStatus = SUBDEV_AUTH_NONE;
        strcpy(subDev->did, did);
        strcpy(subDev->pin, pin);
        strcpy(subDev->model, model);
        strcpy(subDev->version, version);
        if(factoryName)
        {
            strcpy(subDev->factoryName, factoryName);
        }

        PListInit(&subDev->modules);
        PListInit(&subDev->property);
        PListInit(&subDev->resource);
        PListAdd(&ctx->subDevices, subDev);
        return 0;
    }
    return -1;
}

_ptag PrivateCtx_t *PPrivateCreate(void)
{
    PrivateCtx_t *pri = (PrivateCtx_t *)malloc(sizeof(PrivateCtx_t));

    if(pri)
    {
        memset(pri, 0, sizeof(PrivateCtx_t));
        PListInit(&pri->modules);
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


