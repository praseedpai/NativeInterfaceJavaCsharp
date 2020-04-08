/////////////////////////////////
//
// A Simple X86 based C/C++ program which 
// injects two long values into the instruction stream
//
//
// Written by Praseed Pai K.T.
//            http://praseedp.blogspot.com
//
//


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

char addfunc[] = { (char)0xB8,0x0,0x0,0x0,0x0 ,                
                   (char)0x05, 0x0,0x0,0x0,0x0,           
                   (char)0xC3 };



typedef long (*BIN_FUNC)( );

int main( int argc , char **argv )
{

    DWORD oldprotect ;
    long a = 4;
    long b = 6;
    char *code = ( char *)malloc(128);
    memcpy(code,addfunc,sizeof(addfunc));
    memcpy(code+1,&a,sizeof(long));
    memcpy(code+6,&b,sizeof(long));    
    VirtualProtect(code, 128 , PAGE_EXECUTE, &oldprotect);
    BIN_FUNC func  = (BIN_FUNC)((void *)code); 
    FlushInstructionCache(GetCurrentProcess(),0,0);
    long c = (*func)();    
    VirtualProtect(code, 128, oldprotect, &oldprotect);
    free(code);
    printf("%d\n",c );


}