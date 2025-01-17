#ifndef LOCAL_H
#define LOCAL_H
#include <stddef.h>
#include <stdint.h>

void init_pinout(void);

void start_periodic_task(uint32_t period_ms);
void stop_periodic_task(void);
#endif /* LOCAL_H */
