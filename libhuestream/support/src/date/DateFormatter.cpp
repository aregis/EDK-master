/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <ctime>
#include <sstream>
#include <string>
#include <algorithm>

#include "support/date/DateFormatter.h"

using std::time_t;
using std::tm;
using std::string;
using std::stringstream;
using std::strftime;

// strptime() is not implemented in VS2013 C++.

#ifdef _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

const char * strp_weekdays[] = { "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday" };
const char * strp_monthnames[] = { "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december" };

bool strp_atoi(const char*& s, int& result, int low, int high, int offset) {  //NOLINT (runtime/references)
    bool worked = false;
    char * end;
    int num = strtoul(s, &end, 10);
    if (num >= low && num <= high) {
        result = num + offset;
        s = end;
        worked = true;
    }
    return worked;
}

void strptime_weekday_name(struct tm* tm, bool* working, const char* s) {
    tm->tm_wday = -1;
    *working = false;
    for (int i = 0; i < 7; ++i) {
        size_t len = strlen(strp_weekdays[i]);
        if (!_strnicmp(strp_weekdays[i], s, len)) {
            tm->tm_wday = i;
            s += len;
            *working = true;
            break;
        } else if (!_strnicmp(strp_weekdays[i], s, 3)) {
            tm->tm_wday = i;
            s += 3;
            *working = true;
            break;
        }
    }
}

void strptime_month_name(struct tm* tm, bool* working, const char* s) {
    tm->tm_mon = -1;
    *working = false;
    for (int i = 0; i < 12; ++i) {
        size_t len = strlen(strp_monthnames[i]);
        if (!_strnicmp(strp_monthnames[i], s, len)) {
            tm->tm_mon = i;
            s += len;
            *working = true;
            break;
        } else if (!_strnicmp(strp_monthnames[i], s, 3)) {
            tm->tm_mon = i;
            s += 3;
            *working = true;
            break;
        }
    }
}

void strptime_mdy(struct tm* tm, bool* working, const char*& s) {  //NOLINT (runtime/references)
    const char * s_save = s;
    *working = strp_atoi(s, tm->tm_mon, 1, 12, -1);
    if (*working && *s == '/') {
        ++s;
        *working = strp_atoi(s, tm->tm_mday, 1, 31, 0);
        if (*working && *s == '/') {
            ++s;
            *working = strp_atoi(s, tm->tm_year, 0, 99, 0);
            if (*working && tm->tm_year < 69)
                tm->tm_year += 100;
        }
    }
    if (!*working)
        s = s_save;
}

void strptime_ampm(struct tm* tm, bool* working, const char* s) {
    if (!_strnicmp(s, "am", 2)) {
        // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
        if (tm->tm_hour == 12)  // 12 am == 00 hours
            tm->tm_hour = 0;
    } else if (!_strnicmp(s, "pm", 2)) {
        if (tm->tm_hour < 12)  // 12 pm == 12 hours
            tm->tm_hour += 12;  // 1 pm -> 13 hours, 11 pm -> 23 hours
    } else {
        *working = false;
    }
}

void strptime_12h(struct tm* tm, bool* working, const char*& s) {  //NOLINT (runtime/references)
    const char * s_save = s;
    *working = strp_atoi(s, tm->tm_hour, 1, 12, 0);
    if (*working && *s == ':') {
        ++s;
        *working = strp_atoi(s, tm->tm_min, 0, 59, 0);
        if (*working && *s == ':') {
            ++s;
            *working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
            if (*working && isspace(static_cast<int>(*s))) {
                ++s;
                while (isspace(static_cast<int>(*s)))
                    ++s;
                if (!_strnicmp(s, "am", 2)) {
                    // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                    if (tm->tm_hour == 12)  // 12 am == 00 hours
                        tm->tm_hour = 0;
                } else if (!_strnicmp(s, "pm", 2)) {
                    if (tm->tm_hour < 12)  // 12 pm == 12 hours
                        tm->tm_hour += 12;  // 1 pm -> 13 hours, 11 pm -> 23 hours
                } else {
                    *working = false;
                }
            }
        }
    }
    if (!*working)
        s = s_save;
}

void strptime_hm(struct tm* tm, bool* working, const char*& s) {  //NOLINT (runtime/references)
    const char * s_save = s;
    *working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
    if (*working && *s == ':') {
        ++s;
        *working = strp_atoi(s, tm->tm_min, 0, 59, 0);
    }
    if (!*working)
        s = s_save;
}

void strptime_hms(struct tm* tm, bool* working, const char*& s) {  //NOLINT (runtime/references)
    const char * s_save = s;
    *working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
    if (*working && *s == ':') {
        ++s;
        *working = strp_atoi(s, tm->tm_min, 0, 59, 0);
        if (*working && *s == ':') {
            ++s;
            *working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
        }
    }
    if (!*working)
        s = s_save;
}

char* strptime(const char *s, const char *format, struct tm *tm) {
    bool working = true;
    while (working && *format && *s) {
        switch (*format) {
            case '%':
            {
                ++format;
                switch (*format) {
                    case 'a':
                    case 'A':  // weekday name
                        strptime_weekday_name(tm, &working, s);
                        break;
                    case 'b':
                    case 'B':
                    case 'h':  // month name
                        strptime_month_name(tm, &working, s);
                        break;
                    case 'd':
                    case 'e':  // day of month number
                        working = strp_atoi(s, tm->tm_mday, 1, 31, 0);
                        break;
                    case 'D':  // %m/%d/%y
                        strptime_mdy(tm, &working, s);
                        break;
                    case 'H':  // hour
                        working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
                        break;
                    case 'I':  // hour 12-hour clock
                        working = strp_atoi(s, tm->tm_hour, 1, 12, 0);
                        break;
                    case 'j':  // day number of year
                        working = strp_atoi(s, tm->tm_yday, 1, 366, -1);
                        break;
                    case 'm':  // month number
                        working = strp_atoi(s, tm->tm_mon, 1, 12, -1);
                        break;
                    case 'M':  // minute
                        working = strp_atoi(s, tm->tm_min, 0, 59, 0);
                        break;
                    case 'n':  // arbitrary whitespace
                    case 't':
                        while (isspace(static_cast<int>(*s)))
                            ++s;
                        break;
                    case 'p':  // am / pm
                        strptime_ampm(tm, &working, s);
                        break;
                    case 'r':  // 12 hour clock %I:%M:%S %p
                        strptime_12h(tm, &working, s);
                        break;
                    case 'R':  // %H:%M
                        strptime_hm(tm, &working, s);
                        break;
                    case 'S':  // seconds
                        working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
                        break;
                    case 'T':  // %H:%M:%S
                        strptime_hms(tm, &working, s);
                        break;
                    case 'w':  // weekday number 0->6 sunday->saturday
                        working = strp_atoi(s, tm->tm_wday, 0, 6, 0);
                        break;
                    case 'Y':  // year
                        working = strp_atoi(s, tm->tm_year, 1900, 65535, -1900);
                        break;
                    case 'y':  // 2-digit year
                        working = strp_atoi(s, tm->tm_year, 0, 99, 0);
                        if (working && tm->tm_year < 69)
                            tm->tm_year += 100;
                        break;
                    case '%':  // escaped
                        if (*s != '%')
                            working = false;
                        ++s;
                        break;
                    default:
                        working = false;
                }
            }
                break;
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\f':
            case '\v':
                // zero or more whitespaces:
                while (isspace(static_cast<int>(*s)))
                    ++s;
                break;
            default:
                // match character
                if (*s != *format)
                    working = false;
                else
                    ++s;
                break;
        }
        ++format;
    }
    return (working) ? const_cast<char*>(s) : NULL;
}

#endif  // _MSC_VER


namespace support {
    /** */
    static const int    TIME_PARSE_ERROR = -1;
    /** */
    static const size_t TIME_BUFFER_SIZE = 50;

                                     // Example: 2014-01-01T12:00::01
    DateFormatter::DateFormatter() : _format("%Y-%m-%dT%H:%M:%S") { }
    
    DateFormatter::DateFormatter(const string& format) : _format(string(format)) { }
    
    const string& DateFormatter::get_format() {
        return _format;
    }
    
    void DateFormatter::set_format(const string& format) {
        _format = format;
    }
    
    string DateFormatter::to_string(const Date& date) {
        char date_str[TIME_BUFFER_SIZE];
        
        time_t time_raw = system_clock::to_time_t(date.get_time());
        // Convert to time struct (UTC)
        tm time_struct = support::gmtime(&time_raw);
        // Convert to time string (UTC)
        // Note VS2013 compiler is not supported some formats e.g. %F or %T
        // if these formats are required make sence look to
        // JX9 implementation of the strftime() function from unqlite.c (see jx9Strftime)
        strftime(date_str, TIME_BUFFER_SIZE, _format.c_str(), &time_struct);

        return string(date_str);
    }
    
    string DateFormatter::to_string(const Date& date, const Timezone& timezone) {
        if (timezone.get_name() == "UTC") {
            // Convert to time string (UTC)
            return to_string(date);
        }

        char date_str[TIME_BUFFER_SIZE];
        
        /* local time */
        time_t time_raw = system_clock::to_time_t(date.get_time());
        // Convert to time struct (local)
        tm time_struct = support::localtime(&time_raw);
        // Convert to time string (local)
        strftime(date_str, TIME_BUFFER_SIZE, _format.c_str(), &time_struct);
        
        return string(date_str);
    }
    
    Date* DateFormatter::from_string(const string& date_str) {
        Date* date = nullptr;
    
        tm time_struct;
        std::fill(reinterpret_cast<char*>(&time_struct), reinterpret_cast<char*>(&time_struct + 1), 0);
        // Convert date string to time struct
        // TO DO: fix error C3861: 'strptime': identifier not found
        char* result = strptime(date_str.c_str(), _format.c_str(), &time_struct);
        
        if (result != nullptr && time_struct.tm_year >= 70) {
            // Convert to raw time (UTC)
            time_t time = support::timegm(&time_struct);

            if (time != TIME_PARSE_ERROR) {
                // No error occurred, so create the date object
                date = new Date(system_clock::from_time_t(time));
            }
        }
        return date;
    }

    Date* DateFormatter::from_string(const string& date_str, const Timezone& timezone) {
        if (timezone.get_name() == "UTC") {
            // Convert to date (UTC)
            return from_string(date_str);
        }
        
        
        /* local time */
        
        Date* date = nullptr;
        
        tm time_struct;
        // Convert date string to time struct
        // TO DO: fix error C3861: 'strptime': identifier not found
        strptime(date_str.c_str(), _format.c_str(), &time_struct);

        // mktime determines whether dst is in effect
        time_struct.tm_isdst = -1;
        // Convert to raw time (local)
        time_t time          = support::mktime(&time_struct);
        
        if (time != TIME_PARSE_ERROR) {
            // No error occurred, so create the date object
            date = new Date(system_clock::from_time_t(time));
        }
        
        return date;
    }

}  // namespace support
