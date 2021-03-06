{
    This file is part of the Free Component Library (FCL)
    Copyright (c) 1999-2000 by the Free Pascal development team

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}
{****************************************************************************}
{*                             TStrings                                     *}
{****************************************************************************}

// Function to quote text. Should move maybe to sysutils !!
// Also, it is not clear at this point what exactly should be done.

{ //!! is used to mark unsupported things. }

Function QuoteString (Const S : String; Quote : String) : String;
Var
  I,J : Integer;
begin
  J:=0;
  Result:=S;
  for i:=1to length(s) do
   begin
     inc(j);
     if S[i]=Quote then
      begin
        System.Insert(Quote,Result,J);
        inc(j);
      end;
   end;
  Result:=Quote+Result+Quote;
end;

{
  For compatibility we can't add a Constructor to TSTrings to initialize
  the special characters. Therefore we add a routine which is called whenever
  the special chars are needed.
}

Procedure Tstrings.CheckSpecialChars;

begin
  If Not FSpecialCharsInited then
    begin
    FQuoteChar:='"';
    FDelimiter:=',';
    FNameValueSeparator:='=';
    FSpecialCharsInited:=true;
    end;
end;


procedure TStrings.SetDelimiter(c:Char);
begin
  CheckSpecialChars;
  FDelimiter:=c;
end;


procedure TStrings.SetQuoteChar(c:Char);
begin
  CheckSpecialChars;
  FQuoteChar:=c;
end;


procedure TStrings.SetNameValueSeparator(c:Char);
begin
  CheckSpecialChars;
  FNameValueSeparator:=c;
end;


function TStrings.GetCommaText: string;

Var
  C1,C2 : Char;

begin
  CheckSpecialChars;
  C1:=Delimiter;
  C2:=QuoteChar;
  Delimiter:=',';
  QuoteChar:='"';
  Try
    Result:=GetDelimitedText;
  Finally
    Delimiter:=C1;
    QuoteChar:=C2;
  end;
end;


Function TStrings.GetDelimitedText: string;

Var
  I : integer;
  p : pchar;
begin
  CheckSpecialChars;
  result:='';
  For i:=0 to count-1 do
    begin
    p:=pchar(strings[i]);
    while not(p^ in [#0..' ',QuoteChar,Delimiter]) do
     inc(p);
    if p^<>#0 then
     Result:=Result+QuoteString (Strings[I],QuoteChar)
    else
     result:=result+strings[i];
    if I<Count-1 then Result:=Result+Delimiter;
    end;
  If Length(Result)=0 then
    Result:=QuoteChar+QuoteChar;
end;

procedure TStrings.GetNameValue(Index : Integer; Var AName,AValue : String);

Var L : longint;

begin
  CheckSpecialChars;
  AValue:=Strings[Index];
  L:=Pos(FNameValueSeparator,AValue);
  If L<>0 then
    begin
    AName:=Copy(AValue,1,L-1);
    System.Delete(AValue,1,L);
    end
  else
    AName:='';
end;

function TStrings.GetName(Index: Integer): string;

Var
  V : String;

begin
  GetNameValue(Index,Result,V);
end;

Function TStrings.GetValue(const Name: string): string;

Var
  L : longint;
  N : String;

begin
  Result:='';
  L:=IndexOfName(Name);
  If L<>-1 then
    GetNameValue(L,N,Result);
end;

Function TStrings.GetValueFromIndex(Index: Integer): string;

Var
  N : String;

begin
  GetNameValue(Index,N,Result);
end;

Procedure TStrings.SetValueFromIndex(Index: Integer; const Value: string);

begin
  If (Value='') then
    Delete(Index)
  else
    begin
    If (Index<0) then
      Index:=Add('');
    CheckSpecialChars;
    Strings[Index]:=GetName(Index)+FNameValueSeparator+Value;
    end;
end;


procedure TStrings.ReadData(Reader: TReader);
begin
  Reader.ReadListBegin;
  BeginUpdate;
  try
    Clear;
    while not Reader.EndOfList do
      Add(Reader.ReadString);
  finally
    EndUpdate;
  end;
  Reader.ReadListEnd;
end;

Function GetQuotedString (Var P : Pchar; QuoteChar : Char) : AnsiString;

Var P1,L : Pchar;
    ReplaceQuotes : boolean;

begin
  Result:='';
  ReplaceQuotes := False;
  P1:=P+1;
  While P1^<>#0 do
    begin
      If (P1^=QuoteCHar) then
       begin
         if (P1[1]<>QuoteChar) then
          break;
         inc(p1);
         ReplaceQuotes := True;
       end;
      inc(p1);
    end;
  // P1 points to last quote, or to #0;
  P:=P+1;
  If P1-P>0 then
    begin
    SetLength(Result,(P1-P));
    L:=Pointer(Result);
    Move (P^,L^,P1-P);
    P:=P1+1;
    end;
  if ReplaceQuotes then
    result := StringReplace (result, QuoteChar+QuoteChar,QuoteChar, [rfReplaceAll]);
end;

Function GetNextQuotedChar (var P : PChar; Var S : String; Delim,Quote : Char): Boolean;

Var PS,L : PChar;

begin
  Result:=False;
  S:='';
  While (p^<>#0) and (byte(p^)<=byte(' ')) do
   inc(p);
  If P^=#0 then exit;
  PS:=P;
  If P^=Quote then
   begin
     S:=GetQuotedString(P,Quote);
     While (p^<>#0) and (byte(p^)<=byte(' ')) do
      inc(p);
   end
  else
   begin
     While (p^>' ') and (P^<>Delim) do
      inc(p);
     Setlength (S,P-PS);
     L:=Pointer(S);
     Move (PS^,L^,P-PS);
   end;
  if p^=Delim then
   inc(p);
  Result:=True;
end;

Procedure TStrings.SetDelimitedText(const AValue: string);

Var
  P : PChar;
  S : String;
begin
  CheckSpecialChars;
  BeginUpdate;
  try
    Clear;
    P:=PChar(AValue);
    if assigned(p) then
     begin
       While GetNextQuotedChar (P,S,FDelimiter,FQuoteChar) do
        Add (S);
     end;
  finally
    EndUpdate;
  end;
end;

Procedure TStrings.SetCommaText(const Value: string);

Var
  C1,C2 : Char;

begin
  CheckSpecialChars;
  C1:=Delimiter;
  C2:=QuoteChar;
  Delimiter:=',';
  QuoteChar:='"';
  Try
    SetDelimitedText(Value);
  Finally
    Delimiter:=C1;
    QuoteChar:=C2;
  end;
end;


Procedure TStrings.SetStringsAdapter(const Value: IStringsAdapter);

begin
end;



Procedure TStrings.SetValue(const Name, Value: string);

Var L : longint;

begin
  CheckSpecialChars;
  L:=IndexOfName(Name);
  if L=-1 then
   Add (Name+FNameValueSeparator+Value)
  else
   Strings[L]:=Name+FNameValueSeparator+value;
end;



procedure TStrings.WriteData(Writer: TWriter);
var
  i: Integer;
begin
  Writer.WriteListBegin;
  for i := 0 to Count - 1 do
    Writer.WriteString(Strings[i]);
  Writer.WriteListEnd;
end;



procedure TStrings.DefineProperties(Filer: TFiler);
var
  HasData: Boolean;
begin
  if Assigned(Filer.Ancestor) then
    // Only serialize if string list is different from ancestor
    if Filer.Ancestor.InheritsFrom(TStrings) then
      HasData := not Equals(TStrings(Filer.Ancestor))
    else
      HasData := True
  else
    HasData := Count > 0;
  Filer.DefineProperty('Strings', @ReadData, @WriteData, HasData);
end;


Procedure TStrings.Error(const Msg: string; Data: Integer);
begin
  Raise EStringListError.CreateFmt(Msg,[Data]) at get_caller_addr(get_frame);
end;


Procedure TStrings.Error(const Msg: pstring; Data: Integer);
begin
  Raise EStringListError.CreateFmt(Msg^,[Data]) at get_caller_addr(get_frame);
end;


Function TStrings.GetCapacity: Integer;

begin
  Result:=Count;
end;



Function TStrings.GetObject(Index: Integer): TObject;

begin
  Result:=Nil;
end;



Function TStrings.GetTextStr: string;

Const
{$ifdef Unix}
  NewLineSize=1;
{$else}
  NewLineSize=2;
{$endif}

Var P : Pchar;
    I,L : Longint;
    S : String;

begin
  // Determine needed place
  L:=0;
  For I:=0 to count-1 do
    L:=L+Length(Strings[I])+NewLineSize;
  Setlength(Result,L);
  P:=Pointer(Result);
  For i:=0 To count-1 do
    begin
    S:=Strings[I];
    L:=Length(S);
    if L<>0 then
      System.Move(Pointer(S)^,P^,L);
    P:=P+L;
{$ifndef Unix}
    p[0]:=#13;
    p[1]:=#10;
{$else}
    p[0]:=#10;
{$endif}
    P:=P+NewLineSize;
    end;
end;



Procedure TStrings.Put(Index: Integer; const S: string);

Var Obj : TObject;

begin
  Obj:=Objects[Index];
  Delete(Index);
  InsertObject(Index,S,Obj);
end;



Procedure TStrings.PutObject(Index: Integer; AObject: TObject);

begin
  // Empty.
end;



Procedure TStrings.SetCapacity(NewCapacity: Integer);

begin
  // Empty.
end;

Function GetNextLine (Const Value : String; Var S : String; Var P : Integer) : Boolean;

Var 
  PS : PChar;
  IP,L : Integer;
  
begin
  L:=Length(Value);
  S:='';
  Result:=False;
  If ((L-P)<0) then 
    exit;
  if ((L-P)=0) and (not (value[P] in [#10,#13])) Then
    Begin
      s:=value[P];
      inc(P);
      Exit(True);
    End;
  PS:=PChar(Value)+P-1;
  IP:=P;
  While ((L-P)>=0) and (not (PS^ in [#10,#13])) do 
    begin
    P:=P+1;
    Inc(PS);
    end;
  SetLength (S,P-IP);
  System.Move (Value[IP],Pointer(S)^,P-IP);
  If (P<=L) and (Value[P]=#13) then 
    Inc(P);
  If (P<=L) and (Value[P]=#10) then
    Inc(P); // Point to character after #10(#13)
  Result:=True;
end;

Procedure TStrings.SetTextStr(const Value: string);

Var
  S : String;
  P,L : Integer;

begin
  Try
    beginUpdate;
    Clear;
    P:=1;
    While GetNextLine (Value,S,P) do
      Add(S);
  finally
    EndUpdate;
  end;
end;



Procedure TStrings.SetUpdateState(Updating: Boolean);

begin
end;



destructor TSTrings.Destroy;

begin
  inherited destroy;
end;



Function TStrings.Add(const S: string): Integer;

begin
  Result:=Count;
  Insert (Count,S);
end;



Function TStrings.AddObject(const S: string; AObject: TObject): Integer;

begin
  Result:=Add(S);
  Objects[result]:=AObject;
end;



Procedure TStrings.Append(const S: string);

begin
  Add (S);
end;



Procedure TStrings.AddStrings(TheStrings: TStrings);

Var Runner : longint;

begin
  try
    beginupdate;
    For Runner:=0 to TheStrings.Count-1 do
      self.AddObject (Thestrings[Runner],TheStrings.Objects[Runner]);
  finally
    EndUpdate;
  end;
end;



Procedure TStrings.Assign(Source: TPersistent);

Var
  S : TStrings;

begin
  If Source is TStrings then
    begin
    S:=TStrings(Source);
    BeginUpdate;
    Try
      clear;
      FSpecialCharsInited:=S.FSpecialCharsInited;
      FQuoteChar:=S.FQuoteChar;
      FDelimiter:=S.FDelimiter;
      FNameValueSeparator:=S.FNameValueSeparator;
      AddStrings(S);
    finally
      EndUpdate;
    end;
    end
  else
    Inherited Assign(Source);
end;



Procedure TStrings.BeginUpdate;

begin
   if FUpdateCount = 0 then SetUpdateState(true);
   inc(FUpdateCount);
end;



Procedure TStrings.EndUpdate;

begin
  If FUpdateCount>0 then
     Dec(FUpdateCount);
  if FUpdateCount=0 then
    SetUpdateState(False);
end;



Function TStrings.Equals(TheStrings: TStrings): Boolean;

Var Runner,Nr : Longint;

begin
  Result:=False;
  Nr:=Self.Count;
  if Nr<>TheStrings.Count then exit;
  For Runner:=0 to Nr-1 do
    If Strings[Runner]<>TheStrings[Runner] then exit;
  Result:=True;
end;



Procedure TStrings.Exchange(Index1, Index2: Integer);

Var
  Obj : TObject;
  Str : String;

begin
  Try
    beginUpdate;
    Obj:=Objects[Index1];
    Str:=Strings[Index1];
    Objects[Index1]:=Objects[Index2];
    Strings[Index1]:=Strings[Index2];
    Objects[Index2]:=Obj;
    Strings[Index2]:=Str;
  finally
    EndUpdate;
  end;
end;



Function TStrings.GetText: PChar;
begin
  Result:=StrNew(Pchar(Self.Text));
end;


Function TStrings.DoCompareText(const s1,s2 : string) : PtrInt;
  begin
    result:=CompareText(s1,s2);
  end;


Function TStrings.IndexOf(const S: string): Integer;
begin
  Result:=0;
  While (Result<Count) and (DoCompareText(Strings[Result],S)<>0) do Result:=Result+1;
  if Result=Count then Result:=-1;
end;


Function TStrings.IndexOfName(const Name: string): Integer;
Var
  len : longint;
  S : String;
begin
  CheckSpecialChars;
  Result:=0;
  while (Result<Count) do
    begin
    S:=Strings[Result];
    len:=pos(FNameValueSeparator,S)-1;
    if (len>0) and (DoCompareText(Name,Copy(S,1,Len))=0) then
      exit;
    inc(result);
    end;
  result:=-1;
end;


Function TStrings.IndexOfObject(AObject: TObject): Integer;
begin
  Result:=0;
  While (Result<count) and (Objects[Result]<>AObject) do Result:=Result+1;
  If Result=Count then Result:=-1;
end;


Procedure TStrings.InsertObject(Index: Integer; const S: string;
  AObject: TObject);

begin
  Insert (Index,S);
  Objects[Index]:=AObject;
end;



Procedure TStrings.LoadFromFile(const FileName: string);
Var
        TheStream : TFileStream;
begin
  TheStream:=TFileStream.Create(FileName,fmOpenRead);
  LoadFromStream(TheStream);
  TheStream.Free;
end;



Procedure TStrings.LoadFromStream(Stream: TStream);
{
   Borlands method is no good, since a pipe for
   instance doesn't have a size.
   So we must do it the hard way.
}
Const
  BufSize = 1024;
  MaxGrow = 1 shl 29;

Var
  Buffer     : AnsiString;
  BytesRead,
  BufLen,
  I,BufDelta     : Longint;
begin
  // reread into a buffer
  try
    beginupdate;
    Buffer:='';
    BufLen:=0;
    I:=1;
    Repeat
      BufDelta:=BufSize*I;
      SetLength(Buffer,BufLen+BufDelta);
      BytesRead:=Stream.Read(Buffer[BufLen+1],BufDelta);
      inc(BufLen,BufDelta);
      If I<MaxGrow then
        I:=I shl 1;
    Until BytesRead<>BufDelta;
    SetLength(Buffer, BufLen-BufDelta+BytesRead);
    SetTextStr(Buffer);
    SetLength(Buffer,0);
  finally
    EndUpdate;
  end;
end;


Procedure TStrings.Move(CurIndex, NewIndex: Integer);
Var
  Obj : TObject;
  Str : String;
begin
  BeginUpdate;
  Obj:=Objects[CurIndex];
  Str:=Strings[CurIndex];
  Delete(Curindex);
  InsertObject(NewIndex,Str,Obj);
  EndUpdate;
end;



Procedure TStrings.SaveToFile(const FileName: string);

Var TheStream : TFileStream;

begin
  TheStream:=TFileStream.Create(FileName,fmCreate);
  SaveToStream(TheStream);
  TheStream.Free;
end;



Procedure TStrings.SaveToStream(Stream: TStream);
Var
  S : String;
begin
  S:=Text;
  Stream.Write(Pointer(S)^,Length(S));
end;




Procedure TStrings.SetText(TheText: PChar);

Var S : String;

begin
  If TheText<>Nil then
    S:=StrPas(TheText)
  else
    S:='';
  SetTextStr(S);  
end;


{****************************************************************************}
{*                             TStringList                                  *}
{****************************************************************************}



Procedure TStringList.ExchangeItems(Index1, Index2: Integer);

Var P1,P2 : Pointer;

begin
  P1:=Pointer(Flist^[Index1].FString);
  P2:=Pointer(Flist^[Index1].FObject);
  Pointer(Flist^[Index1].Fstring):=Pointer(Flist^[Index2].Fstring);
  Pointer(Flist^[Index1].FObject):=Pointer(Flist^[Index2].FObject);
  Pointer(Flist^[Index2].Fstring):=P1;
  Pointer(Flist^[Index2].FObject):=P2;
end;



Procedure TStringList.Grow;

Var Extra : Longint;

begin
  If FCapacity>64 then
    Extra:=FCapacity Div 4
  Else If FCapacity>8 Then
    Extra:=16
  Else
    Extra:=4;
  SetCapacity(FCapacity+Extra);
end;



Procedure TStringList.QuickSort(L, R: Integer; CompareFn: TStringListSortCompare);

Var I,J, Pivot : Longint;

begin
  Repeat
    I:=L;
    J:=R;
    Pivot:=(L+R) div 2;
    Repeat
      While CompareFn(Self, I, Pivot)<0 do Inc(I);
      While CompareFn(Self, J, Pivot)>0 do Dec(J);
      If I<=J then
        begin
        ExchangeItems(I,J); // No check, indices are correct.
        if Pivot=I then
          Pivot:=J
        else if Pivot=J then
          Pivot := I;
        Inc(I);
        Dec(j);
        end;
    until I>J;
    If L<J then QuickSort(L,J, CompareFn);
    L:=I;
  Until I>=R;
end;


Procedure TStringList.InsertItem(Index: Integer; const S: string);
begin
  Changing;
  If FCount=Fcapacity then Grow;
  If Index<FCount then
    System.Move (FList^[Index],FList^[Index+1],
                 (FCount-Index)*SizeOf(TStringItem));
  Pointer(Flist^[Index].Fstring):=Nil;  // Needed to initialize...
  Flist^[Index].FString:=S;
  Flist^[Index].Fobject:=Nil;
  Inc(FCount);
  Changed;
end;


Procedure TStringList.InsertItem(Index: Integer; const S: string; O: TObject);
begin
  Changing;
  If FCount=Fcapacity then Grow;
  If Index<FCount then
    System.Move (FList^[Index],FList^[Index+1],
                 (FCount-Index)*SizeOf(TStringItem));
  Pointer(Flist^[Index].Fstring):=Nil;  // Needed to initialize...
  Flist^[Index].FString:=S;
  Flist^[Index].FObject:=O;
  Inc(FCount);
  Changed;
end;


Procedure TStringList.SetSorted(Value: Boolean);

begin
  If FSorted<>Value then
    begin
    If Value then sort;
    FSorted:=VAlue
    end;
end;



Procedure TStringList.Changed;

begin
  If (FUpdateCount=0) Then
   If Assigned(FOnChange) then
     FOnchange(Self);
end;



Procedure TStringList.Changing;

begin
  If FUpdateCount=0 then
    if Assigned(FOnChanging) then
      FOnchanging(Self);
end;



Function TStringList.Get(Index: Integer): string;

begin
  If (Index<0) or (INdex>=Fcount)  then
    Error (SListIndexError,Index);
  Result:=Flist^[Index].FString;
end;



Function TStringList.GetCapacity: Integer;

begin
  Result:=FCapacity;
end;



Function TStringList.GetCount: Integer;

begin
  Result:=FCount;
end;



Function TStringList.GetObject(Index: Integer): TObject;

begin
  If (Index<0) or (INdex>=Fcount)  then
    Error (SListIndexError,Index);
  Result:=Flist^[Index].FObject;
end;



Procedure TStringList.Put(Index: Integer; const S: string);

begin
  If Sorted then
    Error(SSortedListError,0);
  If (Index<0) or (INdex>=Fcount)  then
    Error (SListIndexError,Index);
  Changing;
  Flist^[Index].FString:=S;
  Changed;
end;



Procedure TStringList.PutObject(Index: Integer; AObject: TObject);

begin
  If (Index<0) or (INdex>=Fcount)  then
    Error (SListIndexError,Index);
  Changing;
  Flist^[Index].FObject:=AObject;
  Changed;
end;



Procedure TStringList.SetCapacity(NewCapacity: Integer);

Var NewList : Pointer;
    MSize : Longint;

begin
  If (NewCapacity<0) then
     Error (SListCapacityError,NewCapacity);
  If NewCapacity>FCapacity then
    begin
    GetMem (NewList,NewCapacity*SizeOf(TStringItem));
    If NewList=Nil then
      Error (SListCapacityError,NewCapacity);
    If Assigned(FList) then
      begin
      MSize:=FCapacity*Sizeof(TStringItem);
      System.Move (FList^,NewList^,MSize);
      FillWord (Pchar(NewList)[MSize],(NewCapacity-FCapacity)*WordRatio, 0);
      FreeMem (Flist,MSize);
      end;
    Flist:=NewList;
    FCapacity:=NewCapacity;
    end
  else if NewCapacity<FCapacity then
    begin
    if NewCapacity = 0 then
    begin
      FreeMem(FList);
      FList := nil;
    end else
    begin
      GetMem(NewList, NewCapacity * SizeOf(TStringItem));
      System.Move(FList^, NewList^, NewCapacity * SizeOf(TStringItem));
      FreeMem(FList);
      FList := NewList;
    end;
    FCapacity:=NewCapacity;
    end;
end;



Procedure TStringList.SetUpdateState(Updating: Boolean);

begin
  If Updating then
    Changing
  else
    Changed
end;



destructor TStringList.Destroy;

Var I : Longint;

begin
  FOnChange:=Nil;
  FOnChanging:=Nil;
  // This will force a dereference. Can be done better...
  For I:=0 to FCount-1 do
    FList^[I].FString:='';
  FCount:=0;
  SetCapacity(0);
  Inherited destroy;
end;



Function TStringList.Add(const S: string): Integer;

begin
  If Not Sorted then
    Result:=FCount
  else
    If Find (S,Result) then
      Case DUplicates of
        DupIgnore : Exit;
        DupError : Error(SDuplicateString,0)
      end;
   InsertItem (Result,S);
end;



Procedure TStringList.Clear;

Var I : longint;

begin
  if FCount = 0 then Exit;
  Changing;
  For I:=0 to FCount-1 do
    Flist^[I].FString:='';
  FCount:=0;
  SetCapacity(0);
  Changed;
end;



Procedure TStringList.Delete(Index: Integer);

begin
  If (Index<0) or (Index>=FCount) then
    Error(SlistINdexError,Index);
  Changing;
  Flist^[Index].FString:='';
  Dec(FCount);
  If Index<FCount then
    System.Move(Flist^[Index+1],
                Flist^[Index],
                (Fcount-Index)*SizeOf(TStringItem));
  Changed;
end;



Procedure TStringList.Exchange(Index1, Index2: Integer);

begin
  If (Index1<0) or (Index1>=FCount) then
    Error(SListIndexError,Index1);
  If (Index2<0) or (Index2>=FCount) then
    Error(SListIndexError,Index2);
  Changing;
  ExchangeItems(Index1,Index2);
  changed;
end;


procedure TStringList.SetCaseSensitive(b : boolean);
  begin
        if b<>FCaseSensitive then
          begin
                FCaseSensitive:=b;
            if FSorted then
              sort;
          end;
  end;


Function TStringList.DoCompareText(const s1,s2 : string) : PtrInt;
  begin
        if FCaseSensitive then
          result:=AnsiCompareStr(s1,s2)
        else
          result:=AnsiCompareText(s1,s2);
  end;


Function TStringList.Find(const S: string; var Index: Integer): Boolean;

{ Searches for the first string <= S, returns True if exact match,
  sets index to the index f the found string. }

Var I,L,R,Temp : Longint;

begin
  Result:=False;
  // Use binary search.
  L:=0;
  R:=FCount-1;
  While L<=R do
    begin
    I:=(L+R) div 2;
    Temp:=DoCompareText(FList^ [I].FString,S);
    If Temp<0 then
      L:=I+1
    else
      begin
      R:=I-1;
      If Temp=0 then
        begin
        Result:=True;
        If Duplicates<>DupAccept then L:=I;
        end;
      end;
    end;
  Index:=L;
end;



Function TStringList.IndexOf(const S: string): Integer;

begin
  If Not Sorted then
    Result:=Inherited indexOf(S)
  else
    // faster using binary search...
    If Not Find (S,Result) then
      Result:=-1;
end;



Procedure TStringList.Insert(Index: Integer; const S: string);

begin
  If Sorted then
    Error (SSortedListError,0)
  else
    If (Index<0) or (Index>FCount) then
      Error (SListIndexError,Index)
    else
      InsertItem (Index,S);
end;


Procedure TStringList.CustomSort(CompareFn: TStringListSortCompare);

begin
  If Not Sorted and (FCount>1) then
    begin
    Changing;
    QuickSort(0,FCount-1, CompareFn);
    Changed;
    end;
end;

function StringListAnsiCompare(List: TStringList; Index1, Index: Integer): Integer;

begin
  Result := List.DoCompareText(List.FList^[Index1].FString,
    List.FList^[Index].FString);
end;

Procedure TStringList.Sort;

begin
  CustomSort(@StringListAnsiCompare);
end;



