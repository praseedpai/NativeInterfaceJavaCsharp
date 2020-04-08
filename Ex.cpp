//////////////////////////////////////////
//
// A Simple Program to demonstrate execution
// of x86 instruction from a buffer
//
// Written By Praseed Pai K.T
//            http://praseedp.blogspot.com
//
//
//

long Add( long a , long b ) {

    return a + b;
}


char addfunc[] = { (char)0x55,                 // push EBP 
                   (char)0x8B, (char)0xEC,           // mov ebp,esp   
                   (char)0x8B, (char)0x45 , (char)0x08,    // mov eax,dword ptr [ebp+8] 
                   (char)0x03, (char)0x45 , (char)0x0c,    // add eax,dword ptr [ebp+12]
                   (char)0x5D,                 // pop ebp
                   (char)0xC3                  // ret
                 };


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef long (*BIN_FUNC)( long a , long b );

int main( int argc , char **argv )
{

    DWORD oldprotect ;

    VirtualProtect(addfunc, 128L , PAGE_EXECUTE, &oldprotect);
    BIN_FUNC func  = (BIN_FUNC)((void *)addfunc); 
    FlushInstructionCache(GetCurrentProcess(),0,0);
    long c = (*func)(4,3);    
    VirtualProtect(addfunc, 128L, oldprotect, &oldprotect);
    printf("%d\n",c );


}