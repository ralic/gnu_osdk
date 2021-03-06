{
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2001 by the Free Pascal development team

    Variant routines for non-windows oses.

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{ ---------------------------------------------------------------------
    Some general stuff: Error handling and so on.
  ---------------------------------------------------------------------}

{ we so ugly things with tvararray here }
{$RANGECHECKS OFF}

Procedure SetUnlockResult (P : PVarArray; Res : HResult);

begin
  If Res=VAR_OK then
    Res:=SafeArrayUnlock(P)
  else
    SafeArrayUnlock(P);
end;

Procedure MakeWideString (Var P : PWideChar; W : WideString);

begin
  P:=PWideChar(W);
end;

Procedure CopyAsWideString (Var PDest : PWideChar; PSource : PWideChar);

begin
  WideString(Pointer(PDest)):=WideString(Pointer(PSource));
end;

{ ---------------------------------------------------------------------
    Basic variant handling.
  ---------------------------------------------------------------------}

function VariantInit(var Varg: TVarData): HRESULT;stdcall;
begin
  With Varg do
    begin
      FillChar(VBytes, SizeOf(VBytes), 0);
      VType:=varEmpty;
    end;
  Result:=VAR_OK;
end;

function VariantClear(var Varg: TVarData): HRESULT;stdcall;
begin
  With Varg do
    if (VType and varArray)=varArray then
      begin
      Exit(SafeArrayDestroy(VArray))
      end
    else
      begin
      if (VType and varByRef) = 0 then
        case VType of
          varEmpty, varNull, varSmallint, varInteger, varSingle, varDouble, varWord,
          varCurrency, varDate, varError, varBoolean, varByte,VarShortInt,
          varInt64, VarLongWord,VarQWord:
            ;
          varOleStr:
            WideString(Pointer(VOleStr)):='';
          varDispatch,
          varUnknown:
            iinterface(vunknown):=nil;
        else
          exit(VAR_BADVARTYPE)
        end;
    end;
  Result:=VariantInit(Varg);
end;

function VariantCopy(var VargDest: TVarData; const VargSrc: TVarData): HRESULT;stdcall;
begin
  if @VargSrc = @VargDest then
    Exit(VAR_OK);
  Result:=VariantClear(VargDest);
  if Result<>VAR_OK then
    exit;
  With VargSrc do
    begin
    if (VType and varArray) <> 0 then
      Result:=SafeArrayCopy(VArray,VargDest.VArray)
    else
      begin
      if (VType and varByRef) <> 0 then
        VArgDest.VPointer:=VPointer
      else
        case (VType and varTypeMask) of
          varEmpty, varNull:;
          varSmallint, varInteger, varSingle, varDouble, varCurrency, varWord,
          varDate, varError, varBoolean, varByte,VarShortInt,
          varInt64, VarLongWord,VarQWord:
            Move(VBytes, VargDest.VBytes, SizeOf(VargDest.VBytes));
          varOleStr:
            CopyAsWideString(VargDest.VOleStr,VOleStr);
          varDispatch:
            IUnknown(VargDest.vdispatch):=IUnknown(VargSrc.vdispatch);
          varUnknown:
            IUnknown(VargDest.vunknown):=IUnknown(VargSrc.vunknown);
          else
            Exit(VAR_BADVARTYPE);
          end;
        end;
        VargDest.VType:=VType;
    end;
end;

function VariantCopyInd(var VargDest: TVarData; const VargSrc: TVarData): HRESULT;stdcall;

begin
  if (VargSrc.VType and varByRef) = 0 then
    Exit(VariantCopy(VargDest, VargSrc));
  With VargSrc do
    begin
    if (VType and varArray) <> 0 then
      Exit(VAR_INVALIDARG);
    case (VType and varTypeMask) of
      varEmpty, varNull:;
      varSmallint : VargDest.VSmallInt:=PSmallInt(VPointer)^;
      varInteger  : VargDest.VInteger:=PLongint(VPointer)^;
      varSingle   : VargDest.VSingle:=PSingle(VPointer)^;
      varDouble   : VargDest.VDouble:=PDouble(VPointer)^;
      varCurrency : VargDest.VCurrency:=PCurrency(VPointer)^;
      varDate     : VargDest.VDate:=PDate(VPointer)^;
      varBoolean  : VargDest.VBoolean:=PWordBool(VPointer)^;
      varError    : VargDest.VError:=PError(VPointer)^;
      varByte     : VargDest.VByte:=PByte(VPointer)^;
      varWord     : VargDest.VWord:=PWord(VPointer)^;
      VarShortInt : VargDest.VShortInt:=PShortInt(VPointer)^;
      VarInt64    : VargDest.VInt64:=PInt64(VPointer)^;
      VarLongWord : VargDest.VLongWord:=PCardinal(VPointer)^;
      VarQWord    : VargDest.VQWord:=PQWord(VPointer)^;
      varVariant  : Variant(VargDest):=Variant(PVarData(VPointer)^);
      varOleStr   : CopyAsWideString(VargDest.VOleStr,PVarData(VPointer)^.VoleStr);
      varDispatch,
      varUnknown  : NoInterfaces;
      else
        Exit(VAR_BADVARTYPE);
      end;
    VargDest.VType:=VType and VarTypeMask;
    end;
  Result:=VAR_OK;
end;

Function VariantChangeTypeEx(var VargDest: TVarData; const VargSrc: TVarData;
  LCID: Integer; Flags: Word; VarType: Word): HRESULT; stdcall;
var
  Tmp : TVarData;
begin
  if ((VarType and varArray) <> 0) or
     ((VargSrc.VType and varArray) <> 0) or
     ((VarType and varByRef) <> 0) then
    Exit(VAR_INVALIDARG);
  Result:=VariantCopyInd(Tmp, VargSrc);
  if Result = VAR_OK then
    try
    Result:=VariantClear(VargDest);
    {$RANGECHECKS ON}
    if Result = VAR_OK then
      try
        case Vartype of
          varSmallInt : VargDest.VSmallInt:=VariantToSmallInt(Tmp);
          varInteger  : VargDest.VInteger:=VariantToLongint(Tmp);
          varSingle   : VargDest.VSingle:=VariantToSingle(Tmp);
          varDouble   : VargDest.VDouble:=VariantToDouble(Tmp);
          varCurrency : VargDest.VCurrency:=VariantToCurrency(Tmp);
          varDate     : VargDest.VDate:=VariantToDate(tmp);
          varOleStr   : MakeWideString(VargDest.VoleStr, VariantToWideString(tmp));
          varDispatch : Result:=VAR_TYPEMISMATCH;
          varUnknown  : Result:=VAR_TYPEMISMATCH;
          varBoolean  : VargDest.VBoolean:=VariantToBoolean(Tmp);
          varByte     : VargDest.VByte:=VariantToByte(Tmp);
          VarShortInt : VargDest.VShortInt:=VariantToShortInt(Tmp);
          VarInt64    : VargDest.Vint64:=VariantToInt64(Tmp);
          VarLongWord : VargDest.VLongWord:=VariantToCardinal(Tmp);
          VarQWord    : VargDest.VQWord:=VariantToQword(tmp);
       else
          Result:=VAR_BADVARTYPE;
       end;
       If Result = VAR_OK then
         VargDest.VType:=VarType;
      except
        On E : EVariantError do
          Result:=E.ErrCode;
        else
          Result:=VAR_INVALIDARG;
      end;
    finally
      VariantClear(Tmp);
    end;
  {$RANGECHECKS OFF}
end;

{ ---------------------------------------------------------------------
    Variant array support
  ---------------------------------------------------------------------}

Function CheckArrayUnlocked (psa : PVarArray) : HResult;

begin
  If psa^.LockCount = 0 Then
    Result:=VAR_OK
  else
    Result:=VAR_ARRAYISLOCKED;
end;

Function CheckVarArray(psa: PVarArray ): HRESULT;

begin
  If psa=nil then
    Result:=VAR_INVALIDARG
  else
    Result:=VAR_OK;
end;

Function SafeArrayCalculateElementAddress(psa: PVarArray; aElement: SizeInt): Pointer;

begin
  Result:=Pointer(psa^.Data)+(aElement*psa^.ElementSize);
end;

Function CheckVarArrayAndCalculateAddress(psa: PVarArray;
  Indices: PVarArrayCoorArray; var Address: Pointer; Lockit: Boolean): HRESULT;

  Function CountElements(D: Longint): Longint;
  begin
    if (D<psa^.DimCount) then
      Result:=CountElements(D+1)+psa^.Bounds[D-1].ElementCount
    else
      Result:=1;
  end;

var
  LB,HB,I,Count : LongInt;

begin
  Result:=CheckVarArray(psa);
  Address:=nil;
  Count:=0;
  If Result<>VAR_OK then
    exit;
  for I:=1 to psa^.DimCount do
    begin
    LB:=psa^.Bounds[I-1].LowBound;
    HB:=LB+psa^.Bounds[I-1].ElementCount;
    if (LB=HB) or ((Indices^[I-1]< LB) or(Indices^[I-1]>HB)) then
      Exit(VAR_BADINDEX);
    Count:=Count+(Indices^[I-1]-LB)*CountElements(I+1);
  end;
  Address:=SafeArrayCalculateElementAddress(psa, Count);
  if LockIt then
    Result:=SafeArrayLock(psa);
end;

Function SafeArrayElementTotal(psa: PVarArray): Integer;

var
  I: Integer;

begin
  Result:=1;
  With psa^ do
   for I:=0 to DimCount - 1 do
     Result:=Result*Bounds[I].ElementCount;
end;

type
  TVariantArrayType = (vatNormal, vatInterface, vatWideString);

Function VariantArrayType(psa: PVarArray): TVariantArrayType;

begin
  if ((psa^.Flags and ARR_DISPATCH) <> 0) or
     ((psa^.Flags and ARR_UNKNOWN) <> 0) then
    Result:=vatInterface
  else if (psa^.Flags AND ARR_OLESTR) <> 0 then
    Result:=vatWideString
  else
    Result:=vatNormal;
end;

Function SafeArrayClearDataSpace(psa: PVarArray; WipeBytes: Boolean {= True}): HRESULT;

var
  vat: TVariantArrayType;

begin
  try
    vat:=VariantArrayType(psa);
    case vat of
      vatNormal     : FillChar(psa^.Data^,
                         SafeArrayElementTotal(psa)*psa^.ElementSize,
                         0);
      vatInterface  : NoInterfaces;
      vatWideString : NoWidestrings;
    end;
    Result:=VAR_OK;
  except
    On E : Exception do
      Result:=ExceptionToVariantError (E);
  end;
end;

Function SafeArrayCopyDataSpace(psa, psaOut: PVarArray): HRESULT;
var
  vVargSrc, vTarget: Pointer;
  vat: TVariantArrayType;
begin
  try
    vat:=VariantArrayType(psa);
    case vat of
      vatNormal: Move(psa^.Data^,
                      psaOut^.Data^,
                      SafeArrayElementTotal(psa)*psa^.ElementSize);
      vatInterface : NoInterfaces; // Copy element per element...
      vatWideString: NoWideStrings; // here also...
    end;
    Result:=VAR_OK;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
end;

Type
  TVartypes = varEmpty..varByte;

Const
  Supportedpsas : set of TVarTypes =
    [varSmallint,varInteger,varSingle,varDouble,varCurrency,varDate,varOleStr,
     varDispatch,varError,varBoolean,varVariant,varUnknown,varByte];
  psaElementSizes : Array [varEmpty..varByte] of Byte =
    (0,0,2,4,4,8,8,8,4,4,4,2,16,4,0,0,0,1);
  psaElementFlags : Array [varEmpty..varByte] of Longint =
    (ARR_NONE,ARR_NONE,ARR_NONE,ARR_NONE,ARR_NONE,ARR_NONE,ARR_NONE,ARR_NONE,
     ARR_OLESTR,ARR_DISPATCH,ARR_NONE,ARR_NONE,ARR_NONE,ARR_UNKNOWN,
     ARR_NONE,ARR_NONE,ARR_NONE,ARR_NONE);

Function SafeArrayCreate(VarType, Dim: SizeInt; const Bounds: TVarArrayBoundArray): PVarArray;stdcall;
  var
    res : HRESULT;
    I   : SizeInt;
  begin
    Result:=nil;
    if Not (VarType in Supportedpsas) Then
      exit;
    Res:=SafeArrayAllocDescriptor(Dim, Result);
    if Res<>VAR_OK then
      exit;
    Result^.DimCount:=Dim;
    Result^.Flags:=psaElementFlags[VarType];
    Result^.ElementSize:=psaElementSizes[VarType];
    Result^.LockCount := 0;
    for i:=0 to Dim-1 do
      begin
        Result^.Bounds[i].LowBound:=Bounds[Dim-I-1].LowBound;
        Result^.Bounds[I].ElementCount:=Bounds[Dim-I-1].ElementCount;
      end;
    res:=SafeArrayAllocData(Result);
    if res<>VAR_OK then
      begin
      SafeArrayDestroyDescriptor(Result);
      Result:=nil;
      end;
  end;


Function SafeArrayAllocDescriptor(DimCount: SizeInt; var psa: PVarArray): HRESULT;stdcall;
begin
  try
    { one bound item is included in TVarArray }
    psa:=GetMem(SizeOf(TVarArray) + SizeOf(TVarArrayBound)*(DimCount-1));
    Result:=VAR_OK;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
end;

Function SafeArrayAllocData(psa: PVarArray): HRESULT;stdcall;
begin
  try
    With psa^ do
      begin
        Data:=GetMem(SafeArrayElementTotal(psa)*ElementSize);
        fillchar(Data^,SafeArrayElementTotal(psa)*ElementSize,0);
      end;
    Result:=VAR_OK;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
end;

Function SafeArrayDestroy(psa: PVarArray): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if Result<> VAR_OK then
    exit;
  Result:=CheckArrayUnlocked(psa);
  if Result<> VAR_OK then
    exit;
  Result:=SafeArrayDestroyData(psa);
  if Result<>VAR_OK then
    exit;
  Result:=SafeArrayDestroyDescriptor(psa);
end;

Function SafeArrayDestroyDescriptor(psa: PVarArray): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  Result:=CheckArrayUnlocked(psa);
  if Result<> VAR_OK then
    exit;
  try
    FreeMem(psa);
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
end;

Function SafeArrayDestroyData(psa: PVarArray): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  Result:=CheckArrayUnlocked(psa);
  if Result<> VAR_OK then
    exit;
  try
    Result:=SafeArrayClearDataSpace(psa, False);
    if (Result=VAR_OK) and ((psa^.Flags and ARR_FIXEDSIZE)=0) then
      begin
      FreeMem(psa^.Data);
      psa^.Data:=nil;
      end;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
end;

Function SafeArrayRedim(psa: PVarArray; const NewBound: TVarArrayBound): HRESULT;stdcall;

var
  vat: TVariantArrayType;
  i, D,j,count : Integer;
  P : Pointer;

begin
  Result:=CheckVarArray(psa);
  if Result <> VAR_OK then
    exit;
  if (psa^.Flags and ARR_FIXEDSIZE) <> 0 then
    Exit(VAR_INVALIDARG);
  Result:=SafeArrayLock(psa);
  if Result<>VAR_OK then
    exit;
  try
    D:=NewBound.ElementCount - psa^.Bounds[0].ElementCount;
    for i:=1 to psa^.DimCount - 1 do
      D:=D*psa^.Bounds[i].ElementCount;
    if D<>0 then
      begin
      Count:=SafeArrayElementTotal(psa);
      if D<0 then
        begin
        vat:=VariantArrayType(psa);
        for j:=Count-1 downto Count+D do
          begin
          P:=SafeArrayCalculateElementAddress(psa,j);
          if vat = vatInterface then
            NoInterfaces // Set to nil
          else
            NoWideStrings; // Set to empty...
          end;
        end;
      ReAllocMem(psa^.Data,Count+D);
      end;
    psa^.Bounds[0].ElementCount:=NewBound.ElementCount;
    psa^.Bounds[0].LowBound:=NewBound.LowBound;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
  SetUnlockResult(psa,Result);
end;

Function SafeArrayCopy(psa: PVarArray; var psaOut: PVarArray): HRESULT;stdcall;

var
  i : Integer;

begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  Result:=SafeArrayLock(psa);
  if Result<>VAR_OK then
    exit;
  try
    Result:=SafeArrayAllocDescriptor(psa^.DimCount,psaOut);
    if Result<>VAR_OK then
      Exit;
    try
      With psaOut^ do
        begin
        Flags:=psa^.Flags;
        ElementSize:=psa^.ElementSize;
        LockCount := 0;
        DimCount:=psa^.DimCount;
        for i:=0 to DimCount-1 do
          begin
          Bounds[i].ElementCount:=psa^.Bounds[i].ElementCount;
          Bounds[i].LowBound:=psa^.Bounds[i].LowBound;
          end;
        end;
      Result:=SafeArrayAllocData(psaOut);
      if Result<>VAR_OK then
        exit;
      Result:=SafeArrayCopyDataSpace(psa, psaOut);
    finally
      if Result<>VAR_OK then
        begin
        SafeArrayDestroyDescriptor(psaOut);
        psaOut:=nil;
        end;
    end;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E)
  end;
  SetUnlockResult(psa,Result);
end;

Function SafeArrayCopyData(psa, psaOut: PVarArray): HRESULT;stdcall;
var
  i : Integer;
begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  Result:=CheckVarArray(psaOut);
  if Result<>VAR_OK then
    exit;
  Result:=SafeArrayLock(psaOut);
  if Result<>VAR_OK then
    exit;
  try
    Result:=SafeArrayLock(psa);
    if Result<>VAR_OK then
     exit;
    try
      With psaOut^ do
        begin
        if (psa^.Flags<>Flags) or
           (psa^.ElementSize<>ElementSize) or
           (psa^.DimCount<>DimCount) then
          Exit(VAR_INVALIDARG);
        for i:=0 to psa^.DimCount - 1 do
          if (psa^.Bounds[i].LowBound<>Bounds[i].LowBound) or
             (psa^.Bounds[i].ElementCount<>Bounds[i].ElementCount) then
            exit(VAR_INVALIDARG);
        end;
      Result:=SafeArrayClearDataSpace(psaOut,True);
      if Result<> VAR_OK then
        exit;
      Result:=SafeArrayCopyDataSpace(psa, psaOut);
    finally
      SetUnlockResult(psa,Result);
    end;
  finally
    SetUnlockResult(psaOut,Result);
  end;
end;

Function SafeArrayGetLBound(psa: PVarArray; Dim: SizeInt; var LBound: SizeInt): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  if (Dim>0) and (Dim<=psa^.DimCount) then
    LBound:=psa^.Bounds[Dim-1].LowBound
  else
    Result:=VAR_BADINDEX;
end;

Function SafeArrayGetUBound(psa: PVarArray; Dim: SizeInt; var UBound: SizeInt): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  if (Dim>0) and (Dim<=psa^.DimCount) then
    UBound:=psa^.Bounds[Dim-1].LowBound +
            psa^.Bounds[Dim-1].ElementCount-1
  else
    Result:=VAR_BADINDEX
end;

Function SafeArrayGetDim(psa: PVarArray): SizeInt;stdcall;
begin
  if CheckVarArray(psa)<>VAR_OK then
    Result:=0
  else
    Result:=psa^.DimCount;
end;

Function SafeArrayAccessData(psa: PVarArray; var ppvData: Pointer): HRESULT;stdcall;
begin
  Result:=SafeArrayLock(psa);
  if Result<>VAR_OK then
    ppvData:=nil
  else
    ppvData:=psa^.Data;
end;

Function SafeArrayUnaccessData(psa: PVarArray): HRESULT;stdcall;
begin
  Result:=SafeArrayUnlock(psa);
end;

Function SafeArrayLock(psa: PVarArray): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if Result<>VAR_OK then
    exit;
  InterlockedIncrement(psa^.LockCount);
end;

Function SafeArrayUnlock(psa: PVarArray): HRESULT;stdcall;
begin
  Result:=CheckVarArray(psa);
  if (Result<>VAR_OK) then
    exit;
  if InterlockedDecrement(psa^.LockCount)<0 then
    begin
      InterlockedIncrement(psa^.LockCount);
      result:=VAR_UNEXPECTED;
    end;
end;


Function SafeArrayGetElement(psa: PVarArray; Indices: PVarArrayCoorArray;
  Data: Pointer): HRESULT;stdcall;
var
  P: Pointer;
begin
  Result:=CheckVarArrayAndCalculateAddress(psa, Indices, P, True);
  if Result<>VAR_OK then
    exit;
  try
    case VariantArrayType(psa) of
      vatNormal:
        Move(P^, Data^, psa^.ElementSize);
      vatInterface:
        NoInterfaces; // Just assign...
      vatWideString:
        NoWideStrings; // Just assign...
    end;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
  SetUnlockResult(psa,Result);
end;


Function SafeArrayPutElement(psa: PVarArray; Indices: PVarArrayCoorArray;
  const Data: Pointer): HRESULT;stdcall;
var
  P: Pointer;
begin
  Result:=CheckVarArrayAndCalculateAddress(psa,Indices,P,True);
  if Result<>VAR_OK then
    exit;
  try
    case VariantArrayType(psa) of
      vatNormal:
        Move(Data^,P^,psa^.ElementSize);
      vatInterface:
        NoInterfaces;
      vatWideString:
        NoWideStrings;
    end;
  except
    On E : Exception do
      Result:=ExceptionToVariantError(E);
  end;
  SetUnlockResult(psa,Result);
end;


Function SafeArrayPtrOfIndex(psa: PVarArray; Indices: PVarArrayCoorArray;
  var Address: Pointer): HRESULT;stdcall;
begin
  Result:=CheckVarArrayAndCalculateAddress(psa,Indices,Address,False);
end;


Function SafeArrayGetElemSize(psa: PVarArray): LongWord;stdcall;
begin
  if CheckVarArray(psa)<>VAR_OK then
    Result:=0
  else
    Result:=psa^.ElementSize;
end;

