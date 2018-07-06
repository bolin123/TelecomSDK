#include "PPrivate.h"
#include "PlatformCTypes.h"

static char g_token[PLATFORM_TOKEN_LEN + 1];
static char g_sessionKey[PLATFORM_SESSIONKEY_LEN + 1];
static char g_did[PLATFORM_DEVID_LEN + 1];
static char g_pin[PLATFORM_PIN_LEN + 1];
static char *g_model = PNULL;

_ptag const char *PPrivateGetToken(void)
{
    return g_token;
}

_ptag void PPrivateSetToken(const char *token)
{
    g_token[0] = '\0';
    strcpy(g_token, token);
}

_ptag void PPrivateSetSessionkey(const char *key)
{
    g_sessionKey[0] = '\0';
    strcpy(g_sessionKey, key);
}

_ptag const char *PPrivateGetSessionkey(void)
{
    return g_sessionKey;
}

_ptag void PPrivateSetPin(const char *pin)
{
    g_pin[0] = '\0';
    strcpy(g_pin, pin);
}

const char *PPrivateGetPin(void)
{
    return g_pin;
}

_ptag void PPrivateSetDevid(const char *did)
{
    g_did[0] = '\0';
    strcpy(g_did, did);
}

const char *PPrivateGetDevid(void)
{
    return g_did;
}

_ptag void PPrivateSetModel(const char *model)
{
    if(g_model)
    {
        free(g_model);
        g_model = PNULL;
    }
    g_model = malloc(strlen(model) + 1);
    if(g_model)
    {
        strcpy(g_model, model);
    }
}

const char *PPrivateGetModel(void)
{
    return g_model;
}
