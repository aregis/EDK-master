/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <locale>
#include <string>

#include "support/date/Date.h"
#include "support/date/Timezone.h"

using std::string;

#ifdef _MSC_VER
/* defines for helper functions of strptime */
void strptime_weekday_name(struct tm*, bool*, const char*);
void strptime_month_name(struct tm*, bool*, const char*);
void strptime_mdy(struct tm*, bool*, const char*&);
void strptime_hm(struct tm*, bool*, const char*&);
void strptime_hms(struct tm*, bool*, const char*&);
void strptime_ampm(struct tm*, bool*, const char*);
void strptime_12h(struct tm*, bool*, const char*&);
#endif

namespace support {

    class DateFormatter {
    public:
        /**
         Constructor which intializes the formatter 
         with a default format: %Y-%m-%dT%H:%M:%S
         @note Default timezone: UTC
         @see http://www.cplusplus.com/reference/iomanip/put_time/?kw=put_time
         */
        DateFormatter();
        
        /**
         Construct with format
         @see http://www.cplusplus.com/reference/iomanip/put_time/?kw=put_time
         @note Default timezone: UTC
         @param format The format to use
         */
        explicit DateFormatter(const string& format);
        
        /**
         Get the used format
         @return The used format
         */
        const string& get_format();
        
        /**
         Set the format
         @see http://www.cplusplus.com/reference/iomanip/put_time/?kw=put_time
         @param format The format to use
         */
        void set_format(const string& format);
        
        /**
         Convert the date object to a string representation based on the set format (UTC)
         @param date The date to convert
         @return The string representation
         */
        string to_string(const Date& date);
        
        /**
         Convert the date object to a string representation based on the set format
         and timezone
         @param date     The date to convert
         @param timezone The timezone in which the string should be represented
                         (calculated from UTC)
         @return The string representation
         */
        string to_string(const Date& date, const Timezone& timezone);
        
        /**
         Convert string to date object (UTC)
         @param date_str The date string to convert
         @return The date object
         */
        Date* from_string(const string& date_str);

        /**
         Convert string, which is represented in passed timezone, to date object (UTC)
         @param date_str The date string to convert
         @param timezone The string in represented in this timezone
         @return The date object in UTC
         */
        Date* from_string(const string& date_str, const Timezone& timezone);
        
    private:
        /** the format to use for conversion */
        string _format;
    };

}  // namespace support

