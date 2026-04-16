#pragma once
#include "gd32e23x.h"

struct RtcTime{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;

    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t day_of_week;
};

class Rtc {
private:
    void init();
public:
    Rtc() {
        init();
    }
    ~Rtc() {}

    void set(RtcTime &set_time);
    const RtcTime& get(void) const;
};