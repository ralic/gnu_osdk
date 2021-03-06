{
    This file is part of the Free Pascal run time library.
    Copyright (c) 1999-2001 by the Free Pascal development team

    Implementation of mathematical routines (for extended type)

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{****************************************************************************
                            FPU Control word
 ****************************************************************************}

    procedure Set8087CW(cw:word);assembler;
    asm
{$ifndef REGCALL}
      movw cw,%ax
{$endif}
      movw %ax,default8087cw
      fnclex
      fldcw default8087cw
    end;

    function Get8087CW:word;assembler;
    asm
      pushl $0
      fnstcw (%esp)
      popl %eax
    end;

{****************************************************************************
                       EXTENDED data type routines
 ****************************************************************************}

    {$define FPC_SYSTEM_HAS_PI}
    function fpc_pi_real : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_ABS}
    function fpc_abs_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_SQR}
    function fpc_sqr_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_SQRT}
    function fpc_sqrt_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_ARCTAN}
    function fpc_arctan_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_LN}
    function fpc_ln_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_SIN}
    function fpc_sin_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;
    {$define FPC_SYSTEM_HAS_COS}
    function fpc_cos_real(d : ValReal) : ValReal;compilerproc;
    begin
      { Function is handled internal in the compiler }
      runerror(207);
      result:=0;
    end;

  {$define FPC_SYSTEM_HAS_EXP}
    function fpc_exp_real(d : ValReal) : ValReal;assembler;compilerproc;
      asm
        // comes from DJ GPP
        fldt        d
        fldl2e
        fmulp       %st,%st(1)
        fstcw      .LCW1
        fstcw      .LCW2
        fwait
        andw        $0xf3ff,.LCW2
        orw         $0x0400,.LCW2
        fldcw      .LCW2
        fld         %st(0)
        frndint
        fldcw      .LCW1
        fxch        %st(1)
        fsub        %st(1),%st
        f2xm1
        fld1
        faddp       %st,%st(1)
        fscale
        fstp        %st(1)
        fclex
        jmp         .LCW3
        // store some help data in the data segment
    .data
    .LCW1:
        .word       0
    .LCW2:
        .word       0
    .text
    .LCW3:
     end;


    {$define FPC_SYSTEM_HAS_FRAC}
    function fpc_frac_real(d : ValReal) : ValReal;assembler;compilerproc;
      asm
        subl $16,%esp
        fnstcw -4(%ebp)
        fwait
        movw -4(%ebp),%cx
        orw $0x0f00,%cx
        movw %cx,-8(%ebp)
        fldcw -8(%ebp)
        fldt d
        frndint
        fldt d
        fsub %st(1),%st
        fstp %st(1)
        fldcw -4(%ebp)
      end;


    {$define FPC_SYSTEM_HAS_INT}
    function fpc_int_real(d : ValReal) : ValReal;assembler;compilerproc;
      asm
        subl $16,%esp
        fnstcw -4(%ebp)
        fwait
        movw -4(%ebp),%cx
        orw $0x0f00,%cx
        movw %cx,-8(%ebp)
        fldcw -8(%ebp)
        fwait
        fldt d
        frndint
        fwait
        fldcw -4(%ebp)
      end;



    {$define FPC_SYSTEM_HAS_TRUNC}
    function fpc_trunc_real(d : ValReal) : int64;assembler;compilerproc;
      var
        oldcw,
        newcw : word;
        res   : int64;
      asm
        fnstcw oldcw
        fwait
        movw oldcw,%cx
        orw $0x0f00,%cx
        movw %cx,newcw
        fldcw newcw
        fldt d
        fistpq res
        fwait
        movl res,%eax
        movl res+4,%edx
        fldcw oldcw
      end;


    {$define FPC_SYSTEM_HAS_ROUND}
    function fpc_round_real(d : ValReal) : int64;assembler;compilerproc;
      var
        res   : int64;
      asm
        fldt d
        fistpq res
        fwait
        movl res,%eax
        movl res+4,%edx
      end;


    {$define FPC_SYSTEM_HAS_POWER}
   function power(bas,expo : ValReal) : ValReal;
     begin
        if bas=0 then
          begin
            if expo<>0 then
              power:=0.0
            else
              HandleError(207);
          end
        else if expo=0 then
         power:=1
        else
        { bas < 0 is not allowed when doing roots }
         if (bas<0) and (frac(expo) <> 0) then
          handleerror(207)
         else
           begin
             power:=exp(ln(abs(bas))*expo);
             if (bas < 0) and
                odd(trunc(expo)) then
               begin
                 power := -power;
               end;
           end;
     end;

