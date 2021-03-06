#include "board.h"
#include <string.h>
#include "sx12xx.h"

Sx12xx_t sx12xx_handle;

void OnTxDone(void);

void OnRxDone(uint8_t * payload, uint16_t size, int16_t rssi, int8_t snr);

void OnTxTimeout(void);

void OnRxTimeout(void);

void OnRxError(void);

void 
sx12xx_init(Radio_t * radio, BoardBindings_t bindings)
{
    sx12xx_handle.bindings = bindings;
    // configure sx12xx radio events with local functions
    sx12xx_handle.radio_events.TxDone    = OnTxDone;
    sx12xx_handle.radio_events.RxDone    = OnRxDone;
    sx12xx_handle.radio_events.TxTimeout = OnTxTimeout;
    sx12xx_handle.radio_events.RxTimeout = OnRxTimeout;
    sx12xx_handle.radio_events.RxError   = OnRxError;

    // this function calls TimerInits and radio->IoIrqInit, which are
    // implemented here
    radio->Init(&sx12xx_handle.radio_events);

    // sleep the radio and wait for a send or receive call
    radio->Sleep();
}



Sx12xxState_t
sx12xx_handle_event(Sx12xxEvent_t event)
{
    // initialize state here but the callbacks from
    // the Semtech library (define below) may alter it
    sx12xx_handle.state = Sx12xxState_Busy;

    switch (event)
    {
    case Sx12xxEvent_DIO0:
        (*(sx12xx_handle.dio_irq_handles[0]))();
        break;
    case Sx12xxEvent_DIO1:
        // SX126x library only has the one handle, so fire it even fore DIO1
        (*(sx12xx_handle.dio_irq_handles[0]))();
        break;
    case Sx12xxEvent_DIO2:
        (*(sx12xx_handle.dio_irq_handles[2]))();
        break;
    case Sx12xxEvent_DIO3:
        (*(sx12xx_handle.dio_irq_handles[3]))();
        break;
    case Sx12xxEvent_DIO4:
        (*(sx12xx_handle.dio_irq_handles[4]))();
        break;
    case Sx12xxEvent_DIO5:
        (*(sx12xx_handle.dio_irq_handles[5]))();
        break;
    case Sx12xxEvent_Timer1:
        // TODO: needs to dispatch the callback stashed from TimerInit
        break;
    case Sx12xxEvent_Timer2:
        // TODO: needs to dispatch the callback stashed from TimerInit
        break;
    case Sx12xxEvent_Timer3:
        // TODO: needs to dispatch the callback stashed from TimerInit
        break;
    default:
        break;
    }

    return sx12xx_handle.state;
}

// each sx12xx board invokes this during initialization
void
IoIrqInit(IrqHandler * irq_handlers[NUM_IRQ_HANDLES])
{
    for (uint32_t i = 0; i < NUM_IRQ_HANDLES; i++)
    {
        sx12xx_handle.dio_irq_handles[i] = irq_handlers[i];
    }
}

void
OnTxDone(void)
{
    sx12xx_handle.state = Sx12xxState_TxDone;
}

uint8_t * sx12xx_get_raw_buffer() {
    return sx12xx_handle.raw_buffer;
}

void
OnRxDone(uint8_t * payload, uint16_t size, int16_t rssi, int8_t snr)
{
    sx12xx_handle.raw_buffer = payload;
    sx12xx_handle.state = Sx12xxState_RxDone;
    sx12xx_handle.rx_metadata.rx_len = size;
    sx12xx_handle.rx_metadata.rssi = rssi;
    sx12xx_handle.rx_metadata.snr  = snr;
}

Sx12xxRxMetadata_t 
sx12xx_get_rx_metadata() {
    Sx12xxRxMetadata_t metadata  = {
        .rx_len = sx12xx_handle.rx_metadata.rx_len,
        .rssi = sx12xx_handle.rx_metadata.rssi,
        .snr = sx12xx_handle.rx_metadata.snr,
    };
    return metadata;
}

void
OnTxTimeout(void)
{
    sx12xx_handle.state = Sx12xxState_TxTimeout;
}

void
OnRxTimeout(void)
{
    sx12xx_handle.state = Sx12xxState_RxTimeout;
}

void
OnRxError(void)
{
    sx12xx_handle.state = Sx12xxState_RxError;
}