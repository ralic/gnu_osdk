Var ChPos,OldPos,ArgPos,DoArg,Len : SizeInt;
    Hs,ToAdd : TFormatString;
    Index : SizeInt;
    Width,Prec : Longint;
    Left : Boolean;
    Fchar : char;
    vq : qword;

  {
    ReadFormat reads the format string. It returns the type character in
    uppercase, and sets index, Width, Prec to their correct values,
    or -1 if not set. It sets Left to true if left alignment was requested.
    In case of an error, DoFormatError is called.
  }

  Function ReadFormat : Char;

  Var Value : longint;

    Procedure ReadInteger;

var Code: word;

    begin
      If Value<>-1 then exit; // Was already read.
      OldPos:=chPos;
      While (Chpos<=Len) and
            (Pos(Fmt[chpos],'1234567890')<>0) do inc(chpos);
      If Chpos>len then
        DoFormatError(feInvalidFormat);
      If Fmt[Chpos]='*' then
        begin
        If (Chpos>OldPos) or (ArgPos>High(Args))
           or (Args[ArgPos].Vtype<>vtInteger) then
          DoFormatError(feInvalidFormat);
        Value:=Args[ArgPos].VInteger;
        Inc(ArgPos);
        Inc(chPos);
        end
      else
        begin
        If (OldPos<chPos) Then
          begin
          Val (Copy(Fmt,OldPos,ChPos-OldPos),value,code);
          // This should never happen !!
          If Code>0 then DoFormatError (feInvalidFormat);
          end
        else
          Value:=-1;
        end;
    end;

    Procedure ReadIndex;

    begin
      ReadInteger;
      If Fmt[ChPos]=':' then
        begin
        If Value=-1 then DoFormatError(feMissingArgument);
        Index:=Value;
        Value:=-1;
        Inc(Chpos);
        end;
{$ifdef fmtdebug}
      Log ('Read index');
{$endif}
    end;

    Procedure ReadLeft;

    begin
      If Fmt[chpos]='-' then
        begin
        left:=True;
        Inc(chpos);
        end
      else
        Left:=False;
{$ifdef fmtdebug}
      Log ('Read Left');
{$endif}
    end;

    Procedure ReadWidth;

    begin
      ReadInteger;
      If Value<>-1 then
        begin
        Width:=Value;
        Value:=-1;
        end;
{$ifdef fmtdebug}
      Log ('Read width');
{$endif}
    end;

    Procedure ReadPrec;

    begin
      If Fmt[chpos]='.' then
        begin
        inc(chpos);
        ReadInteger;
        If Value=-1 then
         Value:=0;
        prec:=Value;
        end;
{$ifdef fmtdebug}
      Log ('Read precision');
{$endif}
    end;

{$ifdef INWIDEFORMAT}
  var
    FormatChar : TFormatChar;
{$endif INWIDEFORMAT}

  begin
{$ifdef fmtdebug}
    Log ('Start format');
{$endif}
    Index:=-1;
    Width:=-1;
    Prec:=-1;
    Value:=-1;
    inc(chpos);
    If Fmt[Chpos]='%' then
      begin
        Result:='%';
        exit;                           // VP fix
      end;
    ReadIndex;
    ReadLeft;
    ReadWidth;
    ReadPrec;
{$ifdef INWIDEFORMAT}
    FormatChar:=UpCase(Fmt[ChPos])[1];
    if word(FormatChar)>255 then
      ReadFormat:=#255
    else
      ReadFormat:=FormatChar;
{$else INWIDEFORMAT}
    ReadFormat:=Upcase(Fmt[ChPos]);
{$endif INWIDEFORMAT}
{$ifdef fmtdebug}
    Log ('End format');
{$endif}
end;


{$ifdef fmtdebug}
Procedure DumpFormat (C : char);
begin
  Write ('Fmt : ',fmt:10);
  Write (' Index : ',Index:3);
  Write (' Left  : ',left:5);
  Write (' Width : ',Width:3);
  Write (' Prec  : ',prec:3);
  Writeln (' Type  : ',C);
end;
{$endif}


function Checkarg (AT : SizeInt;err:boolean):boolean;
{
  Check if argument INDEX is of correct type (AT)
  If Index=-1, ArgPos is used, and argpos is augmented with 1
  DoArg is set to the argument that must be used.
}
begin
  result:=false;
  if Index=-1 then
    DoArg:=Argpos
  else
    DoArg:=Index;
  ArgPos:=DoArg+1;
  If (Doarg>High(Args)) or (Args[Doarg].Vtype<>AT) then
   begin
     if err then
      DoFormatError(feInvalidArgindex);
     dec(ArgPos);
     exit;
   end;
  result:=true;
end;

Const Zero = '000000000000000000000000000000000000000000000000000000000000000';

begin
  Result:='';
  Len:=Length(Fmt);
  Chpos:=1;
  OldPos:=1;
  ArgPos:=0;
  While chpos<=len do
    begin
    While (ChPos<=Len) and (Fmt[chpos]<>'%') do
      inc(chpos);
    If ChPos>OldPos Then
      Result:=Result+Copy(Fmt,OldPos,Chpos-Oldpos);
    If ChPos<Len then
      begin
      FChar:=ReadFormat;
{$ifdef fmtdebug}
      DumpFormat(FCHar);
{$endif}
      Case FChar of
        'D' : begin
              if Checkarg(vtinteger,false) then
                Str(Args[Doarg].VInteger,ToAdd)
              else if CheckArg(vtInt64,false) then
                Str(Args[DoArg].VInt64^,toadd)
              else if CheckArg(vtQWord,true) then
                Str(int64(Args[DoArg].VQWord^),toadd);
              Width:=Abs(width);
              Index:=Prec-Length(ToAdd);
              If ToAdd[1]<>'-' then
                ToAdd:=StringOfChar('0',Index)+ToAdd
              else
                // + 1 to accomodate for - sign in length !!
                Insert(StringOfChar('0',Index+1),toadd,2);
              end;
        'U' : begin
              if Checkarg(vtinteger,false) then
                Str(cardinal(Args[Doarg].VInteger),ToAdd)
              else if CheckArg(vtInt64,false) then
                Str(qword(Args[DoArg].VInt64^),toadd)
              else if CheckArg(vtQWord,true) then
                Str(Args[DoArg].VQWord^,toadd);
              Width:=Abs(width);
              Index:=Prec-Length(ToAdd);
              ToAdd:=StringOfChar('0',Index)+ToAdd
              end;
        'E' : begin
              if CheckArg(vtCurrency,false) then
                ToAdd:=FloatToStrF(Args[doarg].VCurrency^,ffexponent,Prec,3)
              else if CheckArg(vtExtended,true) then
                ToAdd:=FloatToStrF(Args[doarg].VExtended^,ffexponent,Prec,3);
              end;
        'F' : begin
              if CheckArg(vtCurrency,false) then
                ToAdd:=FloatToStrF(Args[doarg].VCurrency^,ffFixed,9999,Prec)
              else if CheckArg(vtExtended,true) then
                ToAdd:=FloatToStrF(Args[doarg].VExtended^,ffFixed,9999,Prec);
              end;
        'G' : begin
              if CheckArg(vtCurrency,false) then
                ToAdd:=FloatToStrF(Args[doarg].VCurrency^,ffGeneral,Prec,3)
              else if CheckArg(vtExtended,true) then
                ToAdd:=FloatToStrF(Args[doarg].VExtended^,ffGeneral,Prec,3);
              end;
        'N' : begin
              if CheckArg(vtCurrency,false) then
                ToAdd:=FloatToStrF(Args[doarg].VCurrency^,ffNumber,9999,Prec)
              else if CheckArg(vtExtended,true) then
                ToAdd:=FloatToStrF(Args[doarg].VExtended^,ffNumber,9999,Prec);
              end;
        'M' : begin
              if CheckArg(vtExtended,false) then
                ToAdd:=FloatToStrF(Args[doarg].VExtended^,ffCurrency,9999,Prec)
              else if CheckArg(vtCurrency,true) then
                ToAdd:=FloatToStrF(Args[doarg].VCurrency^,ffCurrency,9999,Prec);
              end;
        'S' : begin
                if CheckArg(vtString,false) then
                  hs:=Args[doarg].VString^
                else
                  if CheckArg(vtChar,false) then
                    hs:=Args[doarg].VChar
                else
                  if CheckArg(vtPChar,false) then
                    hs:=Args[doarg].VPChar
                else
                  if CheckArg(vtPWideChar,false) then
                    hs:=WideString(Args[doarg].VPWideChar)
                else
                  if CheckArg(vtWideChar,false) then
                    hs:=WideString(Args[doarg].VWideChar)
                else
                  if CheckArg(vtWidestring,false) then
                    hs:=WideString(Args[doarg].VWideString)
                else
                  if CheckArg(vtAnsiString,true) then
                    hs:=ansistring(Args[doarg].VAnsiString);
                Index:=Length(hs);
                If (Prec<>-1) and (Index>Prec) then
                  Index:=Prec;
                ToAdd:=Copy(hs,1,Index);
              end;
        'P' : Begin
              CheckArg(vtpointer,true);
              ToAdd:=HexStr(ptrint(Args[DoArg].VPointer),sizeof(Ptrint)*2);
              // Insert ':'. Is this needed in 32 bit ? No it isn't.
              // Insert(':',ToAdd,5);
              end;
        'X' : begin
              if Checkarg(vtinteger,false) then
                 begin
                   vq:=Cardinal(Args[Doarg].VInteger);
                   index:=16;
                 end
              else
                 if CheckArg(vtQWord, false) then
                   begin
                     vq:=Qword(Args[DoArg].VQWord^);
                     index:=31;
                   end
              else
                 begin
                   CheckArg(vtInt64,true);
                   vq:=Qword(Args[DoArg].VInt64^);
                   index:=31;
                 end;
              If Prec>index then
                ToAdd:=HexStr(vq,index)
              else
                begin
                // determine minimum needed number of hex digits.
                Index:=1;
                While (qWord(1) shl (Index*4)<=vq) and (index<16) do
                  inc(Index);
                If Index>Prec then
                  Prec:=Index;
                ToAdd:=HexStr(vq,Prec);
                end;
              end;
        '%': ToAdd:='%';
      end;
      If Width<>-1 then
        If Length(ToAdd)<Width then
          If not Left then
            ToAdd:=Space(Width-Length(ToAdd))+ToAdd
          else
            ToAdd:=ToAdd+space(Width-Length(ToAdd));
      Result:=Result+ToAdd;
      end;
    inc(chpos);
    Oldpos:=chpos;
    end;
end;
