// i2c_sharing.h
#ifndef INCLUDE_GUARD__I2C_SHARING_H
#define INCLUDE_GUARD__I2C_SHARING_H


void I2C_borrow_bus_1(void);

void I2C_done_borrowing_bus_1(void);

void I2C_init_bus_1_interrupt_for_rx(void);

#endif // INCLUDE_GUARD__I2C_SHARING_H
