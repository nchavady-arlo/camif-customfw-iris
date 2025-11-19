#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>    // For O_* constants
#include <sys/stat.h> // For mode constants
#include <unistd.h>
//==============================================================================
#include "pega_dtbo.h"
//==============================================================================


int main(int argc, char *argv[])
{
	Pega_dtbo_init();
	
    return 0;	
}
