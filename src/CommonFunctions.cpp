#include <time.h>
#include "BlackbirdInclude.h"
////////////////////////////////////////////////////////////////////////////
/// \brief Returns boolean value indicating if year passed is a leap year
/// \note Only valid until ~4000 AD
///
/// \param year     [in] Integer year
/// \param calendar [in] enum int of calendar used
/// \return Boolean value indicating if year passed is a leap year

//bool IsLeapYear(const int year, const int calendar) {
//  bool leap = false;
//
//  // handle default CALENDAR_PROLEPTIC_GREGORIAN first for code efficiency
//  if (calendar == CALENDAR_PROLEPTIC_GREGORIAN) {
//    leap = (((year % 4 == 0) && (year % 100 != 0)) ||
//            (year % 400 == 0)); // valid until ~4000 AD:)
//    return leap;
//  }
//
//  if (calendar == CALENDAR_365_DAY) {
//    return false;
//  }
//  if (calendar == CALENDAR_366_DAY) {
//    return true;
//  }
//
//  // other calendars
//  if ((calendar == CALENDAR_JULIAN || calendar == CALENDAR_GREGORIAN) &&
//      (year % 4 == 0)) {
//    leap = true;
//    if ((calendar == CALENDAR_GREGORIAN) && (year % 100 == 0) &&
//        (year % 400 != 0) && (year > 1583)) {
//      leap = false;
//    }
//  }
//  return leap;
//}

///////////////////////////////////////////////////////////////////////////
/// \brief Fills time structure tt
/// \details Converts Julian decimal date to string and returns day of month,
/// month, and year in time_struct. If dec_date >365/366, then year is
/// incremented. Accounts for leap years
///
/// \param &model_time [in]  Time elapsed since start of simulation
/// \param start_date  [in]  double simulation start date (Julian date)
/// \param start_year  [in]  Integer simulation start year
/// \param calendar    [in]  enum int of calendar used
/// \param &tt         [out] Time structure to house date information
//
//void JulianConvert(double model_time, const double start_date,
//                   const int start_year, const int calendar, time_struct &tt) {
//  int leap(0);
//  std::string mon;
//  double sum, days, ddate;
//  double dday;
//  int dmonth, dyear;
//
//  // handles daily roundoff error, (e.g., t=4.999873->t=5.0)
//  if ((model_time - floor(model_time)) > (1 - TIME_CORRECTION)) {
//    model_time = floor(model_time + TIME_CORRECTION);
//  }
//  // handles hourly roundoff error (e.g., t=5.24999873->t=5.25)
//  if (model_time * HR_PER_DAY - floor(model_time * HR_PER_DAY) >
//      (1.0 - TIME_CORRECTION) * HR_PER_DAY) {
//    model_time =
//        floor(HR_PER_DAY * (model_time + TIME_CORRECTION)) / HR_PER_DAY;
//  }
//
//  double dec_date =
//      start_date +
//      model_time; // decimal date calculated from start_date,start year
//
//  dyear = start_year;
//  ddate = dec_date;
//
//  if (IsLeapYear(dyear, calendar)) {
//    leap = 1;
//  }
//  while (ddate >= (365 + leap)) // correct for years
//  {
//    ddate -= (365.0 + leap);
//    dyear++;
//    leap = 0;
//    if (IsLeapYear(dyear, calendar)) {
//      leap = 1;
//    }
//  }
//  // ddate is now decimal julian date from Jan 1 0:00:00 of current dyear
//
//  dmonth = 1;
//  days = 31;
//  sum = 31 - TIME_CORRECTION;
//  mon = "Jan";
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 28 + leap;
//    sum += days;
//    mon = "Feb";
//  } // Feb
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 31;
//    sum += days;
//    mon = "Mar";
//  } // Mar
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 30;
//    sum += days;
//    mon = "Apr";
//  } // Apr
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 31;
//    sum += days;
//    mon = "May";
//  } // May
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 30;
//    sum += days;
//    mon = "Jun";
//  } // Jun
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 31;
//    sum += days;
//    mon = "Jul";
//  } // Jul
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 31;
//    sum += days;
//    mon = "Aug";
//  } // Aug
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 30;
//    sum += days;
//    mon = "Sep";
//  } // Sep
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 31;
//    sum += days;
//    mon = "Oct";
//  } // Oct
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 30;
//    sum += days;
//    mon = "Nov";
//  } // Nov
//  if (ddate >= sum) {
//    dmonth += 1;
//    days = 31;
//    sum += days;
//    mon = "Dec";
//  } // Dec
//
//  dday = ddate - sum + days; // decimal days since 0:00 on first of month
//
//  tt.model_time = model_time;
//  tt.julian_day = ddate;
//  tt.day_of_month = (int)(ceil(dday + REAL_SMALL)); // real_small to handle dday=1.0
//  if (tt.day_of_month == 0) {
//    tt.day_of_month = 1;
//  }
//  tt.month = dmonth;
//  tt.year = dyear;
//
//  tt.day_changed = false;
//  if ((model_time <= PRETTY_SMALL) ||
//      (tt.julian_day - floor(tt.julian_day + TIME_CORRECTION) < 0.001)) {
//    tt.day_changed = true;
//  }
//
//  static char out[50];
//  sprintf(out, "%4.4d-%2.2i-%2.2d", dyear, tt.month,
//          tt.day_of_month); // 2006-02-28 (ISO Standard)
//
//  tt.date_string = std::string(out);
//  tt.leap_yr = IsLeapYear(tt.year, calendar);
//
//}

////////////////////////////////////////////////////////////////////////////
/// \brief Returns time-of-day string for decimal date (e.g., day=124.3-->"07:12",day=1.5  -->"12:00")
/// \param dec_date [in] Decimal date
/// \return String hours of day in 00:00:00.00 format
//
//std::string DecDaysToHours(const double dec_date, const bool truncate) {
//  double hours = (dec_date - floor(dec_date)) * 24.0;
//  double mind = (hours - floor(hours)) * 60.0;
//  double sec = (mind - floor(mind)) * 60.0;
//
//  int hr = (int)(floor(hours));
//  int min = (int)(floor(mind));
//
//  if (sec >= 59.995) {
//    min++;
//    sec = 0.0;
//  } // to account for case where time is rounded up to 60 sec
//  if (min == 60) {
//    hr++;
//    min = 0;
//  }
//  if (hr == 24) {
//    hr = 0;
//  }
//
//  static char out[12];
//  if (truncate) {
//    sprintf(out, "%2.2d:%2.2d:%2.2d", hr, min, (int)(sec));
//  } else {
//    sprintf(out, "%2.2d:%2.2d:%05.2f", hr, min, sec);
//  }
//  return std::string(out);
//}

////////////////////////////////////////////////////////////////////////////
/// \brief returns time struct corresponding to string in the following format
/// \param sDate    [in] date string in ISO standard format yyyy-mm-dd or yyyy/mm/dd
/// \param calendar [in] Enum int of calendar used \param sTime [in] time string in ISO standard format hh:mm:ss.00
/// \return Time structure equivalent of passed date and time
//
//time_struct DateStringToTimeStruct(const std::string sDate, std::string sTime,
//                                   const int calendar) {
//  static time_struct tt;
//  if (sDate.length() != (size_t)(10)) {
//    std::string errString = "DateStringToTimeStruct: Invalid date format used: " + sDate;
//    ExitGracefully(errString.c_str(), BAD_DATA);
//  }
//  if (sTime.length() < (size_t)(7)) {
//    std::string errString = "DateStringToTimeStruct: Invalid time format used (hourstamp): " + sTime;
//    ExitGracefully(errString.c_str(), BAD_DATA);
//  }
//
//  tt.date_string = sDate;
//  tt.year = s_to_i(sDate.substr(0, 4).c_str());
//  tt.month = s_to_i(sDate.substr(5, 2).c_str());
//  if (tt.month > 12) {
//    std::string errString = "DateStringToTimeStruct: Invalid time format used (month>12): " + sDate;
//    ExitGracefully(errString.c_str(), BAD_DATA);
//  }
//  tt.day_of_month = s_to_i(sDate.substr(8, 2).c_str());
//  tt.model_time = 0.0; // unspecified
//  tt.leap_yr = IsLeapYear(tt.year, calendar);
//  tt.julian_day = tt.day_of_month - 1;
//
//  if (tt.month>= 2){tt.julian_day+=31;}
//  if (tt.month>= 3){tt.julian_day+=28;}
//  if (tt.month>= 4){tt.julian_day+=31;}
//  if (tt.month>= 5){tt.julian_day+=30;}
//  if (tt.month>= 6){tt.julian_day+=31;}
//  if (tt.month>= 7){tt.julian_day+=30;}
//  if (tt.month>= 8){tt.julian_day+=31;}
//  if (tt.month>= 9){tt.julian_day+=31;}
//  if (tt.month>=10){tt.julian_day+=30;}
//  if (tt.month>=11){tt.julian_day+=31;}
//  if (tt.month==12){tt.julian_day+=30;}
//  if ((tt.leap_yr  ) && (tt.month> 2)){tt.julian_day+= 1;}
//
//  if (tt.day_of_month > DAYS_PER_MONTH[tt.month - 1]) {
//    ExitGracefully("DateStringToTimeStruct: Invalid time format used - "
//                   "exceeded max day of month",
//                   BAD_DATA);
//  }
//  if (tt.day_of_month <= 0) {
//    ExitGracefully("DateStringToTimeStruct: Invalid time format used - "
//                   "negative or zero day of month",
//                   BAD_DATA);
//  }
//
//  int hr, min;
//  double sec;
//
//  if (sTime.substr(1, 1) == ":") {
//    sTime = "0" + sTime;
//  } // for h:mm:ss.00 format to hh:mm:ss.00
//
//  ExitGracefullyIf((sTime.substr(2, 1) != ":"),
//                   "DateStringToTimeStruct: Invalid time format used",
//                   BAD_DATA);
//  ExitGracefullyIf((sTime.substr(5, 1) != ":"),
//                   "DateStringToTimeStruct: Invalid time format used",
//                   BAD_DATA);
//
//  hr = s_to_i(sTime.substr(0, 2).c_str());
//  min = s_to_i(sTime.substr(3, 2).c_str());
//  sec = s_to_d(sTime.substr(6, 6).c_str());
//  tt.julian_day += (double)(hr) / HR_PER_DAY;
//  tt.julian_day += (double)(min) / MIN_PER_DAY;
//  tt.julian_day += (double)(sec) / SEC_PER_DAY;
//
//  // Below reprocesses date string (optional)
//  JulianConvert(0.0, tt.julian_day, tt.year, calendar, tt);
//
//  return tt;
//}

////////////////////////////////////////////////////////////////////////////
/// \brief returns time zone string in format " +0700" or " -1200"
/// \param tz [in] time zone as integer - hours from GMT
/// \return time zone as string in format " +0600" or "" if tz is zero
//
//std::string TimeZoneToString(const int tz) {
//  std::string ends = "";
//  if (tz < 0) {
//    if (tz < -9) {
//      ends = " -" + to_string(-tz) + "00";
//    } else {
//      ends = " -0" + to_string(-tz) + "00";
//    }
//  } else if (tz > 0) {
//    if (tz > 9) {
//      ends = " +" + to_string(tz) + "00";
//    } else {
//      ends = " +0" + to_string(tz) + "00";
//    }
//  }
//  return ends;
//}

///////////////////////////////////////////////////////////////////
/// \brief calculates time difference, in days, between two specified dates
/// \details positive if day 2 is after day 1
/// \param jul_day1 [in] Julian day of date 1 (measured from Jan 1 of year @ 00:00:00)
/// \param year1    [in] year of date 1
/// \param jul_day2 [in] Julian day of date 2 (measured from Jan 1 of year @ 00:00:00)
/// \param year1    [in] year of date 2
/// \param calendar [in] enum int of calendar used
//
//double TimeDifference(const double jul_day1,const int year1,const double jul_day2,const int year2, const int calendar)
//{
//  int leap,yr;
//  double diff= jul_day2 - jul_day1;
//  yr=year2-1;
//  while (yr >= year1)
//  {
//    leap=0; if (IsLeapYear(yr,calendar)){ leap = 1; }
//    diff += (365+leap);
//    yr--;
//  }
//  yr=year2;
//  while (yr<year1)
//  {
//    leap=0; if (IsLeapYear(yr,calendar)){ leap = 1; }
//    diff -= (365+leap);
//    yr++;
//  }
//  return diff;
//}

///////////////////////////////////////////////////////////////////
/// \brief adds specified number of days to julian date and returns resultant julian date
/// \param jul_day1    [in]  Julian day of date 1 (measured from Jan 1 of year @ 00:00:00)
/// \param year1       [in]  year of date 1
/// \param daysadded   [in]  positive or negative number of days (can be fractional days) added to date 1
/// \param &Options    [in] Global model options information
/// \param jul_day_out [out] Julian day of output date (measured from Jan 1 of year @ 00:00:00)
/// \param year_out    [out] year of output date
//
//void AddTime(const double jul_day1,const int year1,const double &daysadded,const int calendar, double &jul_day_out,int &year_out)
//{
//  int    yr;
//  double leap;
//  double daysleft;
//
//  yr=year1;
//  jul_day_out=jul_day1;
//
//  if(daysadded>=0)
//  {
//    daysleft=daysadded;
//    do {
//      leap=0; if(IsLeapYear(yr,calendar)) { leap=1; }
//      if((jul_day_out+daysleft)<(365.0+leap)) {
//        jul_day_out+=daysleft;
//        year_out=yr;
//        break;
//      }
//      else {
//        yr++;
//        daysleft-=(365.0+leap-jul_day_out);
//        jul_day_out=0.0;
//      }
//      ExitGracefullyIf(daysleft<0.0,"Invalid input to AddTime routine (negative julian day?)",RUNTIME_ERR);
//    } while(true);
//  }
//  else
//  { //if daysadded<0
//    daysleft=-daysadded;
//    do {
//      if((jul_day_out-daysleft)>=0.0) { //99% of cases
//        jul_day_out-=daysleft;
//        year_out=yr;
//        return;
//      }
//      else {
//        yr--;
//        leap=0; if(IsLeapYear(yr,calendar)) { leap=1; }
//        daysleft-=jul_day_out;
//        if(daysleft<(365+leap)){ jul_day_out=(365+leap)-daysleft;year_out=yr;break; }
//        else                   { jul_day_out=0.0;daysleft-=(365+leap); }//skip whole year
//      }
//      ExitGracefullyIf(daysleft<0.0,"Invalid input to AddTime routine (negative julian day?)",RUNTIME_ERR);
//    } while(true);
//  }
//
//  // if calendar is STANDARD or GREGORIAN and the original time is before 4 October 1582
//  // while the final time is after, one has to add additional 10 days
//  // because in this calendar the day following 4 October 1582 is 15 October 1582 (there are 10 days missing)
//  // --> THIS is why people usually use the Proleptic Gregorian calendar :)
//  if ((calendar == CALENDAR_GREGORIAN) &&
//      ((year1 == 1582 && jul_day1 <= 277) || (year1 < 1582)) &&
//      ((year_out > 1582) || ((year_out == 1582) && (jul_day_out >= 278)))) {
//    double tmp_day;
//    int    tmp_yr;
//
//    tmp_yr  = year_out;
//    tmp_day = jul_day_out;
//
//    AddTime(tmp_day,tmp_yr,10.0,calendar,jul_day_out,year_out);
//    return;
//  }
//  return;
//
//}

///////////////////////////////////////////////////////////////////
/// \brief Parse chars of calendar and return calendar integer
/// \param cal_chars        [in]  String conatining calendar name, e.g., "PROLEPTIC_GREGORIAN"
/// \param StringToCalendar [out] enum integer representing calendar
//
//int StringToCalendar(std::string cal_chars)
//{
//  std::string str=StringToUppercase(cal_chars);
//  if (strcmp("STANDARD", str.c_str()) == 0) {
//    return CALENDAR_GREGORIAN;
//  } else if (strcmp("GREGORIAN", str.c_str()) == 0) {
//    return CALENDAR_GREGORIAN;
//  } else if (strcmp("PROLEPTIC_GREGORIAN", str.c_str()) == 0) {
//    return CALENDAR_PROLEPTIC_GREGORIAN;
//  } else if ((strcmp("NOLEAP", str.c_str()) == 0) || (strcmp("NO_LEAP", str.c_str()) == 0)) {
//    return CALENDAR_365_DAY;
//  } else if (strcmp("365_DAY", str.c_str()) == 0) {
//    return CALENDAR_365_DAY;
//  } else if (strcmp("360_DAY", str.c_str()) == 0) {
//    ExitGracefully("CommonFunctions: StringToCalendar: Blackbird does not support 360_DAY calendars!", BAD_DATA);
//    return CALENDAR_360_DAY;
//  } else if (strcmp("JULIAN", str.c_str()) == 0) {
//    return CALENDAR_JULIAN;
//  } else if (strcmp("ALL_LEAP", str.c_str()) == 0) {
//    return CALENDAR_366_DAY;
//  } else if (strcmp("366_DAY", str.c_str()) == 0) {
//    return CALENDAR_366_DAY;
//  } else {
//    printf("Calendar used: %s", str.c_str());
//    ExitGracefully("CommonFunctions: StringToCalendar: Unknown calendar specified!", BAD_DATA);
//  }
//  return -1;  // just to avoid compiler warning of void function return
//}

///////////////////////////////////////////////////////////////////////////
/// \brief Get the current system date/time
/// \return "now" as an ISO formatted string
//std::string GetCurrentMachineTime(void) {
//  // Get the current wall clock time
//  time_t now;
//  time(&now);
//  struct tm *curTime = localtime(&now);
//
//  // generate the ISO string
//  char s[20];
//  sprintf(s, "%4i-%02i-%02i %02i:%02i:%02i", curTime->tm_year + 1900,
//          curTime->tm_mon + 1, curTime->tm_mday, curTime->tm_hour,
//          curTime->tm_min, curTime->tm_sec);
//
//  return std::string(s);
//}

///////////////////////////////////////////////////////////////////////////
/// \brief Round the timestep to the nearest fractional day
/// \return improved timestep
//double FixTimestep(double tstep) {
//  double tmp = round(1.0 / tstep);
//  ExitGracefullyIf(fabs(tstep * tmp - 1.0) > 0.1,
//                   "CommonFunctions::FixTimestep: timesteps and time intervals "
//                   "must evenly divide into one day",
//                   BAD_DATA);
//  return 1.0 / tmp;
//}

////////////////////////////////////////////////////// /////////////////////
/// \brief True if string is proper iso date (e.g., yyyy-mm-dd or yyyy/mm/dd)
/// \return true if valid date string
//
//bool IsValidDateString(const std::string sDate) {
//  return ((sDate.length() == 10) &&
//          ((sDate.substr(4, 1) == "/") || (sDate.substr(4, 1) == "-")) &&
//          ((sDate.substr(7, 1) == "/") || (sDate.substr(7, 1) == "-")));
//}

////////////////////////////////////////////////////////////////////////////
/// \brief Rounds time to nearest minute to prevent roundoff error in NetCDF reporting of time
/// \param &t [in] model time, in hours
/// \return time, in hours, rounded to nearest minute
//
//double RoundToNearestMinute(const double &t) {
//  const double MIN_PER_HOUR = 60;
//  return floor(t + TIME_CORRECTION) +
//         (round((t - floor(t + TIME_CORRECTION)) * MIN_PER_HOUR)) /
//             MIN_PER_HOUR;
//}

////////////////////////////////////////////////////////////////////////////
/// \brief returns true if julian date is between two julian days (days inclusive)
/// \param julian_day   [in] julian date from 0.0 to 365.0
/// \param julian_start [in] integer start day of date range (0=Jan 1, 364=Dec 31 in non-leap)
/// \param julian_end [in] integer end day of date range (0=Jan 1, 364=Dec 31 in non-leap)
//
//bool IsInDateRange(const double &julian_day, const int &julian_start,
//                   const int &julian_end) {
//  if (julian_start < julian_end) {
//    return ((julian_day >= julian_start) && (julian_day <= julian_end));
//  } else {
//    return ((julian_day >= julian_start) || (julian_day <= julian_end)); // wraps around Dec 31-Jan 1
//  }
//}

////////////////////////////////////////////////////////////////////////////
/// \brief returns zero-indexed julian date (0..364(5)) given string in format Mmm-dd, mm-dd, or Julian date
/// \param date_str [in] date string ('Apr-23', '04-23', or '113')
/// \param calendar   [in] enumerated calendar type
/// \return zero indexed julian date
//
//int GetJulianDayFromMonthYear   (const std::string &date_str, const int calendar)
//{
//  if((date_str.length()==3) && (date_str.substr(1,1)=="-"))//support for m-d format (4-2)
//  {
//    std::string fulldate_str="1999-0"+date_str.substr(0,1)+"-0"+date_str.substr(2,1); //no leap year
//    time_struct tt = DateStringToTimeStruct(fulldate_str, "00:00:00", calendar);
//    return (int)(tt.julian_day);
//  }
//  else if((date_str.length()>=2) && (date_str.substr(1,1)=="-"))//support for m-dd format (4-23)
//  {
//    std::string fulldate_str="1999-0"+date_str; //no leap year
//    time_struct tt = DateStringToTimeStruct(fulldate_str, "00:00:00", calendar);
//    return (int)(tt.julian_day);
//  }
//  else if((date_str.length()==4) && (date_str.substr(2,1)=="-"))//support for mm-d format (10-2)
//  {
//    std::string fulldate_str="1999-"+date_str.substr(0, 3) + "0" + date_str.substr(3, 1); //no leap year
//    time_struct tt = DateStringToTimeStruct(fulldate_str, "00:00:00", calendar);
//    return (int)(tt.julian_day);
//  }
//  else if((date_str.length()==5) && (date_str.substr(2,1)=="-"))//support for mm-dd format (04-23)
//  {
//    std::string fulldate_str="1999-"+date_str; //no leap year
//    time_struct tt = DateStringToTimeStruct(fulldate_str, "00:00:00", calendar);
//    return (int)(tt.julian_day);
//  }
//  else if ((date_str.length()>=3) && (date_str.substr(3,1)=="-"))//support for Mmm-dd & Mmm-d format (Apr-23, Apr-02, or Apr-2)
//  {
//    std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//    std::string fulldate_str="";
//    for (int mon=0;mon<12;mon++){
//      if (date_str.substr(0,3)==months[mon]){
//        if (mon<9){fulldate_str="0"; }
//        if (date_str.length()<=5){ //Apr-1
//          fulldate_str=fulldate_str+to_string(mon+1)+"-0"+date_str.substr(4, 1);
//        }
//        else{ //Apr-01 or Apr-23
//          fulldate_str=fulldate_str+to_string(mon+1)+"-"+date_str.substr(4, 2);
//        }
//      }
//    }
//    if (fulldate_str == "") {
//      ExitGracefully("GetJulianDayFromMonthYear: invalild month indicated in Mmm-dd date format",BAD_DATA);
//    }
//    fulldate_str="1999-"+fulldate_str;
//    time_struct tt = DateStringToTimeStruct(fulldate_str, "00:00:00", calendar);
//    return (int)(tt.julian_day);
//  }
//  else{ //julian date format
//    int val=s_to_i(date_str.c_str());
//    if ((val==0) || (val>365)){
//      ExitGracefully("GetJulianDayFromMonthYear: invalild format of julian date. Should be in 'Mmm-dd', 'mm-dd', or supplied as julian date 1..365",BAD_DATA);
//    }
//    return s_to_i(date_str.c_str())-1; //convert from integer julian days to Blackbird 0-indexed julian days
//  }
//}

////////////////////////////////////////////////////////////////////////////
/// \brief returns time struct corresponding to string in the following format
/// \param unit_t_str [in] full time string from NetCDF file (e.g., '[days/minutes/hours] since YYYY-MM-dd 00:00:00{+0000}' or '[days/minutes/hours] since YYYY-MM-dd' )
/// \return true if string is valid
//
//bool IsValidNetCDFTimeString(const std::string time_string)
//{
//  int att_len=(int)strlen(time_string.c_str());
//  bool isvalid = true;
//  if (att_len<15) {return false;}
//
//  size_t pos=time_string.find("since",0);
//  if(pos==std::string::npos) { return false; } //no "since" in string
//
//  pos+=6;
//  std::string date_string=time_string.substr(pos,10);
//
//  if (!strstr(date_string.substr(4,1).c_str(),"-")){isvalid=false;}//properly located dashes in date string
//  if (!strstr(date_string.substr(7,1).c_str(),"-")){isvalid=false;}
//
//  if(time_string.length()<(pos+19)) { return isvalid; } //no time stamp
//
//  std::string hr_string  =time_string.substr(pos+11,8);
//  //cout<<"TIME STRING: "<<time_string<<" "<<pos<<" "<<date_string<<" "<<hr_string<<endl;
//
//  if(!strstr(hr_string.substr(2,1).c_str(),":")) { isvalid=false; }//properly located dashes in date string
//  if(!strstr(hr_string.substr(5,1).c_str(),":")) { isvalid=false; }
//
//  return isvalid;
//}

////////////////////////////////////////////////////////////////////////////
/// \brief returns time struct corresponding to string in the following format
/// \param unit_t_str [in] full time string from NetCDF file (e.g., 'days since YYYY-MM-dd 00:00:00+0000')
/// \param timestr    [in] first word of string (e.g., 'days')
/// \param calendar   [in] enumerated calendar type
/// \param timezone   [out] time shift from GMT, in days
/// \return Blackbird Time structure equivalent of passed date and time, time shift if applicable
//
//time_struct TimeStructFromNetCDFString(const std::string unit_t_str,const std::string timestr,const int calendar,double &timezone)
//{
//  std::string dash,colon,tmp;
//  tmp=unit_t_str;
//  timezone=0.0;
//  int start=(int)strlen(timestr.c_str());
//  start+=7; //first char of year YYYY (7=length(' since '))
//  // ---------------------------
//  // check if format is hours since YYYY-MM-DD HH:MM:SS, fill with leading zeros if necessary
//  // blank between date and time (position 10) can be either ' ' or 'T'
//  // Y  Y  Y  Y  -  M  M  -  d  d  _  0  0  :  0  0  :  0  0  .  0     +  0  0  0  0
//  // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
//  // ---------------------------
//  dash = tmp.substr(start+4,1);  // first dash in date
//
//  if(!strstr(dash.c_str(),"-"))
//  {
//    printf("time unit string: %s\n",tmp.c_str());
//    ExitGracefully("CommonFunctions:TimeStructFromNetCDFString: time unit string has weird format!",BAD_DATA);
//  }
//  if(!strstr(tmp.substr(start+7 ,1).c_str(),"-")) { tmp.insert(start+5,"0"); } // second dash in date - fixes YYYY-M-dd
//
//  bool date_only=false;
//  if ((int)(strlen(tmp.c_str()))<=(start+10+2)){date_only=true;} //00:00:00 +0000 not included in string
//
//  if(!date_only) {
//    if(!strstr(tmp.substr(start+10,1).c_str()," ") && !strstr(tmp.substr(start+10,1).c_str(),"T")) { tmp.insert(start+8,"0"); } // second dash in date - fixes YYYY-MM-d
//    if( strstr(tmp.substr(start+10,1).c_str(),"T")) {tmp.replace(start+10,1," "); } // replacing 'T' with ' ' in case date looks like YYYY-MM-DDTHH:MM:SS
//    if(!strstr(tmp.substr(start+13,1).c_str(),":")) { tmp.insert(start+11,"0"); } // first colon in time  - fixes 1:00:00
//    if(!strstr(tmp.substr(start+16,1).c_str(),":")) { tmp.insert(start+14,"0"); } // second colon in time - fixes 11:0:00 (?)
//  }
//  else {
//    if(strlen(tmp.c_str())==(size_t)(start+9)) { tmp.insert(start+8,"0"); } // second dash in date - fixes YYYY-MM-d
//  }
//  std::string sTime,sDate;
//  sDate = tmp.substr(start,10); //YYYY-MM-DD
//  if(!date_only) { sTime = tmp.substr(start+11,8); }  //HH:MM:SS
//  else           { sTime = "00:00:00";             }  // assumes start of day if no timestamp given
//  //cout<<"sTime"<<sTime<<" sDate"<< sDate<<" "<<unit_t_str<<" "<<date_only<<" "<<tmp<<endl;
//
//  timezone=0;
//  if((strlen(tmp.c_str())-start)==(size_t)(26)) {
//    if(!strcmp(tmp.substr(start+22,1).c_str(),"+")) {
//      timezone=(double)(s_to_i(tmp.substr(start+23,4).c_str()))/HR_PER_DAY/100; //time zone, in days
//    }
//    else if(!strcmp(tmp.substr(start+22,1).c_str(),"-")) {
//      timezone=-(double)(s_to_i(tmp.substr(start+23,4).c_str()))/HR_PER_DAY/100; //time zone, in days
//    }
//  }
//  return DateStringToTimeStruct(sDate,sTime,calendar);
//}

//////////////////////////////////////////////////////////////////
/// \brief Converts any lowercase characters in a string to uppercase, returning the converted string
/// \param &s [in] String to be converted to uppercase
/// \return &s converted to uppercase
//
std::string StringToUppercase(const std::string& s)
{
  std::string ret(s.size(), char());
  for (int i = 0; i < (int)(s.size()); ++i)
  {
    if ((s[i] <= 'z' && s[i] >= 'a')) { ret[i] = s[i] - ('a' - 'A'); }
    else { ret[i] = s[i]; }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////
/// \brief Simple and fast atof (ascii to float) function.
/// \notes Executes about 5x faster than standard MSCRT library atof().
/// \notes ported 09-May-2009 from Tom Van Baak (tvb) www.LeapSecond.com
//

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

double fast_s_to_d(const char* p)
{
  int frac;
  double sign, value, scale;

  // Skip leading white space, if any.

  while (white_space(*p)) {
    p += 1;
  }

  // Get sign, if any.

  sign = 1.0;
  if (*p == '-') {
    sign = -1.0;
    p += 1;

  }
  else if (*p == '+') {
    p += 1;
  }

  // Get digits before decimal point or exponent, if any.

  for (value = 0.0; valid_digit(*p); p += 1) {
    value = value * 10.0 + (*p - '0');
  }

  // Get digits after decimal point, if any.

  if (*p == '.') {
    double pow10 = 10.0;
    p += 1;
    while (valid_digit(*p)) {
      value += (*p - '0') / pow10;
      pow10 *= 10.0;
      p += 1;
    }
  }

  // Handle exponent, if any.

  frac = 0;
  scale = 1.0;
  if ((*p == 'e') || (*p == 'E')) {
    unsigned int expon;

    // Get sign of exponent, if any.

    p += 1;
    if (*p == '-') {
      frac = 1;
      p += 1;

    }
    else if (*p == '+') {
      p += 1;
    }

    // Get digits of exponent, if any.

    for (expon = 0; valid_digit(*p); p += 1) {
      expon = expon * 10 + (*p - '0');
    }
    if (expon > 308) expon = 308;

    // Calculate scaling factor.

    while (expon >= 50) { scale *= 1E50; expon -= 50; }
    while (expon >= 8) { scale *= 1E8;  expon -= 8; }
    while (expon > 0) { scale *= 10.0; expon -= 1; }
  }

  // Return signed and scaled floating point result.

  return sign * (frac ? (value / scale) : (value * scale));
}

//////////////////////////////////////////////////////////////////
/// \brief returns true if line is empty, begins with '#' or '*'
/// \param &s [in] first string token in file line
/// \param Len length of line
/// \return true if line is empty or a comment
//
bool IsComment(const char* s, const int Len)
{
  if ((Len == 0) || (s[0] == '#') || (s[0] == '*')) { return true; }
  return false;
}

/////////////////////////////////////////////////////////////////
/// \brief writes warning to screen and to Blackbird_errors.txt file
/// \param warn [in] warning message printed
//
void WriteWarning(const std::string warn, bool noisy)
{
  if (!g_suppress_warnings) {
    std::ofstream WARNINGS;
    WARNINGS.open((g_output_directory + "Blackbird_errors.txt").c_str(), std::ios::app);
    if (noisy) { std::cout << "WARNING!: " << warn << std::endl; }
    WARNINGS << "WARNING : " << warn << std::endl;
    WARNINGS.close();
  }
}

/////////////////////////////////////////////////////////////////
/// \brief writes advisory to screen and to Blackbird_errors.txt file
/// \param warn [in] warning message printed
//
void WriteAdvisory(const std::string warn, bool noisy)
{
  if (!g_suppress_warnings) {
    std::ofstream WARNINGS;
    WARNINGS.open((g_output_directory + "Blackbird_errors.txt").c_str(), std::ios::app);
    if (noisy) { std::cout << "ADVISORY: " << warn << std::endl; }
    WARNINGS << "ADVISORY : " << warn << std::endl;
    WARNINGS.close();
  }
}

///////////////////////////////////////////////////////////////////
/// \brief NetCDF error handling
/// \return Error string and NetCDF exit code
//
void HandleNetCDFErrors(int error_code) {

#ifdef _BBNETCDF_
  if (error_code == 0) {
    return;
  } else {
    std::string warn;
    warn = "NetCDF error [" + to_string(nc_strerror(error_code)) + "] occured.";
    ExitGracefully(warn.c_str(), BAD_DATA);
  }
#endif
}

/////////////////////////////////////////////////////////////////
/// \brief custom cpl error handler that does nothing
//
void SilentErrorHandler(CPLErr eErrClass, int err_no, const char *msg) {}