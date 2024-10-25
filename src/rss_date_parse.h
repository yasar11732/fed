/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FED_RSS_DATE_PARSE
#define FED_RSS_DATE_PARSE

#include "str.h"

typedef struct {
    const char *str;
    int len;
} token_t;

enum eweekdays {
        MON,
        TUE,
        WED,
        THU,
        FRI,
        SAT,
        SUN,
        END_WEEK_DAY
};

enum emonths {
    JAN,
    FEB,
    MAR,
    APR,
    MAY,
    JUN,
    JUL,
    AUG,
    SEP,
    OCT,
    NOV,
    DEC,
    END_OF_MONTHS,
};

typedef struct {
    const char *data;
    size_t i; // current parsing index
    
    bool success;

    enum eweekdays wday;

    int day;
    enum emonths month;

    int year;
    int hours;
    int minutes;
    int seconds;
    int zone_h;
    int zone_m;
} parse_ctx;

const char *weekdays[] = {"Mon,","Tue,","Wed,","Thu,","Fri,","Sat,","Sun,"};
const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

// this is technically not correct, but good enough for parsing
// date format
static inline bool isRssDelim(char c) {
    return (c == ' ') || (c == '\t');
}

static inline bool tokeq(token_t tok, const char *s1) {
    return ((int)strlen(s1) == tok.len) && (strncmp(tok.str, s1, tok.len) == 0);
}

token_t next_token(parse_ctx *ctx) {
    token_t tok;

    // End of input
    if(ctx->data[ctx->i] == '\0') {
        tok.len = -1;
        tok.str = NULL;
        return tok;
    }

    tok.str = &ctx->data[ctx->i];
    tok.len = 0;
    while(ctx->data[ctx->i] && !isRssDelim(ctx->data[ctx->i])) {
        tok.len++;
        ctx->i++;
    }

    // skip delimiter for next call
    if (isRssDelim(ctx->data[ctx->i]))
        ctx->i++;

    return tok;
}

static bool isDigitBetween(char c, int lower, int upper)
{
    c -= '0';
    return (c >= lower) && (c <= upper);
}

static bool isDigit(char c)
{
    return isDigitBetween(c, 0, 9);
}

static int char_to_int(char c)
{
    return c - '0';
}

static int char2_to_int(char c1, char c2)
{
    return (char_to_int(c1) * 10) + char_to_int(c2);
}

static int char4_to_int(char c1, char c2, char c3, char c4)
{
    return (char2_to_int(c1, c2) * 100) + char2_to_int(c3, c4);
}

static void rss_parse_zone(parse_ctx *ctx)
{
    token_t tok = next_token(ctx);

    bool match = true;

    if (tokeq(tok,"UT") || tokeq(tok,"GMT")) {
        // empty
    } else if (tokeq(tok,"EDT")) {
        ctx->zone_h = -4;
    } else if (tokeq(tok,"EST") || tokeq(tok,"CDT")) {
        ctx->zone_h = -5;
    } else if (tokeq(tok,"CST") || tokeq(tok,"MDT")) {
        ctx->zone_h = -6;
    } else if (tokeq(tok,"MST") || tokeq(tok,"PDT")) {
        ctx->zone_h = -7;
    } else if (tokeq(tok,"PST")) {
        ctx->zone_h = -8;
    } else {
        match = false;
    }


    if(match) {
        return;
    }

    
    if(tok.len == 5 && (*tok.str == '-' || *tok.str == '+')) {
        // (+|-)\d{4}
        if(
            isDigitBetween(tok.str[1], 0, 2) &&
            isDigit(tok.str[2]) &&
            isDigitBetween(tok.str[3], 0, 5) &&
            isDigit(tok.str[4])
        ) {
            ctx->zone_h = char2_to_int(tok.str[1], tok.str[2]);
            if(*tok.str == '-') {
                ctx->zone_h =  -1 * ctx->zone_h;
            }
            ctx->zone_m = char2_to_int(tok.str[3], tok.str[4]);
        } else {
            ctx->success = false;
        }
    } else if(tok.len == 1) {
        // military time
        switch(*tok.str) {
            case 'Z':
            break;
            case 'A':
                ctx->zone_h = -1;
            break;
            case 'M':
                ctx->zone_h = -12;
            break;
            case 'N':
                ctx->zone_h = +1;
            break;
            case 'Y':
                ctx->zone_h = +12;
            break;
            default:
                ctx->success = false;
        }
    } else {
        ctx->success = false;
    }
}

// rss accept 2 digit or 4 digit year
static void rss_parse_year(parse_ctx *ctx)
{
    token_t tok = next_token(ctx);

    if(tok.len == 2) {

        if(isDigit(tok.str[0]) && isDigit(tok.str[1])) {
             ctx->year = 1900 + char2_to_int(tok.str[0], tok.str[1]);
        } else {
            ctx->success = false;
        }
    } else if (tok.len == 4) {
        if(isDigit(tok.str[0]) &&
           isDigit(tok.str[1]) &&
           isDigit(tok.str[2]) &&
           isDigit(tok.str[3])) {

            ctx->year = char4_to_int(tok.str[0], tok.str[1], tok.str[2], tok.str[3]);
        } else {
            ctx->success = false;
        }
    } else {
        ctx->success = false;
    }
}

static void rss_parse_hour(parse_ctx *ctx)
{

    token_t tok = next_token(ctx);

    // hour  =  2DIGIT ":" 2DIGIT [":" 2DIGIT] ; 00:00:00 - 23:59:59

    if((tok.len != 5) && (tok.len != 8)) {
        ctx->success = false;
        return;
    }

    if(isDigitBetween(tok.str[0], 0, 2) &&
       isDigit(tok.str[1]) &&
       tok.str[2] == ':' &&
       isDigitBetween(tok.str[3], 0, 5) &&
       isDigit(tok.str[4])
    ) {
        ctx->hours = char2_to_int(tok.str[0], tok.str[1]);
        ctx->minutes = char2_to_int(tok.str[3], tok.str[4]);

        // we have optional seconds part
        if(tok.len == 8) {
            if(
                tok.str[5] == ':' &&
                isDigitBetween(tok.str[6], 0, 5) &&
                isDigit(tok.str[7])
            ) {
                ctx->seconds = char2_to_int(tok.str[6], tok.str[7]);
            } else {
                ctx->success = false;
            }
        }

    } else {
        ctx->success = false;
    }
}

static void rss_parse_wday(parse_ctx *ctx)
{
    // if we fail to parse, we reset this
    size_t pos = ctx->i;
    token_t tok = next_token(ctx);

    for(int i = MON; i < END_WEEK_DAY; i++) {
        if(tokeq(tok, weekdays[i])) {
            ctx->wday = (enum eweekdays)i;
            return;
        }
    }

    ctx->i = pos;
}

static void rss_parse_mon(parse_ctx *ctx)
{
    token_t tok = next_token(ctx);

    for(int i = JAN; i < END_OF_MONTHS; i++) {

        if(tokeq(tok, months[i])) {
            ctx->month = (enum emonths)i;
            return;
        }
    }

    ctx->success = false;
}

static void rss_parse_day(parse_ctx *ctx)
{
    token_t tok = next_token(ctx);

    if(tok.len == 1 && isDigit(tok.str[0])) {
        ctx->day = char_to_int(tok.str[0]);
    } else if(tok.len == 2 && isDigit(tok.str[0]) && isDigit(tok.str[1])) {
        ctx->day = char2_to_int(tok.str[0], tok.str[1]);
    } else {
        ctx->success = false;
    }
}

static bool parse_rss_date(parse_ctx *ctx)
{
    rss_parse_wday(ctx);
    rss_parse_day(ctx); 
    
    if(ctx->success) {
        rss_parse_mon(ctx);
    }

    if(ctx->success) {
        rss_parse_year(ctx);
    }

    if (ctx->success) {
        rss_parse_hour(ctx);
    }

    if(ctx->success) {
        rss_parse_zone(ctx);
    }

    return ctx->success;
}


bool date_rss_to_sqlite(const char *rssdate, char *sqlitedate)
{
    int formatted_len = 0;
    parse_ctx ctx = {0};
    ctx.success = true;
    ctx.data = rssdate;
    if(parse_rss_date(&ctx)) {
        char sign = '+';
        if(ctx.zone_h < 0) {
            sign = '-';
            ctx.zone_h = ctx.zone_h * -1;
        }

        formatted_len = sprintf(sqlitedate, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
            ctx.year, ((int)ctx.month)+1, ctx.day, ctx.hours, ctx.minutes,
            ctx.seconds, sign, ctx.zone_h, ctx.zone_m);
    }

    return formatted_len == 25;
}

#endif
