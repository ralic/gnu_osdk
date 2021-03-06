{
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2000 by the Free Pascal development team.

    functions for heap management in the data segment

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{****************************************************************************}

{ Try to find the best matching block in general freelist }
{ define BESTMATCH}

{ DEBUG: Dump info when the heap needs to grow }
{ define DUMPGROW}

const
{$ifdef CPU64}
  blocksize    = 32;  { at least size of freerecord }
  blockshift     = 5;   { shr value for blocksize=2^blockshift}
  maxblocksize = 512+blocksize; { 1024+8 needed for heaprecord }
{$else}
  blocksize    = 16;  { at least size of freerecord }
  blockshift     = 4;   { shr value for blocksize=2^blockshift}
  maxblocksize = 512+blocksize; { 1024+8 needed for heaprecord }
{$endif}
  maxblockindex = maxblocksize div blocksize; { highest index in array of lists of memchunks }
  maxreusebigger = 8; { max reuse bigger tries }

  { common flags }
  fixedsizeflag  = 1;   { flag if the block is of fixed size }
  { memchunk var flags }
  usedflag       = 2;   { flag if the block is used or not }
  lastblockflag  = 4;   { flag if the block is the last in os chunk }
  firstblockflag = 8;   { flag if the block is the first in os chunk }
  sizemask = not(blocksize-1);
  fixedsizemask = sizemask and $ffff;

{****************************************************************************}

{$ifdef DUMPGROW}
  {$define DUMPBLOCKS}
{$endif}

{ Forward defines }
procedure SysHeapMutexInit;forward;
procedure SysHeapMutexDone;forward;
procedure SysHeapMutexLock;forward;
procedure SysHeapMutexUnlock;forward;

{ Memory manager }
const
  MemoryManager: TMemoryManager = (
    NeedLock: true;
    GetMem: @SysGetMem;
    FreeMem: @SysFreeMem;
    FreeMemSize: @SysFreeMemSize;
    AllocMem: @SysAllocMem;
    ReAllocMem: @SysReAllocMem;
    MemSize: @SysMemSize;
    GetHeapStatus: @SysGetHeapStatus;
    GetFPCHeapStatus: @SysGetFPCHeapStatus;
  );

  MemoryMutexManager: TMemoryMutexManager = (
    MutexInit: @SysHeapMutexInit;
    MutexDone: @SysHeapMutexDone;
    MutexLock: @SysHeapMutexLock;
    MutexUnlock: @SysHeapMutexUnlock;
  );

type
  poschunk = ^toschunk;
  { keep size of this record dividable by 16 }
  toschunk = record
    size,
    used,
    chunkindex : ptrint;
    next,
    prev  : poschunk;
{$ifdef CPU64}
    pad1 : array[0..0] of pointer;
{$else CPU64}
    pad1 : array[0..2] of pointer;
{$endif CPU64}
  end;

  pmemchunk_fixed  = ^tmemchunk_fixed;
  tmemchunk_fixed = record
    size   : ptrint;
    poc    : poschunk;
    next_fixed,
    prev_fixed : pmemchunk_fixed;
  end;

  pmemchunk_var  = ^tmemchunk_var;
  tmemchunk_var = record
    size     : ptrint;
    prevsize : ptrint;
    next_var,
    prev_var  : pmemchunk_var;
  end;

  { ``header'', ie. size of structure valid when chunk is in use }
  { should correspond to tmemchunk_var_hdr structure starting with the
    last field. Reason is that the overlap is starting from the end of the
    record.
    Alignment is 8 bytes for 32bit machines. This required
    for x86 MMX/SSE and for sparc Double values }
  tmemchunk_fixed_hdr = record
    size     : ptrint;
    poschunk : pointer;
  end;
  tmemchunk_var_hdr = record
    prevsize,
    size     : ptrint;
  end;

  tfreelists   = array[1..maxblockindex] of pmemchunk_fixed;
  pfreelists   = ^tfreelists;

var
  internal_status : TFPCHeapStatus;

  freelists_fixed    : tfreelists;
  freelists_free_chunk : array[1..maxblockindex] of boolean;
  freelist_var       : pmemchunk_var;
  freeoslist         : poschunk;
  freeoslistcount    : dword;


{*****************************************************************************
                             Memory Manager
*****************************************************************************}

procedure SetMemoryMutexManager(var MutexMgr: TMemoryMutexManager);
begin
  { Release old mutexmanager, the default manager does nothing so
    calling this without initializing is safe }
  MemoryMutexManager.MutexDone;
  { Copy new mutexmanager }
  MemoryMutexManager := MutexMgr;
  { Init new mutexmanager }
  MemoryMutexManager.MutexInit;
end;


procedure GetMemoryManager(var MemMgr:TMemoryManager);
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       MemMgr := MemoryManager;
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     MemMgr := MemoryManager;
   end;
end;


procedure SetMemoryManager(const MemMgr:TMemoryManager);
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       MemoryManager := MemMgr;
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     MemoryManager := MemMgr;
   end;
end;


function IsMemoryManagerSet:Boolean;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       IsMemoryManagerSet := (MemoryManager.GetMem<>@SysGetMem) or
                           (MemoryManager.FreeMem<>@SysFreeMem);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     IsMemoryManagerSet := (MemoryManager.GetMem<>@SysGetMem) or
                         (MemoryManager.FreeMem<>@SysFreeMem);
   end;
end;


procedure GetMem(Var p:pointer;Size:ptrint);
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       p := MemoryManager.GetMem(Size);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     p := MemoryManager.GetMem(Size);
   end;
end;

procedure GetMemory(Var p:pointer;Size:ptrint);
begin
  GetMem(p,size);
end;

procedure FreeMem(p:pointer;Size:ptrint);
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       MemoryManager.FreeMemSize(p,Size);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     MemoryManager.FreeMemSize(p,Size);
   end;
end;


procedure FreeMemory(p:pointer;Size:ptrint);
begin
  FreeMem(p,size);
end;


function GetHeapStatus:THeapStatus;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       result:=MemoryManager.GetHeapStatus();
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     result:=MemoryManager.GetHeapStatus();
   end;
end;


function GetFPCHeapStatus:TFPCHeapStatus;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       result:=MemoryManager.GetFPCHeapStatus();
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     Result:=MemoryManager.GetFPCHeapStatus();
   end;
end;


function MemSize(p:pointer):ptrint;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       MemSize := MemoryManager.MemSize(p);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     MemSize := MemoryManager.MemSize(p);
   end;
end;


{ Delphi style }
function FreeMem(p:pointer):ptrint;[Public,Alias:'FPC_FREEMEM_X'];
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       Freemem := MemoryManager.FreeMem(p);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     Freemem := MemoryManager.FreeMem(p);
   end;
end;


function FreeMemory(p:pointer):ptrint;

begin
 FreeMemory := FreeMem(p);
end;

function GetMem(size:ptrint):pointer;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       GetMem := MemoryManager.GetMem(Size);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     GetMem := MemoryManager.GetMem(Size);
   end;
end;

function GetMemory(size:ptrint):pointer;

begin
 GetMemory := Getmem(size);
end;

function AllocMem(Size:ptrint):pointer;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       AllocMem := MemoryManager.AllocMem(size);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     AllocMem := MemoryManager.AllocMem(size);
   end;
end;


function ReAllocMem(var p:pointer;Size:ptrint):pointer;
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       ReAllocMem := MemoryManager.ReAllocMem(p,size);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     ReAllocMem := MemoryManager.ReAllocMem(p,size);
   end;
end;


function ReAllocMemory(var p:pointer;Size:ptrint):pointer;

begin
 ReAllocMemory := ReAllocMem(p,size);
end;


{ Needed for calls from Assembler }
function fpc_getmem(size:ptrint):pointer;compilerproc;[public,alias:'FPC_GETMEM'];
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       fpc_GetMem := MemoryManager.GetMem(size);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     fpc_GetMem := MemoryManager.GetMem(size);
   end;
end;

procedure fpc_freemem(p:pointer);compilerproc;[public,alias:'FPC_FREEMEM'];
begin
  if IsMultiThread and MemoryManager.NeedLock then
   begin
     try
       MemoryMutexManager.MutexLock;
       if p <> nil then
         MemoryManager.FreeMem(p);
     finally
       MemoryMutexManager.MutexUnlock;
     end;
   end
  else
   begin
     if p <> nil then
       MemoryManager.FreeMem(p);
   end;
end;


{*****************************************************************************
                               GetHeapStatus
*****************************************************************************}

function SysGetFPCHeapStatus:TFPCHeapStatus;
begin
  internal_status.CurrHeapFree:=internal_status.CurrHeapSize-internal_status.CurrHeapUsed;
  result:=internal_status;
end;

function SysGetHeapStatus :THeapStatus;

begin
  internal_status.CurrHeapFree:=internal_status.CurrHeapSize-internal_status.CurrHeapUsed;
  result.TotalAllocated   :=internal_status.CurrHeapUsed;
  result.TotalFree        :=internal_status.CurrHeapFree;
  result.TotalAddrSpace   :=0;
  result.TotalUncommitted :=0;
  result.TotalCommitted   :=0;
  result.FreeSmall        :=0;
  result.FreeBig          :=0;
  result.Unused           :=0;
  result.Overhead         :=0;
  result.HeapErrorCode    :=0;
end;


{$ifdef DUMPBLOCKS}   // TODO
procedure DumpBlocks;
var
  s,i,j : ptrint;
  hpfixed  : pmemchunk_fixed;
  hpvar  : pmemchunk_var;
begin
  { fixed freelist }
  for i := 1 to maxblockindex do
   begin
     hpfixed := freelists_fixed[i];
     j := 0;
     while assigned(hpfixed) do
      begin
        inc(j);
        hpfixed := hpfixed^.next_fixed;
      end;
     writeln('Block ',i*blocksize,': ',j);
   end;
  { var freelist }
  hpvar := freelist_var;
  j := 0;
  s := 0;
  while assigned(hpvar) do
   begin
     inc(j);
     if hpvar^.size>s then
      s := hpvar^.size;
     hpvar := hpvar^.next_var;
   end;
  writeln('Variable: ',j,' maxsize: ',s);
end;
{$endif}



{*****************************************************************************
                                List adding/removal
*****************************************************************************}

procedure append_to_list_var(pmc: pmemchunk_var);inline;
begin
  pmc^.prev_var := nil;
  pmc^.next_var := freelist_var;
  if freelist_var<>nil then
    freelist_var^.prev_var := pmc;
  freelist_var := pmc;
end;

procedure remove_from_list_var(pmc: pmemchunk_var);inline;
begin
  if assigned(pmc^.next_var) then
    pmc^.next_var^.prev_var := pmc^.prev_var;
  if assigned(pmc^.prev_var) then
    pmc^.prev_var^.next_var := pmc^.next_var
  else
    freelist_var := pmc^.next_var;
end;

procedure append_to_oslist(poc: poschunk);
begin
  { decide whether to free block or add to list }
{$ifdef HAS_SYSOSFREE}
  if (freeoslistcount >= MaxKeptOSChunks) or
     (poc^.size > growheapsize2) then
    begin
      dec(internal_status.currheapsize, poc^.size);
      SysOSFree(poc, poc^.size);
    end
  else
    begin
{$endif}
      poc^.prev := nil;
      poc^.next := freeoslist;
      if freeoslist <> nil then
        freeoslist^.prev := poc;
      freeoslist := poc;
      inc(freeoslistcount);
{$ifdef HAS_SYSOSFREE}
   end;
{$endif}
end;

procedure remove_from_oslist(poc: poschunk);
begin
  if assigned(poc^.next) then
    poc^.next^.prev := poc^.prev;
  if assigned(poc^.prev) then
    poc^.prev^.next := poc^.next
  else
    freeoslist := poc^.next;
  dec(freeoslistcount);
end;

procedure append_to_oslist_var(pmc: pmemchunk_var);
var
  poc: poschunk;
begin
  // block eligable for freeing
  poc := pointer(pmc)-sizeof(toschunk);
  remove_from_list_var(pmc);
  append_to_oslist(poc);
end;

procedure append_to_oslist_fixed(poc: poschunk);
var
  pmc: pmemchunk_fixed;
  chunksize,
  chunkindex,
  i, count: ptrint;
begin
  chunkindex:=poc^.chunkindex;
  chunksize:=chunkindex shl blockshift;
  pmc := pmemchunk_fixed(pointer(poc)+sizeof(toschunk));
  count := (poc^.size - sizeof(toschunk)) div chunksize;
  for i := 0 to count - 1 do
    begin
      if assigned(pmc^.next_fixed) then
        pmc^.next_fixed^.prev_fixed := pmc^.prev_fixed;
      if assigned(pmc^.prev_fixed) then
        pmc^.prev_fixed^.next_fixed := pmc^.next_fixed
      else
        freelists_fixed[chunkindex] := pmc^.next_fixed;
      pmc := pointer(pmc)+chunksize;
    end;
  append_to_oslist(poc);
end;

{*****************************************************************************
                         Split block
*****************************************************************************}

procedure split_block(pcurr: pmemchunk_var; size: ptrint);
var
  pcurr_tmp : pmemchunk_var;
  sizeleft: ptrint;
begin
  sizeleft := (pcurr^.size and sizemask)-size;
  if sizeleft>=blocksize then
    begin
      pcurr_tmp := pmemchunk_var(pointer(pcurr)+size);
      { update prevsize of block to the right }
      if (pcurr^.size and lastblockflag) = 0 then
        pmemchunk_var(pointer(pcurr)+(pcurr^.size and sizemask))^.prevsize := sizeleft;
      { inherit the lastblockflag }
      pcurr_tmp^.size := sizeleft or (pcurr^.size and lastblockflag);
      pcurr_tmp^.prevsize := size;
      { the block we return is not the last one anymore (there's now a block after it) }
      { decrease size of block to new size }
      pcurr^.size := size or (pcurr^.size and (not sizemask and not lastblockflag));
      { insert the block in the freelist }
      append_to_list_var(pcurr_tmp);
    end;
end;


{*****************************************************************************
                         Try concat freerecords
*****************************************************************************}

procedure concat_two_blocks(mc_left, mc_right: pmemchunk_var);
var
  mc_tmp : pmemchunk_var;
  size_right : ptrint;
begin
  // mc_right can't be a fixed size block
  if mc_right^.size and fixedsizeflag<>0 then
    HandleError(204);
  // left block free, concat with right-block
  size_right := mc_right^.size and sizemask;
  inc(mc_left^.size, size_right);
  // if right-block was last block, copy flag
  if (mc_right^.size and lastblockflag) <> 0 then
    begin
      mc_left^.size := mc_left^.size or lastblockflag;
    end
  else
    begin
      // there is a block to the right of the right-block, adjust it's prevsize
      mc_tmp := pmemchunk_var(pointer(mc_right)+size_right);
      mc_tmp^.prevsize := mc_left^.size and sizemask;
    end;
  // remove right-block from doubly linked list
  remove_from_list_var(mc_right);
end;

procedure try_concat_free_chunk_forward(mc: pmemchunk_var);
var
  mc_tmp : pmemchunk_var;
begin
  { try concat forward }
  if (mc^.size and lastblockflag) = 0 then
   begin
     mc_tmp := pmemchunk_var(pointer(mc)+(mc^.size and sizemask));
     if (mc_tmp^.size and usedflag) = 0 then
       begin
         // next block free: concat
         concat_two_blocks(mc, mc_tmp);
       end;
   end;
end;

function try_concat_free_chunk(mc: pmemchunk_var): pmemchunk_var;
var
  mc_tmp : pmemchunk_var;
begin
  try_concat_free_chunk_forward(mc);

  { try concat backward }
  if (mc^.size and firstblockflag) = 0 then
    begin
      mc_tmp := pmemchunk_var(pointer(mc)-mc^.prevsize);
      if (mc_tmp^.size and usedflag) = 0 then
        begin
          // prior block free: concat
          concat_two_blocks(mc_tmp, mc);
          mc := mc_tmp;
        end;
    end;

  result := mc;
end;


function check_concat_free_chunk_forward(mc: pmemchunk_var;reqsize:ptrint):boolean;
var
  mc_tmp : pmemchunk_var;
  freesize : ptrint;
begin
  check_concat_free_chunk_forward:=false;
  freesize:=0;
  mc_tmp:=mc;
  repeat
     inc(freesize,mc_tmp^.size and sizemask);
     if freesize>=reqsize then
       begin
         check_concat_free_chunk_forward:=true;
         exit;
       end;
     if (mc_tmp^.size and lastblockflag) <> 0 then
       break;
     mc_tmp := pmemchunk_var(pointer(mc_tmp)+(mc_tmp^.size and sizemask));
     if (mc_tmp^.size and usedflag) <> 0 then
       break;
  until false;
end;


{*****************************************************************************
                                Grow Heap
*****************************************************************************}

function alloc_oschunk(chunkindex, size: ptrint):pointer;
var
  pmcfirst,
  pmclast,
  pmc       : pmemchunk_fixed;
  pmcv      : pmemchunk_var;
  poc       : poschunk;
  chunksize,
  minsize,
  maxsize,
  i, count  : ptrint;
begin
  result:=nil;
  chunksize:=chunkindex shl blockshift;
  { increase size by size needed for os block header }
  minsize := size + sizeof(toschunk);
  if chunkindex<>0 then
    maxsize := (chunksize * $ffff) + sizeof(toschunk)
  else
    maxsize := high(ptrint);
  { blocks available in freelist? }
  poc := freeoslist;
  while poc <> nil do
    begin
      if (poc^.size >= minsize) and
         (poc^.size <= maxsize) then
        begin
          size := poc^.size;
          remove_from_oslist(poc);
          break;
        end;
      poc := poc^.next;
    end;
  if poc = nil then
    begin
{$ifdef DUMPGROW}
      writeln('growheap(',size,')  allocating ',(size+sizeof(toschunk)+$ffff) and $ffff0000);
      DumpBlocks;
{$endif}
      { allocate by 64K size }
      size := (size+sizeof(toschunk)+$ffff) and not $ffff;
      { allocate smaller blocks for fixed-size chunks }
      if chunksize<>0 then
        begin
          poc := SysOSAlloc(GrowHeapSizeSmall);
          if poc<>nil then
            size := GrowHeapSizeSmall;
        end
      { first try 256K (default) }
      else if size<=GrowHeapSize1 then
        begin
          poc := SysOSAlloc(GrowHeapSize1);
          if poc<>nil then
            size := GrowHeapSize1;
        end
      { second try 1024K (default) }
      else if size<=GrowHeapSize2 then
        begin
          poc := SysOSAlloc(GrowHeapSize2);
          if poc<>nil then
            size := GrowHeapSize2;
        end
      { else allocate the needed bytes }
      else
        poc := SysOSAlloc(size);
      { try again }
      if poc=nil then
        begin
          poc := SysOSAlloc(size);
          if (poc=nil) then
            begin
              if ReturnNilIfGrowHeapFails then
                exit
              else
                HandleError(203);
            end;
        end;
      { set the total new heap size }
      inc(internal_status.currheapsize,size);
      if internal_status.currheapsize>internal_status.maxheapsize then
        internal_status.maxheapsize:=internal_status.currheapsize;
    end;
  { initialize os-block }
  poc^.used := 0;
  poc^.size := size;
  poc^.chunkindex := chunkindex;
  { initialized oschunck for fixed chunks }
  if chunkindex<>0 then
    begin
      { chop os chunk in fixedsize parts,
        maximum of $ffff elements are allowed, otherwise
        there will be an overflow }
      count := (size-sizeof(toschunk)) div chunksize;
      if count>$ffff then
        HandleError(204);
      { Initialize linkedlist of chunks, the first chunk
        is pmemchunk_fixed(poc) and the last chunk will be in pmc at
        the end of the loop }
      pmcfirst := pmemchunk_fixed(pointer(poc)+sizeof(toschunk));
      pmc:=pmcfirst;
      for i:=1 to count do
        begin
          pmc^.poc:=poc;
          pmc^.size:=chunksize or fixedsizeflag;
          pmc^.prev_fixed := pointer(pmc)-chunksize;
          pmc^.next_fixed := pointer(pmc)+chunksize;
          pmc := pmemchunk_fixed(pointer(pmc)+chunksize);
        end;
      { undo last increase to get last chunk }
      pmclast := pmemchunk_fixed(pointer(pmc)-chunksize);
      { Add to freelist and fixup first and last chunk }
      pmclast^.next_fixed := freelists_fixed[chunkindex];
      if freelists_fixed[chunkindex]<>nil then
        freelists_fixed[chunkindex]^.prev_fixed := pmclast;
      freelists_fixed[chunkindex] := pmcfirst;
      pmemchunk_fixed(poc)^.prev_fixed:=nil;
      result:=pmcfirst;
    end
  else
    begin
      pmcv := pmemchunk_var(pointer(poc)+sizeof(toschunk));
      append_to_list_var(pmcv);
      pmcv^.size := ((size-sizeof(toschunk)) and sizemask) or (firstblockflag or lastblockflag);
      pmcv^.prevsize := 0;
      result:=pmcv;
    end;
end;


{*****************************************************************************
                                 SysGetMem
*****************************************************************************}

function SysGetMem_Fixed(size: ptrint): pointer;
var
  pmc,hp : pmemchunk_fixed;
  poc : poschunk;
  chunkindex : ptrint;
begin
  { try to find a block in one of the freelists per size }
  chunkindex := size shr blockshift;
  pmc := freelists_fixed[chunkindex];
  result:=nil;
  { no free blocks ? }
  if not assigned(pmc) then
    begin
      pmc:=alloc_oschunk(chunkindex, size);
      if not assigned(pmc) then
        exit;
    end;
  { get a pointer to the block we should return }
  result := pointer(pmc)+sizeof(tmemchunk_fixed_hdr);
  { update freelist }
  hp:=pmc^.next_fixed;
  poc := pmc^.poc;
  freelists_fixed[chunkindex] := hp;
  if assigned(hp) then
    hp^.prev_fixed := nil;
  if (poc^.used = 0) then
    freelists_free_chunk[chunkindex] := false;
  inc(poc^.used);
  { statistics }
  inc(internal_status.currheapused,size);
  if internal_status.currheapused>internal_status.maxheapused then
    internal_status.maxheapused:=internal_status.currheapused;
end;


function SysGetMem_Var(size: ptrint): pointer;
var
  pcurr : pmemchunk_var;
{$ifdef BESTMATCH}
  pbest : pmemchunk_var;
{$endif}
begin
  result:=nil;
{$ifdef BESTMATCH}
  pbest := nil;
{$endif}
  pcurr := freelist_var;
  while assigned(pcurr) do
    begin
{$ifdef BESTMATCH}
      if pcurr^.size=size then
        begin
          break;
        end
      else
        begin
          if (pcurr^.size>size) then
            begin
              if (not assigned(pbest)) or
                 (pcurr^.size<pbest^.size) then
               pbest := pcurr;
            end;
        end;
{$else BESTMATCH}
      if pcurr^.size>=size then
        break;
{$endif BESTMATCH}
      pcurr := pcurr^.next_var;
    end;
{$ifdef BESTMATCH}
  if not assigned(pcurr) then
    pcurr := pbest;
{$endif}

  if not assigned(pcurr) then
   begin
    // all os-chunks full, allocate a new one
    pcurr := alloc_oschunk(0, size);
    if not assigned(pcurr) then
      exit;
   end;

  { get pointer of the block we should return }
  result := pointer(pcurr)+sizeof(tmemchunk_var_hdr);
  { remove the current block from the freelist }
  remove_from_list_var(pcurr);
  { create the left over freelist block, if at least 16 bytes are free }
  split_block(pcurr, size);
  { flag block as used }
  pcurr^.size := pcurr^.size or usedflag;
  { statistics }
  inc(internal_status.currheapused,size);
  if internal_status.currheapused>internal_status.maxheapused then
    internal_status.maxheapused:=internal_status.currheapused;
end;

function SysGetMem(size : ptrint):pointer;
begin
{ Something to allocate ? }
  if size<=0 then
    begin
      { give an error for < 0 }
      if size<0 then
        HandleError(204);
      { we always need to allocate something, using heapend is not possible,
        because heappend can be changed by growheap (PFV) }
      size := 1;
    end;
{ calc to multiple of 16 after adding the needed bytes for memchunk header }
  if size <= (maxblocksize - sizeof(tmemchunk_fixed_hdr)) then
    begin
      size := (size+(sizeof(tmemchunk_fixed_hdr)+(blocksize-1))) and fixedsizemask;
      result := sysgetmem_fixed(size);
    end
  else
    begin
      size := (size+(sizeof(tmemchunk_var_hdr)+(blocksize-1))) and sizemask;
      result := sysgetmem_var(size);
    end;
end;


{*****************************************************************************
                               SysFreeMem
*****************************************************************************}

function SysFreeMem_Fixed(pmc: pmemchunk_fixed): ptrint;
var
  hp : pmemchunk_fixed;
  chunksize,
  chunkindex : ptrint;
  poc : poschunk;
begin
  poc := pmc^.poc;
  chunkindex:=poc^.chunkindex;
  chunksize:=chunkindex shl blockshift;
  { statistics }
  dec(internal_status.currheapused,chunksize);
  hp:=freelists_fixed[chunkindex];
  { insert the block in it's freelist }
  pmc^.prev_fixed := nil;
  pmc^.next_fixed := hp;
  if assigned(hp) then
    hp^.prev_fixed := pmc;
  freelists_fixed[chunkindex] := pmc;
  dec(poc^.used);
  if poc^.used <= 0 then
    begin
      { decrease used blocks count }
      if poc^.used=-1 then
        HandleError(204);
      { osblock can be freed? }
      if freelists_free_chunk[chunkindex] then
        append_to_oslist_fixed(poc)
      else
        freelists_free_chunk[chunkindex] := true;
    end;
  result := chunksize;
end;


function SysFreeMem_Var(pcurr: pmemchunk_var): ptrint;
var
  chunksize: ptrint;
begin
  chunksize := pcurr^.size and sizemask;
  dec(internal_status.currheapused,chunksize);
  { insert the block in it's freelist }
  pcurr^.size := pcurr^.size and (not usedflag);
  append_to_list_var(pcurr);
  result := chunksize;
  pcurr := try_concat_free_chunk(pcurr);
  if (pcurr^.size and (firstblockflag or lastblockflag)) = (firstblockflag or lastblockflag) then
    append_to_oslist_var(pcurr);
end;


function SysFreeMem(p: pointer): ptrint;
var
  hp : pmemchunk_fixed;
begin
  if p=nil then
    begin
      result:=0;
      exit;
    end;

  hp:=pmemchunk_fixed(p-sizeof(tmemchunk_fixed_hdr));

  { check if this is a fixed- or var-sized chunk }
  if (hp^.size and fixedsizeflag) = 0 then
    result := sysfreemem_var(pmemchunk_var(p-sizeof(tmemchunk_var_hdr)))
  else
    result := sysfreemem_fixed(hp);
end;

{*****************************************************************************
                              SysFreeMemSize
*****************************************************************************}

Function SysFreeMemSize(p: pointer; size: ptrint):ptrint;
var
  hp : pmemchunk_fixed;
begin
  SysFreeMemSize := 0;
  if p=nil then
    exit;
  if size<=0 then
    begin
      if size<0 then
        HandleError(204);
      exit;
    end;

  hp:=pmemchunk_fixed(p-sizeof(tmemchunk_fixed_hdr));
  { check if this is a fixed- or var-sized chunk. We can't check the passed
    size parameter since the block can be resized (by reallocmem) to an
    optimized value that the user doesn't know }
  if (hp^.size and fixedsizeflag) = 0 then
    result := sysfreemem_var(pmemchunk_var(p-sizeof(tmemchunk_var_hdr)))
  else
    result := sysfreemem_fixed(hp);
end;


{*****************************************************************************
                                 SysMemSize
*****************************************************************************}

function SysMemSize(p: pointer): ptrint;
begin
  result := pmemchunk_fixed(pointer(p)-sizeof(tmemchunk_fixed_hdr))^.size;
  if (result and fixedsizeflag) = 0 then
    begin
      result := SysMemSize and sizemask;
      dec(result, sizeof(tmemchunk_var_hdr));
    end
  else
    begin
      result := SysMemSize and fixedsizemask;
      dec(result, sizeof(tmemchunk_fixed_hdr));
    end;
end;


{*****************************************************************************
                                 SysAllocMem
*****************************************************************************}

function SysAllocMem(size: ptrint): pointer;
begin
  result := MemoryManager.GetMem(size);
  if result<>nil then
    FillChar(result^,MemoryManager.MemSize(result),0);
end;


{*****************************************************************************
                                 SysResizeMem
*****************************************************************************}

function SysTryResizeMem(var p: pointer; size: ptrint): boolean;
var
  chunksize,
  oldsize,
  currsize : ptrint;
  pcurr : pmemchunk_var;
begin
  SysTryResizeMem := false;

  { fix p to point to the heaprecord }
  chunksize := pmemchunk_fixed(p-sizeof(tmemchunk_fixed_hdr))^.size;

  { handle fixed memchuncks separate. Only allow resizes when the
    new size fits in the same block }
  if (chunksize and fixedsizeflag) <> 0 then
    begin
      currsize := chunksize and fixedsizemask;

      { first check if the size fits in the fixed block range to prevent
        "truncating" the size by the fixedsizemask }
      if (size <= (maxblocksize - sizeof(tmemchunk_fixed_hdr))) and
         ((size+sizeof(tmemchunk_fixed_hdr)+(blocksize-1)) and sizemask =currsize ) then
        begin
          systryresizemem:=true;
          exit;
        end;
      { we need to allocate a new fixed or var memchunck }
      exit;
    end;

  { var memchunck }
  currsize := chunksize and sizemask;
  size := (size+sizeof(tmemchunk_var_hdr)+(blocksize-1)) and sizemask;

  { is the allocated block still correct? }
  if (currsize>=size) and (size>(currsize-blocksize)) then
    begin
      SysTryResizeMem := true;
      exit;
   end;

  { get pointer to block }
  pcurr := pmemchunk_var(pointer(p)-sizeof(tmemchunk_var_hdr));
  oldsize := currsize;

  { do we need to allocate more memory ? }
  if size>currsize then
   begin
     { the size is bigger than the previous size, we need to allocated more mem.
       We first check if the blocks after the current block are free. If not then we
       simply call getmem/freemem to get the new block }
     if check_concat_free_chunk_forward(pcurr,size) then
       repeat
         concat_two_blocks(pcurr,pmemchunk_var(pointer(pcurr)+currsize));
         currsize := pcurr^.size and sizemask;
       until currsize>=size
     else
       exit;
   end;
  { is the size smaller then we can adjust the block to that size and insert
    the other part into the freelist }
  if currsize>size then
    split_block(pcurr, size);

  inc(internal_status.currheapused,size-oldsize);
  SysTryResizeMem := true;
end;


{*****************************************************************************
                                 SysResizeMem
*****************************************************************************}

function SysReAllocMem(var p: pointer; size: ptrint):pointer;
var
  newsize,
  oldsize,
  minsize : ptrint;
  p2 : pointer;
begin
  { Free block? }
  if size=0 then
   begin
     if p<>nil then
      begin
        MemoryManager.FreeMem(p);
        p := nil;
      end;
   end
  else
   { Allocate a new block? }
   if p=nil then
    begin
      p := MemoryManager.GetMem(size);
    end
  else
   { Resize block }
   if not SysTryResizeMem(p,size) then
    begin
      oldsize:=MemoryManager.MemSize(p);
      { Grow with bigger steps to prevent the need for
        multiple getmem/freemem calls for fixed blocks. It might cost a bit
        of extra memory, but in most cases a reallocmem is done multiple times. }
      if oldsize<maxblocksize then
        begin
          newsize:=oldsize*2+blocksize;
          if size>newsize then
            newsize:=size;
        end
      else
        newsize:=size;
      { calc size of data to move }
      minsize:=oldsize;
      if newsize < minsize then
        minsize := newsize;
      p2 := MemoryManager.GetMem(newsize);
      if p2<>nil then
        Move(p^,p2^,minsize);
      MemoryManager.FreeMem(p);
      p := p2;
    end;
  SysReAllocMem := p;
end;


{*****************************************************************************
                       MemoryMutexManager default hooks
*****************************************************************************}

procedure SysHeapMutexInit;
begin
  { nothing todo }
end;

procedure SysHeapMutexDone;
begin
  { nothing todo }
end;

procedure SysHeapMutexLock;
begin
  { give an runtime error. the program is running multithreaded without
    any heap protection. this will result in unpredictable errors so
    stopping here with an error is more safe (PFV) }
  runerror(244);
end;

procedure SysHeapMutexUnLock;
begin
  { see SysHeapMutexLock for comment }
  runerror(244);
end;


{*****************************************************************************
                                 InitHeap
*****************************************************************************}

{ This function will initialize the Heap manager and need to be called from
  the initialization of the system unit }
procedure InitHeap;
begin
  FillChar(freelists_fixed,sizeof(tfreelists),0);
  FillChar(freelists_free_chunk,sizeof(freelists_free_chunk),0);
  freelist_var := nil;
  freeoslist := nil;
  freeoslistcount := 0;
  fillchar(internal_status,sizeof(internal_status),0);
end;
