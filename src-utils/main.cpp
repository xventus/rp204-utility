
//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   main.cpp
/// @author Petr Vanek
//
// @brief - code snippets of show calls

#include <stdio.h>
#include <string>
#include <memory>

#include "hardware/uart.h"
#include "gps.h"
#include "at2432.h"
#include "ds3231.h"
#include "pcf8574.h"
#include "beep.h"
#include "time_base.h"
#include "time_utils.h"
#include "debug_utils.h"

#define UART_ID uart0
#define UART_ID2 uart1
#define BAUD_RATE 9600
#define BAUD_RATE2 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define UART_TX_PIN2 4
#define UART_RX_PIN2 5

GPS gps;
void on_uart_rx();

void uart()
{

    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE2);
    uart_init(UART_ID2, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    gpio_set_function(UART_TX_PIN2, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN2, GPIO_FUNC_UART);

    uart_set_fifo_enabled(UART_ID2, false);

    irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
    irq_set_enabled(UART1_IRQ, true);

    uart_set_irq_enables(UART_ID2, true, false);
}

// RX interrupt handler
void on_uart_rx()
{
    while (uart_is_readable(UART_ID2))
    {
        uint8_t ch = uart_getc(UART_ID2);
        // Can we send it back?
        gps.parse(ch);
        if (gps.isValidTime())
        {

            datetime_t t = {
                .year = gps.year(),
                .month = gps.month(),
                .day = gps.day(),
                .dotw = gps.weekDay(), // 0 is Sunday, so 5 is Friday
                .hour = gps.hour(),
                .min = gps.minute(),
                .sec = gps.second()};

            rtc_set_datetime(&t);
            /*
               if (uart_is_writable(UART_ID2)) {
                       printf("%d.%d.%d\n", gps.hour(),gps.minute(),gps.second());
               }
               */
            gps.resetValidTime();
        }
    }
}

// ---------------------------------------------------------------------------------------

void beeptest()
{
    Beep bp(10);
    bp.init();
    for (auto i = 330; i < 1000; i++)
    {
        bp.beep(i);
        sleep_ms(1);
    }
    bp.off();
}

// ---------------------------------------------------------------------------------------

std::unique_ptr<PCF8574> g_keyboard4x4 = nullptr;

void keyboardIRQHandler(uint gpio, uint32_t events)
{
    if (g_keyboard4x4) {
            printf("[%02x] ", g_keyboard4x4->getKey4x4() /*g_keyboard4x4->getCharKey4x4()*/);
    }
}

void pcftest()
{
    
    g_keyboard4x4 = std::make_unique<PCF8574>(i2c1, 2, 3, 0x38);

    if (g_keyboard4x4->init(true))
    {
        printf("PCF8574 OK\n");
    }
    else
    {
        printf("PCF8574 FAILED\n");
        while (true)
            ;
    }

    gpio_set_function(15, GPIO_FUNC_SIO);
    gpio_set_dir(15, false);
    gpio_pull_up(15);
    gpio_set_irq_enabled_with_callback(15, GPIO_IRQ_EDGE_FALL, true, &keyboardIRQHandler);

    // port setting for reading
    g_keyboard4x4->getKey4x4();
    
    while (true) {};
}

// ---------------------------------------------------------------------------------------

void pcftest2()
{
    PCF8574 pc(i2c1, 2, 3, 0x38);

    if (pc.init(false))
    {
        printf("PCF8574 OK\n");
    }
    else
    {
        printf("PCF8574 FAILED\n");
        while (true)
            ;
    }

    while (true)
    {
        pc.setBit(0, false);
        sleep_ms(100);
        pc.setBit(0, true);
        sleep_ms(100);
        pc.setBit(0, false);
        sleep_ms(100);
        pc.setBit(0, true);
        sleep_ms(100);
        pc.setBit(2, true);
        sleep_ms(100);
        pc.setBit(2, false);
        sleep_ms(100);
        pc.setBit(0, false);
        sleep_ms(100);
        pc.setBit(0, true);
        sleep_ms(100);
    }
}

// ---------------------------------------------------------------------------------------

void at2432test()
{
    AT2432 at(i2c1, 2, 3, 0x57);

    if (at.init(true))
    {
        printf("OK\n");
    }
    else
    {
        printf("FAILED\n");
        while (true)
            ;
    }

    at.clearCheckSum();
    at.writeIO(0x00, 0x01);
    at.writeIO(0x03, 0xa1);
    at.writeIO(0x06, 0x1a);
    DebugUtils::memdump(at, 0, 16);
    printf("checksum: %02x\n", at.checkSum());
}

// ---------------------------------------------------------------------------------------

void ds3231test()
{
    DS3231 ds(i2c1, 2, 3, 0x68);
    if (ds.init(true))
    {
        printf("RTC OK\n");
    }
    else
    {
        printf("RTC FAILED\n");
        while (true)
            ;
    }

    // direct read
    auto v = ds.readIO(DS3231::_AM1S);
    printf("AM1S: %02x\n", v);
    ds.writeIO(DS3231::_AM1S, 23);
    v = ds.readIO(DS3231::_AM1S);
    printf("AM1S: %02x\n", v);

    if (ds.isOSF())
    {
        printf("DS: ON\n");
    }
    else
    {
        printf("DS: OFF\n");
    }

    ds.setHour(15);
    ds.setMinute(59);
    ds.setSecond(50);
    ds.setDay(10);
    ds.setMonth(2);
    ds.setYear(2022);
    ds.setWeekDay(2);

    ds.setOSF();
    if (ds.isOSF())
    {
        printf("DS: ON\n");
    }
    else
    {
        printf("DS: OFF\n");
    }

    while (true)
    {
        printf("%d:%d:%d  %d.%d.%d  weekD=%d\n", ds.hour(), ds.minute(), ds.second(), ds.day(), ds.month(), ds.year(), ds.weekDay());
        printf("temp  -------> %f\n", ds.temperature());
        sleep_ms(1000);
    }
}

void timeutiltest()
{

    auto a = TimeUtils::makeUnixTime(2023,
                                     1,
                                     17,
                                     8,
                                     34,
                                     27);

    if (1673944467 == a)
        printf("SUCCESS\n");

    a = TimeUtils::makeUnixTime(2024,
                                2,
                                29,
                                8,
                                34,
                                27);

    if (1709195667 == a)
        printf("SUCCESS\n");

    datetime_t tm;
    TimeUtils::breakUnixTime(a, tm);
    DebugUtils::printDatetime(tm);

    auto cestFrom = TimeUtils::timeShift(TimeUtils::CESTFrom, 2024);
    auto cestTo = TimeUtils::timeShift(TimeUtils::CESTTo, 2024);

    TimeUtils::breakUnixTime(cestFrom, tm);
    DebugUtils::printDatetime(tm);

    TimeUtils::breakUnixTime(cestTo, tm);
    DebugUtils::printDatetime(tm);

    auto localtm = TimeUtils::localTime(a, cestFrom, cestTo);
    TimeUtils::breakUnixTime(localtm, tm);
    DebugUtils::printDatetime(tm);
}

// ---------------------------------------------------------------------------------------

void timebasetest()
{
    // timesource  - external RTC
    DS3231 ds(i2c1, 2, 3, 0x68);
    ds.init(true);
    /*if (!ds.isOSF())*/ {
        // preset with some date
        ds.setHour(15);
        ds.setMinute(59);
        ds.setSecond(50);
        ds.setDay(16);
        ds.setMonth(1);
        ds.setYear(2023);
        ds.setWeekDay(1);
        ds.setOSF();
    }

    TimeBase tmb;

    if (tmb.init())
    {
        printf("timebase init: OK\n");
    }
    else
    {
        printf("timebase init: Failed\n");
    }

    // extrnal RTC UTC -> timebase
    auto val = ds.timeDate();
    tmb.updateTime(val);
    DebugUtils::printDatetime(val);

    // timebase print UTC
    auto val2 = tmb.getTimeDate();
    DebugUtils::printDatetime(val2);
}

// ---------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------

int main()
 {

    gps.init();
    stdio_init_all();
    //uart();

    printf("START   ------->\n");

    pcftest();
    //timeutiltest();

    // DebugUtils::i2cscan(i2c1);
    // ds3231test();

    printf("STOP   ------->\n");

    while(true) {};
}