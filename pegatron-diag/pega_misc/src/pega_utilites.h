#ifndef _PEGA_UTILITIES_H_
#define _PEGA_UTILITIES_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
int pega_util_shell(const char *cmd, char *value, size_t size);
//==============================================================================
int pega_util_remove_dir(const char *path);
//==============================================================================
int pega_util_file_read(char *data, const char *filename, int size);
//==============================================================================
int pega_util_file_write(char *pData, const char *filename, const char *envUserDir);
//==============================================================================
int pega_util_int_to_string(int num, char *out, size_t size);
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif /* (_PEGA_UTILITIES_H_) */
