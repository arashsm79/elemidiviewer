#if !defined SINTABLE_H
#define SINTABLE_H

#include <stdint.h>
#define SINTABLE_PHYSICAL_SIZE 65536
extern const int16_t SINTABLE_PHYSICAL[SINTABLE_PHYSICAL_SIZE];

#define SINTABLE_SIZE (SINTABLE_PHYSICAL_SIZE * 4)
static int16_t sintable(unsigned int index) __attribute__((__unused__));
static int16_t sintable(unsigned int index) {
	if (index < SINTABLE_PHYSICAL_SIZE)
		return SINTABLE_PHYSICAL[index];
	else if (index < 2 * SINTABLE_PHYSICAL_SIZE)
		return SINTABLE_PHYSICAL[SINTABLE_PHYSICAL_SIZE - (index - SINTABLE_PHYSICAL_SIZE) - 1];
	else if (index < 3 * SINTABLE_PHYSICAL_SIZE)
		return -SINTABLE_PHYSICAL[index - 2 * SINTABLE_PHYSICAL_SIZE];
	else
		return -SINTABLE_PHYSICAL[SINTABLE_PHYSICAL_SIZE - (index - 3 * SINTABLE_PHYSICAL_SIZE) - 1];
}

#endif

