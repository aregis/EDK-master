/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/date/Date.h"

using std::chrono::duration_cast;

namespace support {

    Date::Date() : _time(system_clock::now()) { }

    Date::Date(uint64_t time) : _time(milliseconds(time)) { }

    Date::Date(system_clock::time_point time) : _time(time) { }
    
    Date& Date::operator=(const Date& date) {
        _time = date._time;
        return *this;
    }
    
    bool Date::operator==(const Date& date) const {
        return _time == date._time;
    }

    bool Date::operator!=(const Date& date) const {
        return _time != date._time;
    }
    
    bool Date::operator< (const Date& date) const {
        return _time < date._time;
    }
    
    bool Date::operator> (const Date& date) const {
        return _time > date._time;
    }
    
    bool Date::operator>=(const Date& date) const {
        return _time >= date._time;
    }
    
    bool Date::operator<=(const Date& date) const {
        return _time <= date._time;
    }

    system_clock::time_point Date::get_time() const {
        return _time;
    }
    
    milliseconds Date::get_time_ms() const {
        return duration_cast<milliseconds>(_time.time_since_epoch());
    }

    void Date::set_time(system_clock::time_point time) {
        _time = time;
    }
    
    unsigned int Date::get_hours() const {
        return (unsigned int)to_time_struct(_time).tm_hour;
    }
    
    void Date::set_hours(unsigned int hour) {
        tm time_struct = to_time_struct(_time);
        // Set hour
        time_struct.tm_hour = static_cast<int>(hour);
        
        _time = to_time(&time_struct);
    }
    
    unsigned int Date::get_minutes() const {
        return (unsigned int)to_time_struct(_time).tm_min;
    }
    
    void Date::set_minutes(unsigned int minute) {
        tm time_struct = to_time_struct(_time);
        // Set minute
        time_struct.tm_min = static_cast<int>(minute);
        
        _time = to_time(&time_struct);
    }
    
    unsigned int Date::get_seconds() const {
        return (unsigned int)to_time_struct(_time).tm_sec;
    }
    
    void Date::set_seconds(unsigned int seconds) {
        tm time_struct = to_time_struct(_time);
        // Set seconds
        time_struct.tm_sec = static_cast<int>(seconds);
        
        _time = to_time(&time_struct);
    }
    
    unsigned int Date::get_milliseconds() const {
        // Get current time in ms
        milliseconds time       = duration_cast<milliseconds>(_time.time_since_epoch());
        // Get milliseconds fraction
        milliseconds time_fract = time % 1000;
        
        return (unsigned int)time_fract.count();
    }
    
    void Date::set_milliseconds(unsigned int millisec) {
        // Reset current milliseconds to 0
        _time -= milliseconds(get_milliseconds());
        // Set milliseconds
        _time += milliseconds(millisec);
    }
    
    unsigned int Date::get_day() const {
        return (unsigned int)to_time_struct(_time).tm_mday;
    }
    
    void Date::set_day(unsigned int day) {
        tm time_struct = to_time_struct(_time);
        // Set day
        time_struct.tm_mday = static_cast<int>(day);
        
        _time = to_time(&time_struct);
    }
    
    unsigned int Date::get_month() const {
        return (unsigned int)to_time_struct(_time).tm_mon + 1;
    }
    
    void Date::set_month(unsigned int month) {
        tm time_struct = to_time_struct(_time);
        // Set month
        time_struct.tm_mon = static_cast<int>(month) - 1;
        
        _time = to_time(&time_struct);
    }
    
    unsigned int Date::get_year() const {
        return (unsigned int)to_time_struct(_time).tm_year + 1900;
    }
    
    void Date::set_year(unsigned int year) {
        tm time_struct = to_time_struct(_time);

        // Set year
        time_struct.tm_year = static_cast<int>(year) - 1900;
        
        _time = to_time(&time_struct);
    }
    
    /* private */
    
    system_clock::time_point Date::to_time(tm* time_struct) const {
        time_t time_utc = support::timegm(time_struct);

        // Convert to time point
        return system_clock::from_time_t(time_utc);
    }

    tm Date::to_time_struct(system_clock::time_point time) const {
        time_t time_raw = system_clock::to_time_t(time);
        
        // Convert to time struct
        return support::gmtime(&time_raw);
    }
    
}  // namespace support
