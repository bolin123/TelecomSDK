#include "PropertyManager.h"
#include "Util/PList.h"

_ptag static ModulesVersion_t *findModuleByDid(PrivateCtx_t *ctx, const char *did)
{
    PMSubDevice_t *subDev = PNULL;

    if(did == PNULL)
    {
        return &ctx->modules;
    }

    if(strcmp(ctx->did, did) == 0)
    {
        return &ctx->modules;
    }
    else
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            if(strcmp(subDev->did, did) == 0)
            {
                return &subDev->modules;
            }
        }
    }
    return PNULL;
}

_ptag int PMModuleSetVersion(PrivateCtx_t *ctx, const char *did, const char *name, const char *version)
{
    ModulesVersion_t *module;
    ModulesVersion_t *modhead = findModuleByDid(ctx, did);

    if(modhead == PNULL)
    {
        return -1;
    }

    module = (ModulesVersion_t *)malloc(sizeof(ModulesVersion_t));
    if(module)
    {
        memset(module, 0, sizeof(ModulesVersion_t));
        module->name = (char *)malloc(strlen(name) + 1);
        if(module->name)
        {
            module->name[0] = '\0';
            strcpy(module->name, name);
            strcpy(module->version, version);
            PListAdd(modhead, module);
            return 0;
        }
        else
        {
            free(module);
            return -2;
        }
    }
    return -2;
}

_ptag ResourceItems_t *PMFindResourceItemByID(ResourceItems_t *itemsHead, puint16_t id)
{
    ResourceItems_t *item;

    PListForeach(itemsHead, item)
    {
        if(item->idValue == id)
        {
            return item;
        }
    }
    return PNULL;
}

_ptag ResourceInfo_t *PMFindResourceInfoByDid(PrivateCtx_t *ctx, const char *did)
{
    PMSubDevice_t *subDev = PNULL;

    if(did == PNULL)
    {
        return &ctx->resources;
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

#if 0
_ptag int PMResourceInfoSetNumValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, int value)
{
    ResourceNode_t *rscNode;
    ResourceInfo_t *infoNode;
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

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
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

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
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

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
#endif

_ptag int PMResourceItemSet(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t id, PCommonInfo_t *value, int valNum)
{
    int i;
    ResourceNode_t *node = PNULL;
    ResourceItems_t *item;
    ResourceInfo_t *resource;
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

    if(head == PNULL)
    {
        return -1;
    }
    PListForeach(head, resource)
    {
        if(strcmp(resource->rscName, rscName) == 0)
        {
            item = PMFindResourceItemByID(&resource->items, id);
            if(item)
            {
                item->changed = ptrue;
                item->mode = RESOURCE_MODE_SET;
                for(i = 0; i < valNum; i++)
                {
                    PListForeach(&item->node, node)
                    {
                        if(strcmp(node->name, value[i].name) == 0)
                        {
                            if(node->type == PROPERTY_TYPE_TEXT)
                            {
                                if(node->value.text)
                                {
                                    if(strcmp(node->value.text, value[i].value.text) != 0)
                                    {
                                        free(node->value.text);
                                        node->value.text = PNULL;
                                    }
                                }

                                if(node->value.text == PNULL)
                                {
                                    node->value.text = (char *)malloc(strlen(value[i].value.text) + 1);
                                    node->value.text[0] = '\0';
                                    strcpy(node->value.text, value[i].value.text);
                                }
                            }
                            else
                            {
                                node->value.num = value[i].value.num;
                            }
                        }
                    }
                }
                return 0;
            }
            return -1;
        }
    }
    return -1;
}

_ptag int PMResourceItemDel(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t id)
{
    ResourceNode_t *node = PNULL;
    ResourceInfo_t *resource;
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

    if(head == PNULL)
    {
        return -1;
    }

    PListForeach(head, resource)
    {
        if(strcmp(resource->rscName, rscName) == 0)
        {
            ResourceItems_t *item = PMFindResourceItemByID(&resource->items, id);
            if(item)
            {
                PListDel(item);
                PListForeach(&item->node, node)
                {
                    PListDel(node);
                    free(node->name);
                    if(node->type == PROPERTY_TYPE_TEXT)
                    {
                        if(node->value.text)
                        {
                            free(node->value.text);
                        }
                    }
                    free(node);
                }
                free(item);
                return 0;
            }
        }
    }

    return -1;
}

_ptag int PMResourceItemAdd(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t id, PCommonInfo_t *value, int valNum)
{
    int i;
    ResourceNode_t *node = PNULL;
    ResourceInfo_t *resource;
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

    if(head == PNULL)
    {
        return -1;
    }

    PListForeach(head, resource)
    {
        if(strcmp(resource->rscName, rscName) == 0)
        {
            ResourceItems_t *item = PMFindResourceItemByID(&resource->items, id);
            if(item)
            {
                return -1;
            }
            item = (ResourceItems_t *)malloc(sizeof(ResourceItems_t));
            if(item)
            {
                item->changed = ptrue;
                item->idValue = id;
                item->mode = RESOURCE_MODE_ADD;
                PListInit(&item->node);

                for(i = 0; i < valNum; i++)
                {
                    node = (ResourceNode_t *)malloc(sizeof(ResourceNode_t));
                    if(node)
                    {
                        node->name = (char *)malloc(strlen(value[i].name) + 1);
                        node->name[0] = '\0';
                        strcpy(node->name, value[i].name);
                        if(value[i].isText)
                        {
                            node->type = PROPERTY_TYPE_TEXT;
                            node->value.text = (char *)malloc(strlen(value[i].value.text) + 1);
                            node->value.text[0] = '\0';
                            strcpy(node->value.text, value[i].value.text);
                        }
                        else
                        {
                            node->type = PROPERTY_TYPE_NUM;
                            node->value.num = value[i].value.num;
                        }
                        PListAdd(&item->node, node);
                    }
                }

                PListAdd(&resource->items, item);
                return 0;
            }
        }
    }

    return -1;
}

_ptag int PMResourceKeywordRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, const char *keyword, pbool_t isText)
{
    ResourceInfo_t *resource;
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

    if(head == PNULL)
    {
        return -1;
    }

    PListForeach(head, resource)
    {
        if(strcmp(resource->rscName, rscName) == 0)
        {
            resource->key[resource->keyNum] = keyword;
            resource->keyValueIsText[resource->keyNum] = isText;
            resource->keyNum++;
            return 0;
        }
    }
    return -1;
}

_ptag int PMResourceRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, const char *idName, puint16_t sid)
{
    //    puint16_t i;
    ResourceInfo_t *resource;
    ResourceInfo_t *head = PMFindResourceInfoByDid(ctx, did);

    if(head == PNULL)
    {
        return -1;
    }

    resource = (ResourceInfo_t *)malloc(sizeof(ResourceInfo_t));
    if(resource)
    {
        memset(resource, 0, sizeof(ResourceInfo_t));
        resource->rscName = (char *)malloc(strlen(rscName) + 1);
        if(resource->rscName)
        {
            resource->rscName[0] = '\0';
            strcpy(resource->rscName, rscName);
            resource->idName = (char *)malloc(strlen(idName) + 1);
            if(resource->idName)
            {
                resource->idName[0] = '\0';
                strcpy(resource->idName, idName);
            }
            resource->serialID = sid;
            PListInit(&resource->items);
            PListAdd(head, resource);
            return 0;
        }
        else
        {
            free(resource);
            return -2;
        }
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

    plog("id:%d, value:%s", id, value);
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

    plog("id:%d, value:%d", id, value);
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
_ptag PMProperty_t *PMFindPropertyHeadByDid(PrivateCtx_t *ctx, const char *did)
{
    PMSubDevice_t *subDev;
    if(did)
    {
        if(strcmp(ctx->did, did) == 0)
        {
            return &ctx->properties;
        }
        else
        {
            PListForeach(&ctx->subDevices, subDev)
            {
                if(strcmp(subDev->did, did) == 0)
                {
                    //subDev->online = ptrue;
                    return &subDev->property;
                }
            }
        }
    }
    else
    {
        return &ctx->properties;
    }
    return PNULL;
}

_ptag PMProperty_t *PMFindPropertyByName(PrivateCtx_t *ctx, const char *did, const char *name)
{
    PMProperty_t *property;
    PMProperty_t *phead = PMFindPropertyHeadByDid(ctx, did);

    if(phead && name)
    {
        PListForeach(phead, property)
        {
            if(strcmp(property->name, name) == 0)
            {
                return property;
            }
        }
    }
    return PNULL;
}

_ptag int PMPropertySetTextValue(PrivateCtx_t *ctx, const char *did, puint16_t id, const char *value)
{
    PMProperty_t *properties = PMFindPropertyHeadByDid(ctx, did);
    return propertySetTextValue(properties, id, value);
}

_ptag int PMPropertySetNumValue(PrivateCtx_t *ctx, const char *did, puint16_t id, puint32_t value)
{
    PMProperty_t *properties = PMFindPropertyHeadByDid(ctx, did);
    return propertySetNumValue(properties, id, value);
}

_ptag int PMPropertyRegister(PrivateCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo)
{
    PMProperty_t *properties = PMFindPropertyHeadByDid(ctx, did);
    if(properties && pInfo)
    {
        return propertyRegister(properties, pInfo);
    }
    return -1;
}


/***************************从设备属性管理********************************
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
*/

_ptag void PMInitialize(void)
{

}

_ptag void PMPoll(void)
{
}

