/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */
#include "infra_config.h"

#ifdef FS_ENABLED

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

void *HAL_Fopen(const char *path, const char *mode)
{
    return (void *)fopen(path, mode);
}

uint32_t HAL_Fread(void *buff, uint32_t size, uint32_t count, void *stream)
{
    return fread(buff, (size_t)size, (size_t)count, (FILE *)stream);
}

int HAL_Fseek(void *stream, long offset, int framewhere)
{
    return fseek((FILE *)stream, offset, framewhere);
}

int HAL_Fclose(void *stream)
{
    return fclose((FILE *)stream);
}

long HAL_Ftell(void *stream)
{
    return ftell((FILE *)stream);
}
#endif

