{

    This file is part of the Free Pascal Run time library.
    Copyright (c) 1999-2000 by the Free Pascal development team

    See the file COPYING.FPC, included in this distribution,
    For details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{****************************************************************************
                                Local types
****************************************************************************}

{
  TextRec and FileRec are put in a separate file to make it available to other
  units without putting it explicitly in systemh.
  This way we keep TP compatibility, and the TextRec definition is available
  for everyone who needs it.
}
{$i filerec.inc}
{$i textrec.inc}

Procedure HandleError (Errno : Longint); forward;
Procedure HandleErrorFrame (Errno : longint;frame : Pointer); forward;

type
  FileFunc = Procedure(var t : TextRec);


const
  STACK_MARGIN = 16384;    { Stack size margin for stack checking }
{ Random / Randomize constants }
  OldRandSeed : Cardinal = 0;

{ For Error Handling.}
  ErrorBase : Pointer = nil;

{ Used by the ansistrings and maybe also other things in the future }
var
  emptychar : char;public name 'FPC_EMPTYCHAR';
  initialstklen : SizeUint;external name '__stklen';
  initialstkptr : Pointer;external name '__stkptr';

{ checks whether the given suggested size for the stack of the current
 thread is acceptable. If this is the case, returns it unaltered.
 Otherwise it should return an acceptable value.

 Operating systems that automatically expand their stack on demand, should
 simply return a very large value.
 Operating systems which do not have a possibility to retrieve stack size
 information, should simply return the given stklen value (This is the default
 implementation).
}
function CheckInitialStkLen(stklen : SizeUInt) : SizeUInt; forward;

{****************************************************************************
                    Include processor specific routines
****************************************************************************}

{$ifdef FPC_USE_LIBC}
{ prefer libc implementations over our own, as they're most likely faster }
{$i cgeneric.inc}
{ is now declared as external reference to another routine in the interface }
{$i cgenstr.inc}
{$endif FPC_USE_LIBC}

{$ifdef cpui386}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i i386.inc}  { Case dependent, don't change }
{$endif cpui386}

{$ifdef cpum68k}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i m68k.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpum68k}

{$ifdef cpux86_64}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i x86_64.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpux86_64}

{$ifdef cpupowerpc}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i powerpc.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpupowerpc}

{$ifdef cpualpha}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i alpha.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpualpha}

{$ifdef cpuiA64}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i ia64.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpuiA64}

{$ifdef cpusparc}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i sparc.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpusparc}

{$ifdef cpuarm}
  {$ifdef SYSPROCDEFINED}
    {$Error Can't determine processor type !}
  {$endif}
  {$i arm.inc}  { Case dependent, don't change }
  {$define SYSPROCDEFINED}
{$endif cpuarm}

procedure fillchar(var x;count : SizeInt;value : boolean);{$ifdef SYSTEMINLINE}inline;{$endif}
begin
  fillchar(x,count,byte(value));
end;

procedure fillchar(var x;count : SizeInt;value : char);{$ifdef SYSTEMINLINE}inline;{$endif}
begin
  fillchar(x,count,byte(value));
end;

{ Include generic pascal only routines which are not defined in the processor
  specific include file }
{$I generic.inc}


{****************************************************************************
                                Set Handling
****************************************************************************}

{ Include set support which is processor specific}
{$i set.inc}
{ Include generic pascal routines for sets if the processor }
{ specific routines are not available.                      }
{$i genset.inc}


{****************************************************************************
                               Math Routines
****************************************************************************}

function Hi(b : byte): byte;{$ifdef SYSTEMINLINE}inline;{$endif}
begin
   Hi := b shr 4
end;

function Lo(b : byte): byte;{$ifdef SYSTEMINLINE}inline;{$endif}
begin
   Lo := b and $0f
end;

Function swap (X : Word) : Word;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  swap:=(X and $ff) shl 8 + (X shr 8)
End;

Function Swap (X : Integer) : Integer;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  swap:=(X and $ff) shl 8 + (X shr 8)
End;

Function swap (X : Longint) : Longint;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Swap:=(X and $ffff) shl 16 + (X shr 16)
End;

Function Swap (X : Cardinal) : Cardinal;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Swap:=(X and $ffff) shl 16 + (X shr 16)
End;

Function Swap (X : QWord) : QWord;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Swap:=(X and $ffffffff) shl 32 + (X shr 32);
End;

Function swap (X : Int64) : Int64;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Swap:=(X and $ffffffff) shl 32 + (X shr 32);
End;

{$ifdef SUPPORT_DOUBLE}
operator := (b:real48) d:double;
begin
 D:=real2double(b);
end;
{$endif SUPPORT_DOUBLE}

{$ifdef SUPPORT_EXTENDED}
operator := (b:real48) e:extended;
begin
 e:=real2double(b);
end;
{$endif SUPPORT_EXTENDED}

{$ifdef FPC_USE_LIBC}
{ Include libc versions }
{$i cgenmath.inc}
{$endif FPC_USE_LIBC}
{ Include processor specific routines }
{$I math.inc}
{ Include generic version }
{$I genmath.inc}


{****************************************************************************
                  Subroutines for String handling
****************************************************************************}

{ Needs to be before RTTI handling }

{$i sstrings.inc}

{ requires sstrings.inc for initval }
{$I int64p.inc}
{$I int64.inc}

{Requires int64.inc, since that contains the VAL functions for int64 and qword}
{$i astrings.inc}

{$i wstrings.inc}

{$i aliases.inc}

{*****************************************************************************
                        Dynamic Array support
*****************************************************************************}

{$i dynarr.inc}

{*****************************************************************************
                        Object Pascal support
*****************************************************************************}

{$i objpas.inc}

{*****************************************************************************
                            Variant support
*****************************************************************************}

{$i variant.inc}
{****************************************************************************
                         Run-Time Type Information (RTTI)
****************************************************************************}

{$i rtti.inc}



{----------------------------------------------------------------------
   Mersenne Twister: A 623-Dimensionally Equidistributed Uniform
   Pseudo-Random Number Generator.

   What is Mersenne Twister?
   Mersenne Twister(MT) is a pseudorandom number generator developped by
   Makoto Matsumoto and Takuji Nishimura (alphabetical order) during
   1996-1997. MT has the following merits:
   It is designed with consideration on the flaws of various existing
   generators.
   Far longer period and far higher order of equidistribution than any
   other implemented generators. (It is proved that the period is 2^19937-1,
   and 623-dimensional equidistribution property is assured.)
   Fast generation. (Although it depends on the system, it is reported that
   MT is sometimes faster than the standard ANSI-C library in a system
   with pipeline and cache memory.)
   Efficient use of the memory. (The implemented C-code mt19937.c
   consumes only 624 words of working area.)

   home page
     http://www.math.keio.ac.jp/~matumoto/emt.html
   original c source
     http://www.math.keio.ac.jp/~nisimura/random/int/mt19937int.c

   Coded by Takuji Nishimura, considering the suggestions by
   Topher Cooper and Marc Rieffel in July-Aug. 1997.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later
   version.
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Library General Public License for more details.
   You should have received a copy of the GNU Library General
   Public License along with this library; if not, write to the
   Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307  USA

   Copyright (C) 1997, 1999 Makoto Matsumoto and Takuji Nishimura.
   When you use this, send an email to: matumoto@math.keio.ac.jp
   with an appropriate reference to your work.

   REFERENCE
   M. Matsumoto and T. Nishimura,
   "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform
   Pseudo-Random Number Generator",
   ACM Transactions on Modeling and Computer Simulation,
   Vol. 8, No. 1, January 1998, pp 3--30.


  Translated to OP and Delphi interface added by Roman Krejci (6.12.1999)

  http://www.rksolution.cz/delphi/tips.htm

  Revised 21.6.2000: Bug in the function RandInt_MT19937 fixed

  2003/10/26: adapted to use the improved intialisation mentioned at
  <http://www.math.keio.ac.jp/~matumoto/MT2002/emt19937ar.html> and
  removed the assembler code

 ----------------------------------------------------------------------}

{$R-} {range checking off}
{$Q-} {overflow checking off}

{ Period parameter }
Const
  MT19937N=624;

Type
  tMT19937StateArray = array [0..MT19937N-1] of longint; // the array for the state vector

{ Period parameters }
const
  MT19937M=397;
  MT19937MATRIX_A  =$9908b0df;  // constant vector a
  MT19937UPPER_MASK=$80000000;  // most significant w-r bits
  MT19937LOWER_MASK=$7fffffff;  // least significant r bits

{ Tempering parameters }
  TEMPERING_MASK_B=$9d2c5680;
  TEMPERING_MASK_C=$efc60000;


VAR
  mt : tMT19937StateArray;

const
  mti: longint=MT19937N+1; // mti=MT19937N+1 means mt[] is not initialized

{ Initializing the array with a seed }
procedure sgenrand_MT19937(seed: longint);
var
  i: longint;
begin
  mt[0] := seed;
  for i := 1 to MT19937N-1 do
    begin
      mt[i] := 1812433253 * (mt[i-1] xor (mt[i-1] shr 30)) + i;
      { See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. }
      { In the previous versions, MSBs of the seed affect   }
      { only MSBs of the array mt[].                        }
      { 2002/01/09 modified by Makoto Matsumoto             }
    end;
  mti := MT19937N;
end;


function genrand_MT19937: longint;
const
  mag01 : array [0..1] of longint =(0, longint(MT19937MATRIX_A));
var
  y: longint;
  kk: longint;
begin
  if RandSeed<>OldRandSeed then
    mti:=MT19937N+1;
  if (mti >= MT19937N) { generate MT19937N longints at one time }
  then begin
     if mti = (MT19937N+1) then  // if sgenrand_MT19937() has not been called,
       begin
         sgenrand_MT19937(randseed);   // default initial seed is used
         { hack: randseed is not used more than once in this algorithm. Most }
         {  user changes are re-initialising reandseed with the value it had }
         {  at the start -> with the "not", we will detect this change.      }
         {  Detecting other changes is not useful, since the generated       }
         {  numbers will be different anyway.                                }
         randseed := not(randseed);
         oldrandseed := randseed;
       end;
     for kk:=0 to MT19937N-MT19937M-1 do begin
        y := (mt[kk] and MT19937UPPER_MASK) or (mt[kk+1] and MT19937LOWER_MASK);
        mt[kk] := mt[kk+MT19937M] xor (y shr 1) xor mag01[y and $00000001];
     end;
     for kk:= MT19937N-MT19937M to MT19937N-2 do begin
       y := (mt[kk] and MT19937UPPER_MASK) or (mt[kk+1] and MT19937LOWER_MASK);
       mt[kk] := mt[kk+(MT19937M-MT19937N)] xor (y shr 1) xor mag01[y and $00000001];
     end;
     y := (mt[MT19937N-1] and MT19937UPPER_MASK) or (mt[0] and MT19937LOWER_MASK);
     mt[MT19937N-1] := mt[MT19937M-1] xor (y shr 1) xor mag01[y and $00000001];
     mti := 0;
  end;
  y := mt[mti]; inc(mti);
  y := y xor (y shr 11);
  y := y xor (y shl 7)  and TEMPERING_MASK_B;
  y := y xor (y shl 15) and TEMPERING_MASK_C;
  y := y xor (y shr 18);
  Result := y;
end;


function random(l:longint): longint;
begin
  { otherwise we can return values = l (JM) }
  if (l < 0) then
    inc(l);
  random := longint((int64(cardinal(genrand_MT19937))*l) shr 32);
end;

function random(l:int64): int64;
begin
  { always call random, so the random generator cycles (TP-compatible) (JM) }
  random := int64((qword(cardinal(genrand_MT19937)) or ((qword(cardinal(genrand_MT19937)) shl 32))) and $7fffffffffffffff);
  if (l<>0) then
    random := random mod l
  else
    random := 0;
end;

function random: extended;
begin
  random := cardinal(genrand_MT19937) * (1.0/(int64(1) shl 32));
end;

{****************************************************************************
                            Memory Management
****************************************************************************}

Function Ptr(sel,off : Longint) : farpointer;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  ptr:=farpointer((sel shl 4)+off);
End;

Function CSeg : Word;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Cseg:=0;
End;

Function DSeg : Word;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Dseg:=0;
End;

Function SSeg : Word;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Sseg:=0;
End;


{*****************************************************************************
                             Directory support.
*****************************************************************************}

Procedure getdir(drivenr:byte;Var dir:ansistring);
{ this is needed to also allow ansistrings, the shortstring version is
  OS dependent }
var
  s : shortstring;
begin
  getdir(drivenr,s);
  dir:=s;
end;

{$ifopt R+}
{$define RangeCheckWasOn}
{$R-}
{$endif opt R+}

{$ifopt I+}
{$define IOCheckWasOn}
{$I-}
{$endif opt I+}

{$ifopt Q+}
{$define OverflowCheckWasOn}
{$Q-}
{$endif opt Q+}

{*****************************************************************************
                             Miscellaneous
*****************************************************************************}

procedure fpc_rangeerror;[public,alias:'FPC_RANGEERROR']; compilerproc;
begin
  HandleErrorFrame(201,get_frame);
end;

procedure fpc_divbyzero;[public,alias:'FPC_DIVBYZERO']; compilerproc;
begin
  HandleErrorFrame(200,get_frame);
end;

procedure fpc_overflow;[public,alias:'FPC_OVERFLOW']; compilerproc;
begin
  HandleErrorFrame(215,get_frame);
end;


procedure fpc_iocheck;[public,alias:'FPC_IOCHECK']; compilerproc;
var
  l : longint;
begin
  if InOutRes<>0 then
   begin
     l:=InOutRes;
     InOutRes:=0;
     HandleErrorFrame(l,get_frame);
   end;
end;


Function IOResult:Word;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  IOResult:=InOutRes;
  InOutRes:=0;
End;

Function GetThreadID:TThreadID;{$ifdef SYSTEMINLINE}inline;{$endif}
begin
(* ThreadID is stored in a threadvar and made available in interface *)
(* to allow setup of this value during thread initialization.        *)
  GetThreadID := ThreadID;
end;

{*****************************************************************************
                         Stack check code
*****************************************************************************}

{$IFNDEF NO_GENERIC_STACK_CHECK}

{$IFOPT S+}
{$DEFINE STACKCHECK}
{$ENDIF}
{$S-}
procedure fpc_stackcheck(stack_size:SizeUInt);[public,alias:'FPC_STACKCHECK'];
var
  c : Pointer;
begin
  { Avoid recursive calls when called from the exit routines }
  if StackError then
   exit;
  c := Sptr - (stack_size + STACK_MARGIN);
  if (c <= StackBottom) then
   begin
     StackError:=true;
     HandleError(202);
   end;
end;
{$IFDEF STACKCHECK}
{$S+}
{$ENDIF}
{$UNDEF STACKCHECK}

{$ENDIF NO_GENERIC_STACK_CHECK}

{*****************************************************************************
                        Initialization / Finalization
*****************************************************************************}

const
  maxunits=1024; { See also files.pas of the compiler source }
type
  TInitFinalRec=record
    InitProc,
    FinalProc : TProcedure;
  end;
  TInitFinalTable=record
    TableCount,
    InitCount  : longint;
    Procs      : array[1..maxunits] of TInitFinalRec;
  end;

var
  InitFinalTable : TInitFinalTable;external name 'INITFINAL';

procedure fpc_InitializeUnits;[public,alias:'FPC_INITIALIZEUNITS']; compilerproc;
var
  i : longint;
begin
  { call cpu/fpu initialisation routine }
  fpc_cpuinit;
  with InitFinalTable do
   begin
     for i:=1 to TableCount do
      begin
        if assigned(Procs[i].InitProc) then
         Procs[i].InitProc();
        InitCount:=i;
      end;
   end;
  if assigned(InitProc) then
    TProcedure(InitProc)();
end;


procedure FinalizeUnits;[public,alias:'FPC_FINALIZEUNITS'];
begin
  with InitFinalTable do
   begin
     while (InitCount>0) do
      begin
        // we've to decrement the cound before calling the final. code
        // else a halt in the final. code leads to a endless loop
        dec(InitCount);
        if assigned(Procs[InitCount+1].FinalProc) then
         Procs[InitCount+1].FinalProc();
      end;
   end;
end;

{*****************************************************************************
                          Error / Exit / ExitProc
*****************************************************************************}

Procedure system_exit;forward;

Procedure InternalExit;
var
  current_exit : Procedure;
Begin
  while exitProc<>nil Do
   Begin
     InOutRes:=0;
     current_exit:=tProcedure(exitProc);
     exitProc:=nil;
     current_exit();
   End;
  { Finalize units }
  FinalizeUnits;
  { Show runtime error and exit }
  If erroraddr<>nil Then
   Begin
     Writeln(stdout,'Runtime error ',Errorcode,' at $',hexstr(PtrInt(Erroraddr),sizeof(PtrInt)*2));
     { to get a nice symify }
     Writeln(stdout,BackTraceStrFunc(Erroraddr));
     dump_stack(stdout,ErrorBase);
     Writeln(stdout,'');
   End;
  { Make sure that all output is written to the redirected file }
  Flush(Output);
  Flush(ErrOutput);
  Flush(StdOut);
  Flush(StdErr);
End;


Procedure do_exit;[Public,Alias:'FPC_DO_EXIT'];
begin
  InternalExit;
  System_exit;
end;


Procedure lib_exit;[Public,Alias:'FPC_LIB_EXIT'];
begin
  InternalExit;
end;


Procedure Halt(ErrNum: Byte);
Begin
  ExitCode:=Errnum;
  Do_Exit;
end;


function SysBackTraceStr (Addr: Pointer): ShortString;
begin
  SysBackTraceStr:='  $'+HexStr(Ptrint(addr),sizeof(PtrInt)*2);
end;


Procedure HandleErrorAddrFrame (Errno : longint;addr,frame : Pointer);[public,alias:'FPC_BREAK_ERROR'];
begin
  If pointer(ErrorProc)<>Nil then
    ErrorProc(Errno,addr,frame);
  errorcode:=word(Errno);
  erroraddr:=addr;
  errorbase:=frame;
  if ExceptAddrStack <> nil then
    raise TObject(nil) at addr,frame;
  if errorcode <= maxExitCode then
    halt(errorcode)
  else
    halt(255)
end;

Procedure HandleErrorFrame (Errno : longint;frame : Pointer);
{
  Procedure to handle internal errors, i.e. not user-invoked errors
  Internal function should ALWAYS call HandleError instead of RunError.
  Can be used for exception handlers to specify the frame
}
begin
  HandleErrorAddrFrame(Errno,get_caller_addr(frame),get_caller_frame(frame));
end;


Procedure HandleError (Errno : longint);[public,alias : 'FPC_HANDLEERROR'];
{
  Procedure to handle internal errors, i.e. not user-invoked errors
  Internal function should ALWAYS call HandleError instead of RunError.
}
begin
  HandleErrorFrame(Errno,get_frame);
end;


procedure RunError(w : word);[alias: 'FPC_RUNERROR'];
begin
  errorcode:=w;
  erroraddr:=get_caller_addr(get_frame);
  errorbase:=get_caller_frame(get_frame);
  if errorcode <= maxExitCode then
    halt(errorcode)
  else
    halt(255)
end;


Procedure RunError;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  RunError (0);
End;


Procedure Halt;{$ifdef SYSTEMINLINE}inline;{$endif}
Begin
  Halt(0);
End;

Procedure Error(RunTimeError : TRunTimeError);

begin
  RunError(RuntimeErrorExitCodes[RunTimeError]);
end;


function do_isdevice(handle:thandle):boolean;forward;

Procedure dump_stack(var f : text;bp : Pointer);
var
  i : Longint;
  prevbp : Pointer;
  is_dev : boolean;
  caller_frame,
  caller_addr : Pointer;
Begin
  try
    prevbp:=bp-1;
    i:=0;
    is_dev:=do_isdevice(textrec(f).Handle);
    while bp > prevbp Do
     Begin
       caller_addr := get_caller_addr(bp);
       caller_frame := get_caller_frame(bp);
       if (caller_addr=nil) then
         break;
       Writeln(f,BackTraceStrFunc(caller_addr));
       if (caller_frame=nil) then
         break;
       Inc(i);
       If ((i>max_frame_dump) and is_dev) or (i>256) Then
         break;
       prevbp:=bp;
       bp:=caller_frame;
     End;
   except
     { prevent endless dump if an exception occured }
   end;
End;


Type
  PExitProcInfo = ^TExitProcInfo;
  TExitProcInfo = Record
    Next     : PExitProcInfo;
    SaveExit : Pointer;
    Proc     : TProcedure;
  End;
const
  ExitProcList: PExitProcInfo = nil;

Procedure DoExitProc;
var
  P    : PExitProcInfo;
  Proc : TProcedure;
Begin
  P:=ExitProcList;
  ExitProcList:=P^.Next;
  ExitProc:=P^.SaveExit;
  Proc:=P^.Proc;
  DisPose(P);
  Proc();
End;


Procedure AddExitProc(Proc: TProcedure);
var
  P : PExitProcInfo;
Begin
  New(P);
  P^.Next:=ExitProcList;
  P^.SaveExit:=ExitProc;
  P^.Proc:=Proc;
  ExitProcList:=P;
  ExitProc:=@DoExitProc;
End;

function ArrayStringToPPchar(const S:Array of AnsiString;reserveentries:Longint):ppchar; // const ?
// Extra allocate reserveentries pchar's at the beginning (default param=0 after 1.0.x ?)
// Note: for internal use by skilled programmers only
// if "s" goes out of scope in the parent procedure, the pointer is dangling.

var p   : ppchar;
    i   : LongInt;
begin
  if High(s)<Low(s) Then Exit(NIL);
  Getmem(p,sizeof(pchar)*(high(s)-low(s)+ReserveEntries+2));  // one more for NIL, one more
                                              // for cmd
  if p=nil then
    begin
      {$ifdef xunix}
      fpseterrno(ESysEnomem);
      {$endif}
      exit(NIL);
    end;
  for i:=low(s) to high(s) do
     p[i+Reserveentries]:=pchar(s[i]);
  p[high(s)+1+Reserveentries]:=nil;
  ArrayStringToPPchar:=p;
end;

Function StringToPPChar(Var S:AnsiString;ReserveEntries:integer):ppchar;
{
  Create a PPChar to structure of pchars which are the arguments specified
  in the string S. Especially usefull for creating an ArgV for Exec-calls
}

begin
  StringToPPChar:=StringToPPChar(PChar(S),ReserveEntries);
end;

Function StringToPPChar(S: PChar;ReserveEntries:integer):ppchar;

var
  i,nr  : longint;
  Buf : ^char;
  p   : ppchar;

begin
  buf:=s;
  nr:=1;
  while (buf^<>#0) do                   // count nr of args
   begin
     while (buf^ in [' ',#9,#10]) do    // Kill separators.
      inc(buf);
     inc(nr);
     if buf^='"' Then                   // quotes argument?
      begin
        inc(buf);
        while not (buf^ in [#0,'"']) do // then end of argument is end of string or next quote
         inc(buf);
        if buf^='"' then                // skip closing quote.
          inc(buf);
      end
     else
       begin                            // else std
         while not (buf^ in [' ',#0,#9,#10]) do
           inc(buf);
       end;
   end;
  getmem(p,(ReserveEntries+nr)*sizeof(pchar));
  StringToPPChar:=p;
  if p=nil then
   begin
     {$ifdef xunix}
     fpseterrno(ESysEnomem);
     {$endif}
     exit;
   end;
  for i:=1 to ReserveEntries do inc(p); // skip empty slots
  buf:=s;
  while (buf^<>#0) do
   begin
     while (buf^ in [' ',#9,#10]) do    // Kill separators.
      begin
       buf^:=#0;
       inc(buf);
      end;
     if buf^='"' Then                   // quotes argument?
      begin
        inc(buf);
        p^:=buf;
        inc(p);
        p^:=nil;
        while not (buf^ in [#0,'"']) do // then end of argument is end of string or next quote
         inc(buf);
        if buf^='"' then                // skip closing quote.
          begin
            buf^:=#0;
            inc(buf);
          end;
      end
     else
       begin
        p^:=buf;
        inc(p);
        p^:=nil;
         while not (buf^ in [' ',#0,#9,#10]) do
           inc(buf);
       end;
   end;
end;




{*****************************************************************************
                          Abstract/Assert support.
*****************************************************************************}

procedure fpc_AbstractErrorIntern;compilerproc;[public,alias : 'FPC_ABSTRACTERROR'];
begin
  If pointer(AbstractErrorProc)<>nil then
    AbstractErrorProc();
  HandleErrorFrame(211,get_frame);
end;


Procedure fpc_assert(Const Msg,FName:Shortstring;LineNo:Longint;ErrorAddr:Pointer); [Public,Alias : 'FPC_ASSERT']; compilerproc;
begin
  if pointer(AssertErrorProc)<>nil then
    AssertErrorProc(Msg,FName,LineNo,ErrorAddr)
  else
    HandleErrorFrame(227,get_frame);
end;


Procedure SysAssert(Const Msg,FName:Shortstring;LineNo:Longint;ErrorAddr:Pointer);
begin
  If msg='' then
    write(stderr,'Assertion failed')
  else
    write(stderr,msg);
  Writeln(stderr,' (',FName,', line ',LineNo,').');
  Writeln(stderr,'');
  Halt(227);
end;


{*****************************************************************************
                       SetJmp/LongJmp support.
*****************************************************************************}

{$i setjump.inc}


{$ifdef IOCheckWasOn}
{$I+}
{$endif}

{$ifdef RangeCheckWasOn}
{$R+}
{$endif}

{$ifdef OverflowCheckWasOn}
{$Q+}
{$endif}


{*****************************************************************************
                       OS dependent Helpers/Syscalls
*****************************************************************************}

{$i sysos.inc}


{*****************************************************************************
                               Heap
*****************************************************************************}

{$i sysheap.inc}

{$i heap.inc}

{*****************************************************************************
                          Thread support
*****************************************************************************}

{ Generic threadmanager }
{$i thread.inc}

{ Generic threadvar support }
{$i threadvr.inc}

{ OS Dependent implementation }
{$i systhrd.inc}


{*****************************************************************************
                            File Handling
*****************************************************************************}

{ OS dependent low level file functions }
{$i sysfile.inc}

{ Text file }
{$i text.inc}

{ Untyped file }
{$i file.inc}

{ Typed file }
{$i typefile.inc}


{*****************************************************************************
                            Directory Handling
*****************************************************************************}

{ OS dependent dir functions }
{$i sysdir.inc}

{*****************************************************************************
                            Resources support
*****************************************************************************}


{$ifndef HAS_RESOURCES}
{$i sysres.inc}
{$endif}

Function FindResource(ModuleHandle: HMODULE; ResourceName, ResourceType: AnsiString): TResourceHandle;

begin
  Result:=FindResource(ModuleHandle,PChar(ResourceName),PChar(ResourceType));
end;
