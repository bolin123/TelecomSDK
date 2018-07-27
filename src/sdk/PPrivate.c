#include "PPrivate.h"

_ptag void PPrivateEventEmit(PrivateCtx_t *ctx, PlatformEvent_t event, void *args)
{
    if(ctx->eventHandle)
    {
        ctx->eventHandle(event, args);
    }
}

_ptag int PPrivateSubDeviceDel(PrivateCtx_t *ctx, const char *did)
{
    puint16_t i;
    PMSubDevice_t *subDev = PNULL;
    ModulesVersion_t *module;
    PMProperty_t *property;
    ResourceInfo_t *resource;
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
                if(resource->name)
                {
                    free(resource->name);
                }
                if(resource->changed)
                {
                    free(resource->changed);
                }
                for(i = 0; i < resource->nodeNum; i++)
                {
                    PListForeach(&resource->node[i], node)
                    {
                        PListDel(node);
                        if(node->name)
                        {
                            free(node->name);
                        }
                        if(node->type == PROPERTY_TYPE_TEXT && node->value.text)
                        {
                            free(node->value.text);
                        }
                        free(node);
                    }
                }
                free(resource);
            }
            free(subDev);
            return 0;
        }
    }
    return -1;
}

_ptag int PPrivateSubDeviceRegister(PrivateCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(&ctx->subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            return 0;
        }
    }

    subDev = (PMSubDevice_t *)malloc(sizeof(PMSubDevice_t));
    if(subDev)
    {
        memset(subDev, 0, sizeof(PMSubDevice_t));
        subDev->online = pfalse;
        subDev->authStatus = SUBDEV_AUTH_NONE;
        strcpy(subDev->did, did);
        strcpy(subDev->pin, pin);
        strcpy(subDev->model, model);
        strcpy(subDev->version, version);

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


