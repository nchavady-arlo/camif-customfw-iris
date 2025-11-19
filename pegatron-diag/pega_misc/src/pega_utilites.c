/*******************************************************************************
* File Name: pega_utilites.c
*
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <dirent.h>
//==============================================================================
#include "pega_utilites.h"
//==============================================================================
int pega_util_shell(const char *cmd, char *value, size_t size)
{    
    int ret = 0;
    FILE *fp;
    size_t len = 0, total=0;
    char *s = value;

    s[0] = '\0';
   	
    fp = popen(cmd, "r");
    if (!fp) 
	{
        fprintf(stderr, "Error: popen(\"%s\") failed\n", cmd);
        return -1;
    }
	
	while (fgets(s, size - total, fp) != NULL) 
	{
		len = strlen(s);
		total += len;
		if (total >= size) 
		{
			ret = 1;
			break;
		}
		s += len;
	}

    if (len > 0 && s[len-1] == '\n') 
	{
		s[len-1] = '\0';
	}
            
	pclose(fp);
	
    return ret;
}

int pega_util_remove_dir(const char *path) 
{
    DIR *dir = opendir(path);
    if (!dir) return -1;

    struct dirent *entry;
	struct stat statbuf;
    char fullpath[1024];

    while ((entry = readdir(dir)) != NULL) 
	{
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
            continue;
		}

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        printf("fullpath=%s\n!", fullpath);		
        
        if (stat(fullpath, &statbuf) == 0) 
		{
            if (S_ISDIR(statbuf.st_mode)) 
			{
                // 遞迴刪除子資料夾
                pega_util_remove_dir(fullpath);
            } else 
			{
                // 刪除檔案
                remove(fullpath);
            }
        }
    }

    closedir(dir);
    // 最後刪掉自己
    return rmdir(path);
}

int pega_util_file_read(char *data, const char *filename, int size)
{
    if (!data || !filename || size == 0) 
		return -1;

    FILE *file = fopen(filename, "r");
	
    if (!file) 
		return -1;

    size_t read_bytes = fread(data, 1, size - 1, file); // 留一個位元給 '\0'
	
    data[read_bytes] = '\0'; // 確保字串終止

    if (ferror(file)) 
	{
        fclose(file);
        return -2; // 讀取失敗
    }

    fclose(file);
	
    return (int)read_bytes; // 返回實際讀到的 bytes
}

int pega_util_file_write(char *pData, const char *filename, const char *envUserDir)
{
    FILE *fp;
    int rtn = 0;
    DIR *dir;
    char filepath[256];

    if (!pData || !filename || !envUserDir) 
	{        
		fprintf(stderr, "Invalid argument\n");
        return -1;
    }

    dir = opendir(envUserDir);
    if (dir) 
	{
        closedir(dir);
    } 
	else 
	{
        if (mkdir(envUserDir, 0755) != 0) 
		{            
			fprintf(stderr, "Unable to create directory %s\n", envUserDir);
            return -1;
        } 
		else 
		{           
			fprintf(stdout, "Directory %s is created\n", envUserDir);
        }
    }

    snprintf(filepath, sizeof(filepath), "%s/%s", envUserDir, filename);
    fp = fopen(filepath, "w");
    
	if (!fp) 
	{        
		fprintf(stderr, "Unable to open file %s\n", filepath);
        return -1;
    }

    fprintf(fp, "%s", pData);
    fclose(fp);
    // sync();  // 可選，視需求而定

    return rtn;
}

int pega_util_int_to_string(int num, char *out, size_t size)
{
    if (out == NULL || size == 0)
	{
        return -1;
	}

    int len = snprintf(out, size, "%d", num);

    // snprintf() 回傳實際需要的字元數，不含 '\0'
    if (len < 0 || (size_t)len >= size)
	{
        return -1; // 緩衝區不夠
	}

    return 0;
}
//==============================================================================