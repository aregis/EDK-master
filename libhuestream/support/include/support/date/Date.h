/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <chrono>

#include "support/util/TimeUtil.h"

using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::tm;

namespace support {

    class Date {
    public:
        /**
         Construct with current time point, stored in milliseconds
         since epoch (UTC)
         */
        Date();
        
        /**
         Assignment
         */
        Date& operator=(const Date& date);
        
        /**
         Compare two date objects
         @param date The date to compare
         @return Whether the objects match the operator
         */
        bool operator==(const Date& date) const;
        bool operator!=(const Date& date) const;
        bool operator< (const Date& date) const;
        bool operator> (const Date& date) const;
        bool operator>=(const Date& date) const;
        bool operator<=(const Date& date) const;
        
        /**
         Construct date with time in milliseconds
         @param time_ms The time in milliseconds
         */
        explicit Date(uint64_t time_ms);
        
        /**
         Construct date with time point, which is in milliseconds
         since epoch
         @param time The datetime in milliseconds since epoch
         */
        explicit Date(system_clock::time_point time);
        
        /**
         Get time in milliseconds since epoch (UTC)
         @return Time point in milliseconds since epoch
         */
        system_clock::time_point get_time() const;
        
        /**
         Get time since epoch in milliseconds (UTC)
         @return Time since epoch in milliseconds
         */
        milliseconds get_time_ms() const;
        
        /**
         Set time, which is in milliseconds since epoch (UTC)
         @param time Time point in milliseconds since epoch
         */
        void set_time(system_clock::time_point time);
        
        /**
         Get hours since midnight
         @return hours since midnight
         */
        unsigned int get_hours() const;
        
        /**
         Set hours since midnight
         @param hours hours since midnight
         */
        void set_hours(unsigned int hours);
        
        /**
         Get minutes after the hour
         @return minutes after the hour
         */
        unsigned int get_minutes() const;
        
        /**
         Set minutes after the hour
         @param minutes minutes after the hour
         */
        void set_minutes(unsigned int minutes);
        
        /**
         Get seconds after the minute
         @return Seconds after the minute
         */
        unsigned int get_seconds() const;
        
        /**
         Set seconds after the minute
         @param seconds Seconds after the minute
         */
        void set_seconds(unsigned int seconds);
        
        /**
         Get milliseconds after the second
         @return milliseconds after the second
         */
        unsigned int get_milliseconds() const;
        
        /**
         Set milliseconds after the second
         @param milliseconds The milliseconds after the second
         */
        void set_milliseconds(unsigned int milliseconds);
        
        /**
         Get day of the month
         @return Day of the month
         */
        unsigned int get_day() const;
        
        /**
         Set day of the month
         @param day Day of the month
         */
        void set_day(unsigned int day);
        
        /**
         Get month of the year (e.g. januari is 1)
         @return The month of the year
         */
        unsigned int get_month() const;
        
        /**
         Set month of the year (e.g. januari is 1)
         @param month The month of the year
         */
        void set_month(unsigned int month);
        
        /**
         Get the year
         @return the year
         */
        unsigned int get_year() const;
        
        /**
         Set the year
         @param year The year
         */
        void set_year(unsigned int year);

    private:
        /** the time representation of the date object */
        system_clock::time_point _time;
        
        /**
         Convert the time struct to a time point
         @param time_struct The time struct to convert
         @return The time point
         */
        system_clock::time_point to_time(tm* time_struct) const;
        
        /**
         Convert the time point to a time struct
         @param time_point The time point to convert
         @return The time struct
         */
        tm to_time_struct(system_clock::time_point time) const;
    };

}  // namespace support

