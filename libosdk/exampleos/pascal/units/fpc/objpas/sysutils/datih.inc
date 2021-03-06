{
    *********************************************************************
    Copyright (C) 1997, 1998 Gertjan Schouten

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    *********************************************************************

    System Utilities For Free Pascal
}


type
  PDayTable = ^TDayTable;
  TDayTable = array[1..12] of Word;

const
   HoursPerDay = 24;
   MinsPerHour = 60;
   SecsPerMin  = 60;
   MSecsPerSec = 1000;
   MinsPerDay  = HoursPerDay * MinsPerHour;
   SecsPerDay  = MinsPerDay * SecsPerMin;
   MSecsPerDay = SecsPerDay * MSecsPerSec;

   DateDelta = 693594;        // Days between 1/1/0001 and 12/31/1899
   UnixDateDelta = 25569;

   { True=Leapyear }
   MonthDays: array [Boolean] of TDayTable =
     ((31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31),
      (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31));

   TwoDigitYearCenturyWindow : word= 50;
                             { Threshold to be subtracted from year before
                               age-detection.}

   {  date time formatting characters:
      c      : shortdateformat + ' ' + shorttimeformat
      d      : day of month
      dd     : day of month (leading zero)
      ddd    : day of week (abbreviation)
      dddd   : day of week (full)
      ddddd  : shortdateformat
      dddddd : longdateformat
      m      : month
      mm     : month (leading zero)
      mmm    : month (abbreviation)
      mmmm   : month (full)
      y      : year (four digits)
      yy     : year (two digits)
      yyyy   : year (with century)
      h      : hour
      hh     : hour (leading zero)
      n      : minute
      nn     : minute (leading zero)
      s      : second
      ss     : second (leading zero)
      t      : shorttimeformat
      tt     : longtimeformat
      am/pm  : use 12 hour clock and display am and pm accordingly
                a/p    : use 12 hour clock and display a and p accordingly
      /      : insert date seperator
      :      : insert time seperator
      "xx"   : literal text
      'xx'   : literal text
   }

type
{$ifndef win32}
   { Win32 reuses the struct from the Windows unit }
   TSystemTime = record
      Year, Month, Day: word;
      Hour, Minute, Second, MilliSecond: word;
   end ;
{$endif win32}

   TTimeStamp = record
      Time: integer;   { Number of milliseconds since midnight }
      Date: integer;   { One plus number of days since 1/1/0001 }
   end ;


function DateTimeToTimeStamp(DateTime: TDateTime): TTimeStamp;
function TimeStampToDateTime(const TimeStamp: TTimeStamp): TDateTime;
function MSecsToTimeStamp(MSecs: Comp): TTimeStamp;
function TimeStampToMSecs(const TimeStamp: TTimeStamp): comp;
function TryEncodeDate(Year, Month, Day: Word; var Date: TDateTime): Boolean;
function TryEncodeTime(Hour, Min, Sec, MSec: Word; var Time: TDateTime): Boolean;
function EncodeDate(Year, Month, Day :word): TDateTime;
function EncodeTime(Hour, Minute, Second, MilliSecond:word): TDateTime;
function ComposeDateTime(Date,Time : TDateTime) : TDateTime;
procedure DecodeDate(Date: TDateTime; var Year, Month, Day: word);
function DecodeDateFully(const DateTime: TDateTime; var Year, Month, Day, DOW: Word): Boolean;
procedure DecodeTime(Time: TDateTime; var Hour, Minute, Second, MilliSecond: word);
procedure DateTimeToSystemTime(DateTime: TDateTime; var SystemTime: TSystemTime);
function SystemTimeToDateTime(const SystemTime: TSystemTime): TDateTime;
function DayOfWeek(DateTime: TDateTime): integer;
function Date: TDateTime;
function Time: TDateTime;
function Now: TDateTime;
function IncMonth(const DateTime: TDateTime; NumberOfMonths: integer = 1 ): TDateTime;
function IsLeapYear(Year: Word): boolean;
function DateToStr(Date: TDateTime): string;
function TimeToStr(Time: TDateTime): string;
function DateTimeToStr(DateTime: TDateTime): string;
function StrToDate(const S: string): TDateTime;
function StrToTime(const S: string): TDateTime;
function StrToDateTime(const S: string): TDateTime;
function FormatDateTime(FormatStr: string; DateTime: TDateTime):string;
procedure DateTimeToString(var Result: string; const FormatStr: string; const DateTime: TDateTime);
Function DateTimeToFileDate(DateTime : TDateTime) : Longint;
Function FileDateToDateTime (Filedate : Longint) :TDateTime;
function TryStrToDate(const S: string; out Value: TDateTime): Boolean;
// function TryStrToDate(const S: string; out Value: TDateTime; const FormatSettings: TFormatSettings): Boolean;
function TryStrToTime(const S: string; out Value: TDateTime): Boolean;
// function TryStrToTime(const S: string; out Value: TDateTime; const FormatSettings: TFormatSettings): Boolean;
function TryStrToDateTime(const S: string; out Value: TDateTime): Boolean;
// function TryStrToDateTime(const S: string; out Value: TDateTime; const FormatSettings: TFormatSettings): Boolean;
function CurrentYear:Word;
{ FPC Extra }
Procedure GetLocalTime(var SystemTime: TSystemTime);

