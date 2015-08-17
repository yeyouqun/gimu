;
; hardware information getter implementation.
; Author:yeyouqun@163.com
; 2010-5-20
;
PUBLIC GetCPUID
.686p
.model flat
.code
GetCPUID proc C VendorID,s1,s2,s3,s4
		xor eax,eax
		cpuid
		mov eax,VendorID
		mov dword ptr [eax+0],ebx
		mov dword ptr [eax+4],edx
		mov dword ptr [eax+8],ecx
		mov eax,01h  
		xor edx,edx  
		cpuid
		mov esi, s1
		mov dword ptr [esi],edx 
		mov esi, s2
		mov dword ptr [esi],eax  
		mov eax,03h
		xor ecx,ecx
		xor edx,edx
		cpuid
		mov esi, s3
		mov dword ptr [esi],edx
		mov esi, s4
		mov dword ptr [esi],ecx
		ret
GetCPUID endp
end
