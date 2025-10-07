#ifndef _PROJECT_PLATFORM_HEADER_
#define _PROJECT_PLATFORM_HEADER_

#define PERLA			1
#define PARAKEET		2
#define PI3				3
#define LAPTOP_64		4
#define LAPTOP_32		5

#define CPU_32			0
#define CPU_64			1

#if TARGET_BOARD == LAPTOP_64
#define CPU_BITS		CPU_64
#else
#define CPU_BITS		CPU_32
#endif

#endif

