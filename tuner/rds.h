#ifndef RDS_H
#define RDS_H

#include <inttypes.h>

uint8_t *rdsGetText(void);
void rdsSetBlocks(uint8_t *rdsBlock);
void rdsDisable(void);
uint8_t rdsGetFlag(void);

#endif /* RDS_H */
