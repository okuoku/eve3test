#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sim_avr.h"
#include "sim_irq.h"
#include "avr_spi.h"
#include "avr_ioport.h"

#define _WIN32
#include "bt8xxemu.h"

struct bt8xxemubridge_s{
    /* States */
    avr_irq_t* irqs;
    BT8XXEMU_Emulator* emu;
};

enum {
    IRQ_BT8xxEMU_SPI_BYTE_IN = 0,
    IRQ_BT8xxEMU_SPI_BYTE_OUT,
    IRQ_BT8xxEMU_RST,
    IRQ_BT8xxEMU_CS,
    IRQ_BT8xxEMU_PD,
    IRQ_BT8xxEMU_COUNT
};

struct bt8xxemubridge_s bt8xxemubridge;
BT8XXEMU_EmulatorParameters bt8xxemuparams;

static const char* irq_names[IRQ_BT8xxEMU_COUNT] = {
    [IRQ_BT8xxEMU_SPI_BYTE_IN] = "8<bt8xx.IN",
    [IRQ_BT8xxEMU_SPI_BYTE_OUT] = "8>bt8xx.OUT",
    [IRQ_BT8xxEMU_RST] = "<bt8xx.RST",
    [IRQ_BT8xxEMU_CS] = "<bt8xx.CS",
    [IRQ_BT8xxEMU_PD] = "<bt8xx.PD"
};

static void
spi_in(struct avr_irq_t* irq, uint32_t value, void* param){
    const uint8_t in_data = value & 0xff;
    uint8_t out_data;

    out_data = BT8XXEMU_transfer(bt8xxemubridge.emu, in_data);

    printf("BT8xx: %02x => %02x \n", in_data, out_data);

    avr_raise_irq(bt8xxemubridge.irqs + IRQ_BT8xxEMU_SPI_BYTE_OUT,
                  out_data);
}

static void
cs(struct avr_irq_t* irq, uint32_t value, void* param){
    if(value){
        BT8XXEMU_chipSelect(bt8xxemubridge.emu, 0);
        printf("BT8xx: END\n");
    }else{
        BT8XXEMU_chipSelect(bt8xxemubridge.emu, 1);
        printf("BT8xx: START\n");
    }
}


void
bt8xxbridge_connect(struct avr_t* avr){

    /* Register IRQ */
    bt8xxemubridge.irqs = avr_alloc_irq(&avr->irq_pool, 0,
                                         IRQ_BT8xxEMU_COUNT, irq_names);

    avr_irq_register_notify(bt8xxemubridge.irqs + IRQ_BT8xxEMU_SPI_BYTE_IN,
                            spi_in, NULL);

    avr_irq_register_notify(bt8xxemubridge.irqs + IRQ_BT8xxEMU_CS,
                            cs, NULL);
    /*
    avr_irq_register_notify(bt8xxemubridge.irqs + IRQ_BT8xxEMU_RST,
                            rst, NULL);
    avr_irq_register_notify(bt8xxemubridge.irqs + IRQ_BT8xxEMU_PD,
                            pd, NULL);
                            */

    /* Connect IRQ to AVR peripherals */
    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_SPI_GETIRQ(0),
                                  SPI_IRQ_OUTPUT),
                    bt8xxemubridge.irqs + IRQ_BT8xxEMU_SPI_BYTE_IN);
    /* CS: Digital 7 = PORTD, bit 7 */
    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 7),
                    bt8xxemubridge.irqs + IRQ_BT8xxEMU_CS);

    /* Turn on BT8xxemu */
    BT8XXEMU_defaults(BT8XXEMU_VERSION_API,
                      &bt8xxemuparams,
                      BT8XXEMU_EmulatorBT816);
    BT8XXEMU_run(BT8XXEMU_VERSION_API, &bt8xxemubridge.emu, 
                 &bt8xxemuparams);
}

