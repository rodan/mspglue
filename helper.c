
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "helper.h"

// in case the user defines USE_ITOA_LUT locally
#include "config.h"

// |error| < 0.005
float _atan2f(const float y, const float x)
{

    float rv;
    float z = y / x;

    if (x == 0.0f) {
        if (y > 0.0f) {
            return PIBY2_FLOAT;
        }
        if (y == 0.0f) {
            return 0.0f;
        }
        return -PIBY2_FLOAT;
    }
    if (fabsf(z) < 1.0f) {
        rv = z / (1.0f + 0.28f * z * z);
        if (x < 0.0f) {
            if (y < 0.0f) {
                return rv - PI_FLOAT;
            }
            return rv + PI_FLOAT;
        }
    } else {
        rv = PIBY2_FLOAT - z / (z * z + 0.28f);
        if (y < 0.0f) {
            return rv - PI_FLOAT;
        }
    }
    return rv;
}

float _sin(const float x)
{
    unsigned char i;
    float denum = 1;
    float res = 0;
    float x_2 = x * x;
    float num = x;
    int s = 1;
    for (i = 0; i < PREC; i++) {
        res += s * (num / denum);
        denum = denum * (denum + 1) * (denum + 2);
        num = num * x_2;
        s = s * -1;
    }
    return res;
    /*
       float x_3 = x * x_2;
       float x_5 = x_3 * x_2;
       float x_7 = x_5 * x_2;
       float res = (x - x_3/6.0 + x_5/120.0 - x_7/5040.0 );
       return res;//+ x_9/362880.0); */
}

float _cos(const float x)
{
    unsigned char i;
    float denum = 2;
    float res = 1;
    float x_2 = x * x;
    float num = x_2;
    int s = -1;
    for (i = 0; i < PREC; i++) {
        res += s * (num / denum);
        denum = denum * (denum + 1) * (denum + 2);
        num = num * x_2;
        s = s * -1;
    }
    return res;
    /*

       float x_2 = x * x;
       float x_4 = x_2 * x_2;
       float x_6 = x_4 * x_2;
       float res = 1 - x_2 / 2.0 + x_4 / 24.0 - x_6 / 720.0 ;
       //float x_8 = x_6 * x_2;
       return res ;//+ x_8/40320.0;
     */
}

float _sqrt(const float number)
{
    unsigned char i = 0;
    float x0, sqx0, error;
    if (number < 1) {
        x0 = number * 2;
    } else {
        x0 = number / 2;
    }
    do {
        x0 = (x0 + (number / x0)) / 2;
        sqx0 = x0 * x0;
        error = (number - sqx0) / number;
        i++;
    } while (i < 20 && ((error > 0 && error > 0.01) || (error < 0 && error < -0.01)));

    return x0;
}

float radians(const float x)
{
    return PI * x / 180.0f;
}

float sq(const float x)
{
    return x * x;
}

// ###############################################
// #
// #  time functions
// #

uint32_t get_unixtime(struct ts t)
{
    const uint8_t days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    uint8_t i;
    uint16_t d;
    int16_t y;
    uint32_t rv;

    if (t.year >= 2000) {
        y = t.year - 2000;
    } else {
        return 0;
    }

    d = t.mday - 1;
    for (i = 1; i < t.mon; i++) {
        d += (uint16_t) (days_in_month[i - 1]);
    }
    if (t.mon > 2 && y % 4 == 0) {
        d++;
    }
    // count leap days
    d += (365 * y + (y + 3) / 4);
    rv = ((d * 24UL + t.hour) * 60 + t.min) * 60 + t.sec + SECONDS_FROM_1970_TO_2000;
    return rv;
}

// custom implementation of gmtime

uint8_t MonthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
    function that converts the number of seconds since January 1st 2000 into a tm structure
    input: time_t (uint32_t) seconds since January 1st 2000
    input: the converted tm struct is provided at the p_time pointer
    return: void
*/
void _gmtime(time_t sec_since_2000, struct tm *p_time)
{
    uint32_t tmp1, tmp2;
    uint16_t iBuf;
    uint8_t i;
    uint8_t leap;

    tmp1 = sec_since_2000 / 0x00015180; //days

    tmp2 = sec_since_2000 - tmp1 * 0x00015180;
    p_time->tm_hour = tmp2 / 3600;      //hours

    tmp1 = tmp2 - ((uint32_t) p_time->tm_hour) * 3600;
    p_time->tm_min = tmp1 / 60; //minute
    p_time->tm_sec = tmp1 - (uint32_t) p_time->tm_min * 60;     //second
    //we have the rest of the information in tmp  ; get day, month and year

    iBuf = sec_since_2000 / 0x00015180;
    for (i = 0; i < 100; i++) {
        leap = (i % 4 == 0);
        if (iBuf < (365 + leap))
            break;
        iBuf -= (365 + leap);
    }
    p_time->tm_year = i;        //year
    leap = (i % 4 == 0);
    for (i = 1;; i++) {
        if (iBuf < (MonthDays[i] + (i == 2 && leap)))
            break;
        iBuf -= MonthDays[i] + (i == 2 && leap);
    }
    p_time->tm_mon = i;         //month
    p_time->tm_mday = iBuf + 1; //day
}

// ###############################################
// #
// #  string functions
// #

uint8_t str_to_uint8(char *str, uint8_t * out, const uint8_t seek,
                     const uint8_t len, const uint8_t min, const uint8_t max)
{
    uint32_t val = 0, pow = 1;
    uint8_t i;

    // pow() is missing in msp gcc, so we improvise
    for (i = 0; i < len - 1; i++) {
        pow *= 10;
    }
    for (i = 0; i < len; i++) {
        if ((str[seek + i] > 47) && (str[seek + i] < 58)) {
            val += (str[seek + i] - 48) * pow;
        }
        pow /= 10;
    }
    if ((val >= min) && (val <= max)) {
        *out = val;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                      const uint8_t len, const uint16_t min, const uint16_t max)
{
    uint16_t val = 0;
    uint32_t pow = 1;
    uint8_t i, c;

    for (i = len + 1; i > seek; i--) {
        c = str[i - 1] - 48;
        if (c < 10) {
            val += c * pow;
            pow *= 10;
        } else {
            if (val) {
                // if we already have a number and this char is unexpected
                break;
            }
        }
    }

    if ((val >= min) && (val <= max)) {
        *out = val;
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

uint8_t str_to_uint32(char *str, uint32_t * out, const uint8_t seek,
                      const uint8_t len, const uint32_t min, const uint32_t max)
{
    uint32_t val = 0, pow = 1;
    uint8_t i;

    // pow() is missing in msp gcc, so we improvise
    for (i = 0; i < len - 1; i++) {
        pow *= 10;
    }
    for (i = 0; i < len; i++) {
        if ((str[seek + i] > 47) && (str[seek + i] < 58)) {
            val += (str[seek + i] - 48) * pow;
        } else {
            val /= 10;
        }
        pow /= 10;
    }
    if ((val >= min) && (val <= max)) {
        *out = val;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

uint8_t str_to_int32(char *str, int32_t * out, const uint8_t seek,
                     const uint8_t len, const int32_t min, const int32_t max)
{
    int32_t val = 0;
    uint32_t pow = 1;
    uint8_t i;
    int8_t sign = 1;

    // pow() is missing in msp gcc, so we improvise
    for (i = 0; i < len - 1; i++) {
        pow *= 10;
    }
    for (i = 0; i < len; i++) {
        if ((str[seek + i] > 47) && (str[seek + i] < 58)) {
            val += (str[seek + i] - 48) * pow;
        } else if (str[seek + i] == 45) {
            sign = -1;
        } else {
            val /= 10;
        }
        pow /= 10;
    }
    if ((val >= min) && (val <= max)) {
        *out = val * sign;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

uint8_t hstr_to_uint8(char *str, uint8_t * out, const uint8_t seek,
                      const uint8_t len, const uint8_t min, const uint16_t max)
{
    uint8_t val = 0;
    uint32_t pow = 1;
    uint8_t i;

    // pow() is missing in msp gcc, so we improvise
    for (i = seek; i < len; i++) {
        pow *= 16;
    }

    for (i = seek; i < len + 1; i++) {
        if ((str[i] > 47) && (str[i] < 58)) {
            // 0 - 9
            val += (str[i] - 48) * pow;
        } else if ((str[i] > 96) && (str[i] < 103)) {
            // a - f
            val += (str[i] - 87) * pow;
        } else if ((str[i] > 64) && (str[i] < 71)) {
            // A - F
            val += (str[i] - 55) * pow;
        }
        pow /= 16;
    }

    if ((val >= min) && (val <= max)) {
        *out = val;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

uint8_t hstr_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                       const uint8_t len, const uint16_t min, const uint16_t max)
{
    uint16_t val = 0;
    uint32_t pow = 1;
    uint8_t i;

    // pow() is missing in msp gcc, so we improvise
    for (i = seek; i < len; i++) {
        pow *= 16;
    }

    for (i = seek; i < len + 1; i++) {
        if ((str[i] > 47) && (str[i] < 58)) {
            // 0 - 9
            val += (str[i] - 48) * pow;
        } else if ((str[i] > 96) && (str[i] < 103)) {
            // a - f
            val += (str[i] - 87) * pow;
        } else if ((str[i] > 64) && (str[i] < 71)) {
            // A - F
            val += (str[i] - 55) * pow;
        }
        pow /= 16;
    }

    if ((val >= min) && (val <= max)) {
        *out = val;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

static uint16_t const hex_ascii[16] =
    { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65,
    0x66
};

char *_utoh(char *buf, const uint32_t val)
{
    char *p = buf + (CONV_BASE_8_BUF_SZ - 1);   // the very end of the buffer
    uint32_t m = val;
    uint8_t i = 0;

    *p = '\0';

    if (val == 0) {
        p -= 1;
        memcpy(p, &hex_ascii[0], sizeof(uint8_t));
    }
    // groups of 8 bits
    while (m > 0 || (i & 1)) {
        p -= 1;
        memcpy(p, &hex_ascii[m & 0xf], sizeof(uint8_t));
        m >>= 4;
        i++;
    }

    p -= 2;
    memcpy(p, "0x", sizeof(uint16_t));

    return p;
}

char *_utoh8(char *buf, const uint32_t val)
{
    char *p = buf + (CONV_BASE_8_BUF_SZ - 1);   // the very end of the buffer
    uint32_t m = val;
    uint8_t i = 0;

    *p = '\0';

    if (val == 0) {
        p -= 2;
        memcpy(p, &hex_ascii[0], sizeof(uint8_t));
        memcpy(p + 1, &hex_ascii[0], sizeof(uint8_t));
    }
    // groups of 8 bits
    while (m > 0 || (i & 1)) {
        p -= 1;
        memcpy(p, &hex_ascii[m & 0xf], sizeof(uint8_t));
        m >>= 4;
        i++;
    }

    return p;
}

char *_utoh16(char *buf, const uint32_t val)
{
    char *p = buf + (CONV_BASE_8_BUF_SZ - 1);   // the very end of the buffer
    uint32_t m = val;
    uint8_t i = 0;

    *p = '\0';

    if (val == 0) {
        p -= 4;
        memcpy(p, &hex_ascii[0], sizeof(uint8_t));
        memcpy(p + 1, &hex_ascii[0], sizeof(uint8_t));
        memcpy(p + 2, &hex_ascii[0], sizeof(uint8_t));
        memcpy(p + 3, &hex_ascii[0], sizeof(uint8_t));
        return p;
    }
    // groups of 8 bits
    while (m > 0 || (i & 1)) {
        p -= 1;
        memcpy(p, &hex_ascii[m & 0xf], sizeof(uint8_t));
        m >>= 4;
        i++;
    }

    if (val < 0x100) {
        p -= 2;
        memcpy(p, &hex_ascii[0], sizeof(uint8_t));
        memcpy(p + 1, &hex_ascii[0], sizeof(uint8_t));
    }

    return p;
}

char *_utoh32(char *buf, const uint32_t val)
{
    char *p = buf + CONV_BASE_8_BUF_SZ - 1;     // the very end of the buffer
    uint32_t m = val;
    uint8_t i = 0;

    *p = '\0';

    p -= 8;
    for (i=0;i<8;i++) {
        *(p + i) = 0x30;
    }

    p += 8;
    // groups of 8 bits
    while (m > 0 || (i & 1)) {
        p -= 1;
        memcpy(p, &hex_ascii[m & 0xf], sizeof(uint8_t));
        m >>= 4;
        i++;
    }

    return buf + 3;
}

static uint16_t const caps_hex_ascii[16] =
    { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46
};

char *_utorh(char *buf, const uint32_t val, const uint8_t pad_size)
{
    char *p = &buf[11];
    uint32_t m = val;
    uint8_t i = 0;
    uint8_t pc = 0;

    *p = '\0';

    if (val == 0) {
        for (pc = 0; pc < pad_size; pc++) {
            memcpy(p - (pc + 1), &caps_hex_ascii[0], sizeof(uint8_t));
        }
        p -= pad_size;
        return p;
    }
    // groups of 8 bits
    //while (m > 0 || (i & 1))
    while (m > 0 || (i < pad_size)) {
        p -= 1;
        memcpy(p, &caps_hex_ascii[m & 0xf], sizeof(uint8_t));
        m >>= 4;
        i++;
    }

    return p;
}

#ifdef USE_ITOA_LUT

static uint16_t const dec_ascii[10] =
    { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
static uint16_t const bin_ascii[2] = { 0x30, 0x31 };

char *_uint32toa(char *buf, const uint32_t val)
{
    char *p = buf + (CONV_BASE_10_BUF_SZ - 1);  // the very end of the buffer
    uint32_t m = val;

    *p = '\0';

    while (m >= 10) {
        uint32_t const old = m;

        p -= 1;
        m /= 10;
        memcpy(p, &dec_ascii[old - (m * 10)], sizeof(uint8_t));
    }

    p -= 1;
    memcpy(p, &dec_ascii[m], sizeof(uint8_t));

    return p;
}

char *_uint16toa(char *buf, const uint16_t val)
{
    char *p = buf + (CONV_BASE_10_BUF_SZ - 1);  // the very end of the buffer
    uint16_t m = val;

    *p = '\0';

    while (m >= 10) {
        uint32_t const old = m;

        p -= 1;
        m /= 10;
        memcpy(p, &dec_ascii[old - (m * 10)], sizeof(uint8_t));
    }

    p -= 1;
    memcpy(p, &dec_ascii[m], sizeof(uint8_t));

    return p;
}

char *_utob(char *buf, const uint16_t val)
{
    char *p = buf + (CONV_BASE_2_BUF_SZ - 1);   // the very end of the buffer
    uint16_t m = val;
    uint8_t i = 0;

    *p = '\0';

    if (val == 0) {
        p -= 1;
        memcpy(p, &bin_ascii[0], sizeof(uint8_t));
    }
    // groups of 8bits
    while (m > 0 || (i & 7)) {
        if (m > 0 && !(i & 7)) {
            p -= 1;
            *p = ' ';
        }
        p -= 1;
        memcpy(p, &bin_ascii[m & 0x1], sizeof(uint8_t));
        m >>= 1;
        i++;
    }

    return p;
}
#else

char *_uint32toa(char *buf, const uint32_t val)
{
    char *p = buf + (CONV_BASE_10_BUF_SZ - 1);  // the very end of the buffer
    uint32_t m = val;

    *p = '\0';

    if (val == 0) {
        p -= 1;
        *p = '0';
    }

    while (m > 0) {
        p -= 1;
        *p = (m % 10) + '0';
        m /= 10;
    }

    return p;
}

char *_uint16toa(char *buf, const uint16_t val)
{
    char *p = buf + (CONV_BASE_10_BUF_SZ - 1);  // the very end of the buffer
    uint32_t m = val;

    *p = '\0';

    if (val == 0) {
        p -= 1;
        *p = '0';
    }

    while (m > 0) {
        p -= 1;
        *p = (m % 10) + '0';
        m /= 10;
    }

    return p;
}

char *_utob(char *buf, const uint16_t val)
{
    char *p = buf + (CONV_BASE_2_BUF_SZ - 1);   // the very end of the buffer
    uint16_t m = val;
    uint8_t i = 0;

    *p = '\0';

    if (val == 0) {
        p -= 1;
        *p = '0';
    }
    // groups of 8bits
    while (m > 0 || (i & 7)) {
        if (m > 0 && !(i & 7)) {
            p -= 1;
            *p = ' ';
        }
        p -= 1;
        *p = (m & 0x1) + '0';
        m >>= 1;
        i++;
    }

    return p;
}

#endif

char *_utoa(char *buf, const uint32_t val)
{
    return _uint32toa(buf, val);
}

char *_itoa(char *buf, const int32_t val)
{
    char *p;
    if (val >= 0) {
        return _uint32toa(buf, val);
    } else {
        p = _uint32toa(buf, val * -1);
        *(p - 1) = '-';
        return p - 1;
    }
}

char *_i16toa(char *buf, const int16_t val)
{
    char *p;
    if (val >= 0) {
        return _uint16toa(buf, val);
    } else {
        p = _uint16toa(buf, val * -1);
        *(p - 1) = '-';
        return p - 1;
    }
}

char *prepend_padding(char *buf, char *converted_buf, const pad_type padding_type,
                      const uint8_t target_len)
{
    uint8_t conv_len;
    uint8_t buf_pos;
    uint8_t cnt;
    uint8_t padding_char = '0';

    conv_len = strlen(converted_buf);
    buf_pos = converted_buf - buf;

    // if not enough buffer space is available for the prepend
    if (buf_pos < target_len - conv_len) {
        return converted_buf;
    }
    // if the converted string is already longer than the target length
    if (target_len <= conv_len) {
        return converted_buf;
    }

    if (padding_type == PAD_SPACES) {
        padding_char = ' ';
    }

    for (cnt = 0; cnt < target_len - conv_len; cnt++) {
        *(buf + buf_pos - cnt - 1) = padding_char;
    }

    return buf + buf_pos - cnt;
}

void mem2ascii(uint8_t * data_in, uint8_t * data_out, uint8_t len)
{
    uint8_t byte;
    while (len) {
        byte = *data_in;
        byte >>= 4;
        byte &= 0x0f;
        if (byte > (uint8_t) 9) {
            byte += 0x37;
        } else {
            byte += 0x30;
        }
        *(data_out++) = byte;
        byte = *(data_in++);
        byte &= 0x0f;
        if (byte > (uint8_t) 9) {
            byte += 0x37;
        } else {
            byte += 0x30;
        }
        *(data_out++) = byte;
        len--;
    }
}

uint8_t dec_to_bcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

uint8_t bcd_to_dec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

uint16_t _flip_u16(const uint16_t val)
{
    uint16_t tmp;
    tmp = val << 8;

    return ((val >> 8) | tmp);
}

// ABCD -> DCBA
uint32_t _flip_u32(const uint32_t val)
{
    uint32_t a, b, c, d;

    a = (val & 0xff000000) >> 24;
    b = (val & 0x00ff0000) >> 8;
    c = (val & 0x0000ff00) << 8;
    d = (val & 0x000000ff) << 24;

    return (a | b | c | d);
}

uint32_t _wiretou32(char *buf, const uint16_t seek, const uint16_t len)
{
    uint32_t val = 0, c = 0;    //, pow = 1;
    uint8_t i, buf0;
    //char itoa_buf[18];

    buf0 = buf[seek];

    if ((buf0 < 0x30) || (buf0 > 0x46) || (buf0 == 0x40)) {
        // binary input
        for (i = 0; i < len; i++) {
            c = buf[seek + len - i - 1];
            val |= c << (i * 8);
        }
        return val;
    }
    // ascii(hex) input
    for (i = 0; i < len / 2; i++) {
        c = buf[seek + i * 2];
        if (c < 0x40) {
            c -= 48;
        } else {
            c -= 55;
        }
        val |= c << (i * 8 + 4);
        c = buf[seek + i * 2 + 1];
        if (c < 0x40) {
            c -= 48;
        } else {
            c -= 55;
        }
        val |= c << (i * 8);
    }

/*
    uart0_print(" val ");
    uart0_print(_utoh(&itoa_buf[0], val));
    uart0_print("\r\n");
*/
    return val;
}
