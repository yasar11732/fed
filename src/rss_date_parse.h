#ifndef FED_RSS_DATE_PARSE
#define FED_RSS_DATE_PARSE

#include "str.h"

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

static inline bool isRssEmpty(char c) {
    return (c == ' ') || (c == '\t');
}
static void rss_skip_empty(parse_ctx *ctx)
{
    int i;
    for(i = 0; i < 32; i++)
    {
        if(isRssEmpty(ctx->data[ctx->i]))
            ctx->i++;
        else
            break;
    }
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

static inline bool rss_prefix(parse_ctx *ctx, const char *prefix) {
    bool success = strprefix(&ctx->data[ctx->i], prefix);
    if(success) {
        ctx->i += strlen(prefix);
    }
    return success;
}

static void rss_parse_timezone(parse_ctx *ctx)
{
    bool match = true;

    if (rss_prefix(ctx,"UT")) {
        // empty
    } else if (rss_prefix(ctx, "GMT")) {
        // empty
    } else if (rss_prefix(ctx, "EST")) {
        ctx->zone_h = -5;
    } else if (rss_prefix(ctx, "EDT")) {
        ctx->zone_h = -4;
    } else if (rss_prefix(ctx, "CST")) {
        ctx->zone_h = -6;
    } else if (rss_prefix(ctx, "CDT")) {
        ctx->zone_h = -5;
    } else if (rss_prefix(ctx, "MST")) {
        ctx->zone_h = -7;
    } else if (rss_prefix(ctx, "MDT")){
        ctx->zone_h = -6;
    } else if (rss_prefix(ctx, "PST")) {
        ctx->zone_h = -8;
    } else if (rss_prefix(ctx, "PDT")) {
        ctx->zone_h = -7;
    } else {
        match = false;
    }

    // (+|-)\d{4}
    if(!match) {
        char c = ctx->data[ctx->i];
        if((c == '-') || (c == '+') ) {
            char h1;
            char h2;
            char m1;
            char m2;

            h1 = ctx->data[ctx->i+1u];
            if(!isDigit(h1)) {
                ctx->success = false;
                return;
            }

            h2 = ctx->data[ctx->i+2u];       
            if(!isDigit(h2)) {
                ctx->success = false;
                return;
            }

            m1 = ctx->data[ctx->i+3u];
            if(!isDigit(m1)) {
                ctx->success = false;
                return;
            }

            m2 = ctx->data[ctx->i+4u];
            if(!isDigit(m2)) {
                ctx->success = false;
                return;
            }

            ctx->zone_h = char2_to_int(h1, h2);
            if(c == '-') {
                ctx->zone_h =  -1 * ctx->zone_h;
            }
            ctx->zone_m = char2_to_int(m1, m2);
            ctx->i += 5u;
        
        } else {
        
            // try parsing military time
            if(c == 'Z') {
                ctx->i++;
            } else if(c == 'A') {
                ctx->i++;
                ctx->zone_h = -1;
            } else if(c == 'M') {
                ctx->i++;
                ctx->zone_h = -12;
            } else if(c == 'N') {
                ctx->i++;
                ctx->zone_h = +1;
            } else if(c == 'Y') {
                ctx->i++;
                ctx->zone_h = +12;
            } else {
                ctx->success = false;
            }
        }
    }
}

// rss accept 2 digit or 4 digit year
static void rss_parse_year(parse_ctx *ctx)
{
    char y1 = 0;
    char y2 = 0;
    char y3 = 0;
    char y4 = 0;

    y1 = ctx->data[ctx->i];
    if(!isDigit(y1)) {
        ctx->success = false;
        return;
    }

    y2 = ctx->data[ctx->i + 1u];
    if(!isDigit(y2)) {
        ctx->success = false;
        return;
    }
    
    y3 = ctx->data[ctx->i + 2u];
    if(isDigit(y3)) {
        y4 = ctx->data[ctx->i + 3u];
    }
    
    if(isDigit(y3) && isDigit(y4)) {
        ctx->year = char4_to_int(y1, y2, y3, y4);
        ctx->i += 4u;
    } else {
        ctx->year = 1900 + char2_to_int(y1, y2);
        ctx->i += 2u;
    }
}

static void rss_parse_time(parse_ctx *ctx)
{

    char h1;
    char h2;
    char m1;
    char m2;
    char s1;
    char s2;
    
    h1 = ctx->data[ctx->i];

    if(!isDigitBetween(h1, 0, 2)) {
        ctx->success = false;
        return;
    }

    h2 = ctx->data[ctx->i+1u];
    if(!isDigit(h2)) {
        ctx->success = false;
        return;
    }

    if(ctx->data[ctx->i + 2u] != ':') {
        ctx->success = false;
        return;
    }

    m1 = ctx->data[ctx->i+3u];
    if(!isDigitBetween(m1, 0, 5)) {
        ctx->success = false;
        return;
    }

    m2 = ctx->data[ctx->i+4u];
    if(!isDigit(m2)) {
        ctx->success = false;
        return;
    }

    ctx->i += 5u;

    ctx->hours = char2_to_int(h1, h2);
    ctx->minutes = char2_to_int(m1, m2);

    // seconds part is optional, try to parse without error
    if(ctx->data[ctx->i] != ':') {
        return;
    }

    s1 = ctx->data[ctx->i + 1u];
    if(!isDigitBetween(s1, 0, 5)) {
        return;
    }

    s2 = ctx->data[ctx->i + 2u];
    if(!isDigit(s2)) {
        return;
    }
    
    ctx->i += 3u;
    ctx->seconds = char2_to_int(s1, s2);
}

static void rss_parse_wday(parse_ctx *ctx)
{
    for(int i = MON; i < END_WEEK_DAY; i++) {
        if(rss_prefix(ctx, weekdays[i])) {
            ctx->wday = (enum eweekdays)i;
            break;
        }
    }
}

static void rss_parse_mon(parse_ctx *ctx)
{
    for(int i = JAN; i < END_OF_MONTHS; i++) {

        if(rss_prefix(ctx, months[i])) {
            ctx->month = (enum emonths)i;
            return;
        }
    }

    ctx->success = false;
}

static void rss_parse_day(parse_ctx *ctx)
{
    char c1 = ctx->data[ctx->i];

    if(isDigit(c1)) {
        
        // need to read next char after we are sure
        // first char is not null.
        char c2 = ctx->data[ctx->i+1u];
        
        if(isDigit(c2)) {
            // we read two digits
            ctx->day = char2_to_int(c1, c2);
            ctx->i += 2u;
        } else {
            // we read one digit
            ctx->day = char_to_int(c1);
            ctx->i += 1u;
        }
    } else {
        ctx->success = false;
    }
}

static bool parse_rss_date(parse_ctx *ctx)
{
    rss_skip_empty(ctx);
    rss_parse_wday(ctx);

    if(ctx->success) {
        rss_skip_empty(ctx);
        rss_parse_day(ctx);
    }

    if(ctx->success) {
        rss_skip_empty(ctx);
        rss_parse_mon(ctx);
    }

    if(ctx->success) {
        rss_skip_empty(ctx);
        rss_parse_year(ctx);
    }

    if (ctx->success) {
        rss_skip_empty(ctx);
        rss_parse_time(ctx);
    }

    if(ctx->success) {
        rss_skip_empty(ctx);
        rss_parse_timezone(ctx);
    }

    return ctx->success;
};


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
