; i386/pit.asm
; Copyrights (c) 2005 Mohamed Abdelsalam Aslan <maslan@users.berlios.de>
;
; This library is free software; you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation; either
; version 2.1 of the License, or (at your option) any later version.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Lesser General Public License for more details.
;
; You should have received a copy of the GNU Lesser General Public
; License along with this library; if not, write to the Free Software
; Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

bits 32

global osdk_timerhz

osdk_timerhz:
	push	ebp
	mov	ebp, esp
	push	eax
	push	ebx
	push	edx
	mov	al, 36h
	out	43h, al
	mov	bx, word [ebp+8]
	mov	ax, 34dch 
	mov	dx, 0012h
	div	bx
	out	40h, al
	mov	al, ah
	out	40h, al
	pop	edx
	pop	ebx
	pop	eax
	mov	esp, ebp
	pop	ebp
	ret