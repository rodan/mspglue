#ifndef __UART0_H__
#define __UART0_H__

#ifdef __cplusplus
extern "C" {
#endif

#define  UART0_EV_NULL 0
#define    UART0_EV_RX 0x1
#define    UART0_EV_TX 0x2

void uart0_init(void);
void uart0_initb(const uint8_t baudrate);
void uart0_port_init(void);
uint16_t uart0_tx_str(const char *str, const uint16_t size);
uint16_t uart0_print(const char *str);

uint16_t uart0_tx_str2(const char *str, const uint16_t size);
uint16_t uart0_print2(const char *str);

uint8_t uart0_get_event(void);
void uart0_rst_event(void);
void uart0_set_eol(void);
char *uart0_get_rx_buf(void);
struct ringbuf *uart0_get_rx_ringbuf(void);

void uart0_set_rx_irq_handler(uint8_t (*input)(const uint8_t c));
void uart0_set_tx_irq_handler(uint8_t (*output)(void));

// sample inside-irq handlers
uint8_t uart0_rx_simple_handler(const uint8_t rx);
uint8_t uart0_rx_ringbuf_handler(const uint8_t c);


#ifdef __cplusplus
}
#endif

#endif
