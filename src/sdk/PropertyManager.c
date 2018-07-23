#include "PropertyManager.h"
#include "Util/PList.h"

_ptag static ResourceInfo_t *findResourceInfoByDid(PrivateCtx_t *ctx, const char *did)
{
    PMSubDevice_t *subDev = PNULL;

    if(did == PNULL)
    {
        return PNULL;
    }

    if(strcmp(ctx->did, did) == 0)
    {
        return &ctx->resources;
    }
    else
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            if(strcmp(subDev->did, did) == 0)
            {
                return &subDev->resource;
            }
        }
    }
    return PNULL;
}

_ptag int PMResourceInfoSetNumValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, int value)
{
    ResourceNode_t *rscNode;
    ResourceInfo_t *infoNode;
    ResourceInfo_t *head = findResourceInfoByDid(ctx, did);

    if(head == PNULL || head->node)
    {
        return -1;
    }

    PListForeach(head, infoNode)
    {
        if(strcmp(infoNode->name, rscName) == 0)
        {
            if(infoID < infoNode->nodeNum)
            {
                PListForeach(&infoNode->node[infoID], rscNode)
                {
                    if(strcmp(rscNode->name, infoName) == 0)
                    {
                        if(rscNode->type == PROPERTY_TYPE_NUM)
                        {
                            if(rscNode->value.num != value)
                            {
                                rscNode->value.num = value;
                                infoNode->changed[infoID] = ptrue;
                            }
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return -1;
}

_ptag int PMResourceInfoSetTextValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, const char *value)
{
    ResourceNode_t *rscNode;
    ResourceInfo_t *infoNode;
    ResourceInfo_t *head = findResourceInfoByDid(ctx, did);

    if(head == PNULL || head->node)
    {
        return -1;
    }

    PListForeach(head, infoNode)
    {
        if(strcmp(infoNode->name, rscName) == 0)
        {
            if(infoID < infoNode->nodeNum)
            {
                PListForeach(&infoNode->node[infoID], rscNode)
                {
                    if(strcmp(rscNode->name, infoName) == 0)
                    {
                        if(rscNode->type == PROPERTY_TYPE_TEXT)
                        {
                            if(rscNode->value.text == PNULL || strcmp(rscNode->value.text, value) != 0)
                            {
                                if(rscNode->value.text)
                                {
                                    free(rscNode->value.text);
                                    rscNode->value.text = PNULL;
                                }
                                rscNode->value.text = (char *)malloc(strlen(value) + 1);
                                if(rscNode->value.text)
                                {
                                    rscNode->value.text[0] = '\0';
                                    strcpy(rscNode->value.text, value);
                                    infoNode->changed[infoID] = ptrue;
                                }
                            }
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return -1;
}

_ptag int PMResourceInfoRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, pbool_t isText)
{
    ResourceNode_t *rscNode;
    ResourceInfo_t *infoNode;
    ResourceInfo_t *head = findResourceInfoByDid(ctx, did);

    if(head == PNULL || head->node)
    {
        return -1;
    }

    PListForeach(head, infoNode)
    {
        if(strcmp(infoNode->name, rscName) == 0)
        {
            if(infoID < infoNode->nodeNum)
            {
                rscNode = (ResourceNode_t *)malloc(sizeof(ResourceNode_t));
                if(rscNode)
                {
                    rscNode->name = (char *)malloc(strlen(infoName) + 1);
                    if(rscNode->name)
                    {
                        rscNode->name[0] = '\0';
                        strcpy(rscNode->name, infoName);
                        if(isText)
                        {
                            rscNode->type = PROPERTY_TYPE_TEXT;
                            rscNode->value.text = PNULL;
                        }
                        else
                        {
                            rscNode->type = PROPERTY_TYPE_NUM;
                            rscNode->value.num = 0;
                        }
                        PListAdd(&infoNode->node[infoID], rscNode);
                        return 0;
                    }
                    else
                    {
                        free(rscNode);
                        return -1;
                    }
                }
                else
                {
                    return -1;
                }
            }
            return -2;
        }
    }

    return -1;
}

_ptag int PMResourceRegister(PrivateCtx_t *ctx, const char *did, const char *name, puint16_t sid, puint16_t infoNum)
{
    puint16_t i;
    ResourceInfo_t *resource;
    ResourceInfo_t *head = findResourceInfoByDid(ctx, did);

    if(head == PNULL)
    {
        return -1;
    }

    resource = (ResourceInfo_t *)malloc(sizeof(ResourceInfo_t));
    if(resource)
    {
        //malloc resource
        resource->name = (char *)malloc(strlen(name) + 1);
        if(resource->name)
        {
            strcpy(resource->name, name);
            resource->serialID = sid;
            resource->nodeNum = infoNum;
            resource->node = (ResourceNode_t *)malloc(sizeof(ResourceNode_t) * infoNum);
            resource->changed = (pbool_t *)malloc(infoNum * sizeof(pbool_t));
            if(resource->node && resource->changed)
            {
                for(i = 0; i < infoNum; i++)
                {
                    PListInit(&resource->node[i]);
                    resource->changed[i] = pfalse;
                }
                PListAdd(head, resource);
            }
            else
            {
                free(resource->name);
                free(resource);
                return -2;
            }
        }
        else
        {
            free(resource);
            return -2;
        }
        return 0;
    }
    return -1;
}

/***************************************************************/
/**********************属性管理*********************************/
/***************************************************************/
_ptag static PMProperty_t *findPropertyByID(PMProperty_t *head, puint16_t id)
{
    PMProperty_t *property = PNULL;

    PListForeach(head, property)
    {
        if(property->appID == id)
        {
            return property;
        }
    }
    return PNULL;
}

_ptag static int propertySetTextValue(PMProperty_t *head, puint16_t id, const char *value)
{
    PMProperty_t *property = findPropertyByID(head, id);

    if(property && property->type == PROPERTY_TYPE_TEXT)
    {
        if(property->value.text == PNULL || strcmp(property->value.text, value) != 0)
        {
            property->changed = ptrue;
            if(property->value.text)
            {
                free(property->value.text);
                property->value.text = PNULL;
            }
            property->value.text = (char *)malloc(strlen(value) + 1);
            if(property->value.text)
            {
                property->value.text[0] = '\0';
                strcpy(property->value.text, value);
            }
            else
            {
                return -2;
            }
        }
        return 0;
    }
    return -1;
}

_ptag static int propertySetNumValue(PMProperty_t *head, puint16_t id, puint32_t value)
{
    PMProperty_t *property = findPropertyByID(head, id);

    if(property && property->type == PROPERTY_TYPE_NUM)
    {
        if(property->value.num != value)
        {
            property->changed = ptrue;
            property->value.num = value;

        }
        return 0;
    }
    return -1;
}

_ptag static int propertyRegister(PMProperty_t *head, PPropertyInfo_t *pInfo)
{
    PMProperty_t *property = PNULL;

    PListForeach(head, property)
    {
        if(property->appID == pInfo->appid) // already register
        {
            return 0;
        }
    }

    property = (PMProperty_t *)malloc(sizeof(PMProperty_t));
    if(property)
    {
        property->changed = pfalse;
        property->appID = pInfo->appid;
        property->serialID = pInfo->sid;
        property->readonly = pInfo->readonly;
        property->name = (char *)malloc(strlen(pInfo->name) + 1);
        if(property->name)
        {
            property->name[0] = '\0';
            strcpy(property->name, pInfo->name);
        }
        else
        {
            return -2;
        }
        property->type = pInfo->isText ? PROPERTY_TYPE_TEXT : PROPERTY_TYPE_NUM;
        property->value.num = 0;
        PListAdd(head, property);
        return 0;
    }
    return -1;
}

/*****************************设备属性管理*****************************/
_ptag int PMPropertySetTextValue(PMProperty_t *properties, puint16_t id, const char *value)
{
    return propertySetTextValue(properties, id, value);
}

_ptag int PMPropertySetNumValue(PMProperty_t *properties, puint16_t id, puint32_t value)
{
    return propertySetNumValue(properties, id, value);
}

_ptag int PMPropertyRegister(PMProperty_t *properties, PPropertyInfo_t *pInfo)
{
    if(pInfo)
    {
        return propertyRegister(properties, pInfo);
    }
    return -1;
}

/*
_ptag int PMDeviceRegister(const char *did)
{
    g_masterDevice.did = PPrivateGetDevid();
    PListInit(&g_masterDevice.property);
    PListInit(&g_masterDevice.resource);
}
*/


/***************************从设备属性管理*********************************/
_ptag int PMSubPropertySetNumValue(PMSubDevice_t *subDevices, const char *did, puint16_t id, puint32_t value)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            return propertySetNumValue(&subDev->property, id, value);
        }
    }
    return -1;

}

_ptag int PMSubPropertySetTextValue(PMSubDevice_t *subDevices, const char *did, puint16_t id, const char *value)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            return propertySetTextValue(&subDev->property, id, value);
        }
    }
    return -1;
}

_ptag int PMSubPropertyRegister(PMSubDevice_t *subDevices, const char *did, PPropertyInfo_t *pInfo)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            return propertyRegister(&subDev->property, pInfo);
        }
    }
    return -1;
}

_ptag int PMSubDeviceRegister(PMSubDevice_t *subDevices, const char *did, const char *pin)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0) //already register
        {
            return 0;
        }
    }

    subDev = (PMSubDevice_t *)malloc(sizeof(PMSubDevice_t));
    if(subDev)
    {
        subDev->online = pfalse;
        subDev->auth = pfalse;
        subDev->did = (char *)malloc(strlen(did) + 1);
        if(subDev->did)
        {
            subDev->did[0] = '\0';
            strcpy(subDev->did, did);
        }
        else
        {
            return -2;
        }
        subDev->pin = (char *)malloc(strlen(pin) + 1);
        if(subDev->pin)
        {
            subDev->pin[0] = '\0';
            strcpy(subDev->pin, pin);
        }
        else
        {
            return -2;
        }
        PListInit(&subDev->property);
        PListInit(&subDev->resource);
        PListAdd(subDevices, subDev);
        return 0;
    }
    return -1;
}

_ptag void PMInitialize(void)
{

}

_ptag void PMPoll(void)
{
}

