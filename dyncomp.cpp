/////////////////////////////
//
//
// 
//
//
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <io.h>
#include <math.h>

#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std ;

enum SYMB_KIND  {  constant , variable };
enum SYMB_TYPE  {  long_type , double_type , bool_type ,unspecified_type };
enum SYMB_FORM { s_atomic , s_array };

typedef struct 
{

  char         SymbolName[128];
  SYMB_KIND    smb_kind; 
  SYMB_TYPE    smb_type;
  SYMB_FORM    smb_form;

  union 
  {

     long offset_from_EBP;

  }ADDRESS;

}SYMBOL_INFO;

class SymbolTableManager 
{
 
  map<string,SYMBOL_INFO >   val;
  int LabelIndexValue ;

 public:

   SymbolTableManager();
   ~SymbolTableManager();
   BOOL AddSymbol( SYMBOL_INFO *new_sym );
   SYMBOL_INFO * LookupSymbol(char *Name);
   long  GetSymbolCount();
       
};


SymbolTableManager::SymbolTableManager() 
{
   LabelIndexValue = 900;
};

SymbolTableManager::~SymbolTableManager() 
{
       

}



BOOL SymbolTableManager::AddSymbol( SYMBOL_INFO *new_sym )
{
        new_sym->ADDRESS.offset_from_EBP = GetSymbolCount();
        val.insert( pair<const string , SYMBOL_INFO>(
		new_sym->SymbolName , *new_sym ) );
         

        return TRUE;
	
} 


SYMBOL_INFO * SymbolTableManager::LookupSymbol(char *Name)
{

        map<string,SYMBOL_INFO >::iterator i;

		return   ( ( i = val.find( Name ) ) != val.end() ) ?
			reinterpret_cast<SYMBOL_INFO *> (&((*i).second)) :
		    reinterpret_cast<SYMBOL_INFO *> (NULL);
			

		
} 


long  SymbolTableManager::GetSymbolCount()
  {
    return val.size();
  }




typedef double (*CompiledCode)();
unsigned char *Code;
int  ip;


void    ParseDoubleDeclaration();
void   ParseSetStatement();
void   ParseReturnStatement();



enum X86_GEN_REG32
{
  REG32_INV = -1,  // Invalid Register
  REG32_EAX =  0,
  REG32_ECX =  1,
  REG32_EDX =  2,
  REG32_EBX =  3,
  REG32_ESP =  4,
  REG32_EBP =  5,
  REG32_ESI =  6,
  REG32_EDI =  7
};


///////////////////////////////////////////
//
//
//
//
//
//
//

struct reg_names32{
     char name[10];
     X86_GEN_REG32 re;           
}REGNAMES[] =
    { {"EAX",REG32_EAX },
      {"ECX",REG32_ECX },     
      {"EDX",REG32_EDX },
      {"EBX",REG32_EBX },
      {"ESP",REG32_ESP },
      {"EBP",REG32_EBP },
      {"ESI",REG32_ESI },
      {"EDI",REG32_EDI }
    };




int FindOrdinal(char *regname )
{

   X86_GEN_REG32 temp_value = REG32_INV;
   for(int i=0;i< 8; ++i )
   {
       if ( stricmp(REGNAMES[i].name,regname ) == 0 )
        {
                 temp_value = REGNAMES[i].re;  
                 break;
        } 

   }

   if ( temp_value == REG32_INV )
   {
           fprintf(stdout,"Invalid Register\n");
           exit(0);   
   }

   return (int)temp_value;
       


}



void X86_PUSH_REG32(char *RegName )
{

  Code[ip++]=(char)(0x50 + (long ) FindOrdinal(RegName));

}



void X86_POP_REG32(char *RegName )
{

   Code[ip++]=(char)(0x58 + (long ) FindOrdinal(RegName));

}


void X86_PUSH_VALUE32(long value)
{
     
   Code[ip++]=0x68;
   *((long *)(Code + ip)) = value;
   ip+=4;

}


void X86_MOV_REG32_VALUE32( char *RegName,long value )
{
   Code[ip++] = 0xB8 + (int)FindOrdinal(RegName);
   *((long *)(Code + ip)) = value;
   ip+=4;
   
}



void X86_MOV_REG32_REG32(char *RegOne,char *RegTwo )
{
        long rone = FindOrdinal(RegOne);
        long rtwo = FindOrdinal(RegTwo);

        Code[ip++] = 0x8B;
        Code[ip++]=  0xC0 | rone<<3 | rtwo;


}


void X86_ADD_REG32_VALUE32(char *RegName , long value32 )
{
   long reg = FindOrdinal(RegName);

   Code[ip++]= 0x81;
   Code[ip++]= 0xC0 + (reg&0x7);
   *((long *)(Code + ip)) = value32;
   ip+=4;

}

void X86_SUB_REG32_VALUE32(char *RegName , long value32 )
{
   long reg = FindOrdinal(RegName);

   Code[ip++]= 0x81;
   Code[ip++]= 0xC0 + (5 << 3 ) + (reg&0x7);
   *((long *)(Code + ip)) = value32;
   ip+=4;

}



void X86_MOV_PTRREG32_REG32( char *RegOne,char *RegTwo )
{
  long reg01 = FindOrdinal(RegOne);
  long reg02 = FindOrdinal(RegTwo);

  ///////////////////////////
  //
  //  OpCode for Move
  //
  Code[ip++] = 0x89;  

  //////////////////////////////
  //
  //  MOD REG R/M BYTE
  //  MOD := 00b 
  //  REG := Ordinal of Source Register (0-7)
  //  R/M  has to be 100b to indicate next byte is SIB

  Code[ip++] = ( (reg02&0x7) << 3 ) | 0x4;

  //////////////////////////////////////
  //  SIB Byte
  //  
  //  Scale := 00b ie 1
  //  Index := 100b  Ie no index
  //  BASE :=  Register number

  Code[ip++] = (4 << 3 ) | reg01 ;
 
   

}

void X86_MOV_PTRREG32PLUSOFFSET_REG32( char *RegOne,long offset , char *RegTwo )
{
  long reg01 = FindOrdinal(RegOne);
  long reg02 = FindOrdinal(RegTwo);
  long mod =0;

  ///////////////////////////
  //
  //  OpCode for Move
  //
  Code[ip++] = 0x89;  


  ////////////////////////////////////////////////////
  //
  //
  //

  mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;
  
   



  //////////////////////////////
  //
  //  MOD REG R/M BYTE
  //  MOD := 00b 
  //  REG := Ordinal of Source Register (0-7)
  //  R/M  has to be 100b to indicate next byte is SIB

  Code[ip++] = ((mod << 6 ) |  (reg02&0x7) << 3 ) | 0x4;

  //////////////////////////////////////
  //  SIB Byte
  //  
  //  Scale := 00b ie 1
  //  Index := 100b  Ie no index
  //  BASE :=  Register number

  Code[ip++] = (4 << 3 ) | reg01 ;
           
  if ( mod == 0x1 ) {
         Code[ip++] = offset; 
  }
  else if (mod == 0x2 ) {
   
          *((long *)(Code + ip)) = offset;
          ip+=4;
  }
  

}



void X86_CDQ()
{
   Code[ip++]= 0x99;

}


void X86_IMUL_REG32_PTRREG32PLUSOFFSET(char *RegOne,char *RegTwo,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long reg02 = FindOrdinal(RegTwo);
    long mod =0;

    Code[ip++] = 0x0F;
    Code[ip++] = 0xAF;

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  (reg01&0x7) << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg02 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
          *((long *)(Code + ip)) = offset;
          ip+=4;
  
    }
  
    

}


void X86_IDIV_REG32(char *RegOne)
{
    
    Code[ip++] = 0xF7;
    Code[ip++] = 248 + (long)FindOrdinal(RegOne);
    
}

void X86_ADD_PTRREG32PLUSOFFSET_REG32( char *RegOne,long offset , char *RegTwo )
{
  long reg01 = FindOrdinal(RegOne);
  long reg02 = FindOrdinal(RegTwo);
  long mod =0;

  ///////////////////////////
  //
  //  OpCode for add
  //

  Code[ip++] = 0x1; 

   ////////////////////////////////////////////////////
  //
  //
  //

  mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;
  
   



  //////////////////////////////
  //
  //  MOD REG R/M BYTE
  //  MOD := 00b 
  //  REG := Ordinal of Source Register (0-7)
  //  R/M  has to be 100b to indicate next byte is SIB

  Code[ip++] = ((mod << 6 ) |  (reg02&0x7) << 3 ) | 0x4;

  //////////////////////////////////////
  //  SIB Byte
  //  
  //  Scale := 00b ie 1
  //  Index := 100b  Ie no index
  //  BASE :=  Register number

  Code[ip++] = (4 << 3 ) | reg01 ;
           
  if ( mod == 0x1 ) {
         Code[ip++] = offset; 
  }
  else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
  }
  



}


void X86_MOV_REG32_PTRREG32PLUSOFFSET(char *RegOne,char *RegTwo,long offset )
{
    long reg01 = FindOrdinal(RegOne);
    long reg02 = FindOrdinal(RegTwo);
    long mod =0;

    Code[ip++] = 0x8B;  


    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  (reg01&0x7) << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg02 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }
  
  

}



void X86_NEG_REG32(char *RegOne)
{
      Code[ip++]=0xF7;
      Code[ip++]= 0xD8 + (long)FindOrdinal(RegOne);

}


void X86_PUSH_EBP()
{
   Code[ip++] = 0x55;

} 

void X86_MOV_EBP_ESP()
{

   Code[ip++]= 0x8B;
   Code[ip++]= 0xEC;

}

void X86_MOV_ESP_EBP()
{

   Code[ip++]=0x8B;
   Code[ip++]=0xE5;

}


void X86_POP_EBP()
{

   Code[ip++] = 0x5D;
}

void X86_RET_EAX()
{
   Code[ip++] = 0xC3;

}


void X86_XOR_EAX_EAX()
{
   Code[ip++] = 0x33;
   Code[ip++] = 0xC0;
}

void X86_PUSH_EAX()
{
   Code[ip++]= 0x50;  

}

void X86_PUSH_ECX()
{

   Code[ip++]= 0x51;  
  
}


void X86_NOP()
{

   Code[ip++] = 0x90;

}



///////////////////////////////////////////////////////////
//
//
//
//
//
//
//


void X87_FLD_REAL64PLUSOFFSET(char *RegOne,long offset)
{

    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDD;  

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x0 << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg01 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }
  

}


void X87_FSTP_REAL64PLUSOFFSET(char *RegOne,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDD;  

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x3 << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg01 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }

}

void X87_FADD_REAL64PLUSOFFSET(char *RegOne,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDC;  

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x0 << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg01 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }

}


void X87_FSUB_REAL64PLUSOFFSET(char *RegOne,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDC;  

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x4 << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg01 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }

}


void X87_FMUL_REAL64PLUSOFFSET(char *RegOne,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDC;  

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x1 << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg01 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }

}


void X87_FDIV_REAL64PLUSOFFSET(char *RegOne,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDC;  

    mod = (offset == 0 ) ? 0x0 : (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x6 << 3 ) | 0x4;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    Code[ip++] = (4 << 3 ) | reg01 ;
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }
  

}


///////////////////////////////////////////////////////////
//
//
//
//
//
//
//


void X87_FLD_REAL64PLUSOFFSET_EBP(char *RegOne,long offset)
{

    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDD;  

    mod = (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x0 << 3 ) | 0x5;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }
  

}

void X87_FSTP_REAL64PLUSOFFSET_EBP(char *RegOne,long offset)
{
    long reg01 = FindOrdinal(RegOne);
    long mod =0;

    Code[ip++] = 0xDD;  

    mod = (offset >=-127 && offset < 255 ) ? 0x1 : 0x2;

    //////////////////////////////
    //
    //  MOD REG R/M BYTE
    //  MOD := 00b 
    //  REG := Ordinal of Source Register (0-7)
    //  R/M  has to be 100b to indicate next byte is SIB

    Code[ip++] = ((mod << 6 ) |  0x3 << 3 ) | 0x5;
 
    //////////////////////////////////////
    //  SIB Byte
    //  
    //  Scale := 00b ie 1
    //  Index := 100b  Ie no index
    //  BASE :=  Register number

    
           
    if ( mod == 0x1 ) {
         Code[ip++] = offset; 
    }
    else if (mod == 0x2 ) {
   
        *((long *)(Code + ip)) = offset;
          ip+=4;
    }

}

void X87_FSIN()
{
   Code[ip++]=0xD9;
   Code[ip++]=0xFE;

}

void X87_FCOS()
{
   Code[ip++]=0xD9;
   Code[ip++]=0xFF;

}

void X87_FTAN()
{


}

void X87_FABS()
{



}



void term();
void factor();
void expr_prime();

//////////////////////////////////////////////
//
//
//
//
//
long FloatValueasLong(float x)
{
   return (*(long * )(void *)&x);

}

long HIGH_DWORD_DOUBLE( double d )
{
      double *temp = (double *) &d;

      long *lng = (long *)((void *) temp);
      return lng[1];
}

long LOW_DWORD_DOUBLE( double d )
{
      double *temp = (double *) &d;

      long *lng = (long *)((void *) temp);
      return lng[0];
}




////////////////////////////
//
//
//

enum TOK_APP
{

  TOK_ILLEGAL = -1,
  TOK_NULL_VALUE = 0,
  TOK_MUL = 1 ,
  TOK_DIV,
  TOK_ADD,
  TOK_SUB,
  TOK_EOS,  
  TOK_FLOAT,
  TOK_LONG,
  TOK_DOUBLE,
  TOK_OPAREN,
  TOK_CPAREN,
  TOK_DT_LONG,
  TOK_DT_DOUBLE,
  TOK_DT_FLOAT,
  TOK_SEMICOLON,
  TOK_UNQUOTED_STRING,
  TOK_COMMA,
  TOK_EQUAL,
  TOK_SET,
  TOK_RETURN ,
  TOK_FUNC_START,
  TOK_SIN,
  TOK_COS,
  TOK_TAN,
  TOK_ABS,
  TOK_PI,
  TOK_ASIN,
  TOK_ACOS,
  TOK_ATAN,
  TOK_EXP,
  TOK_LOG,
  TOK_FUNC_END    
};

struct CURR_VALUE 
{
  TOK_APP tk;
  double   x; 
  char    String[100];
};



void Push(char *reg)
{

    //  fprintf(stdout, "push %s\n",reg );   
    X86_PUSH_REG32(reg);   
}

void Push(long x )
{
  //fprintf(stdout,"MOV EAX ,%d\n  push EAX\n" , x );
    X86_PUSH_VALUE32(x);
  
}

void Push(float x )
{

 // fprintf(stdout,"MOV EAX,%d\n PUSH EAX\n",FloatValueasLong(x));

 X86_PUSH_VALUE32(FloatValueasLong(x));  


}

void Push(double x )
{
   // fprintf(stdout,"MOV EAX,%d\n PUSH EAX\n",HIGH_DWORD_DOUBLE(x));
   // fprintf(stdout,"MOV EAX,%d\n PUSH EAX\n",LOW_DWORD_DOUBLE(x));

   X86_PUSH_VALUE32(HIGH_DWORD_DOUBLE(x)); 
   X86_PUSH_VALUE32(LOW_DWORD_DOUBLE(x));    
   

}

void Pop(char *reg)
{
  // fprintf(stdout, "pop %s\n",reg );   

   X86_POP_REG32(reg);

}




void Mul()
{

   ////////////////////////////////////////////////////
   // fprintf(stdout," pop	eax		\n");
   //

   Pop("EAX"); 

   ///////////////////////////////////////////////////// 
   // fprintf(stdout," imul	eax, [esp]\n");
   //
   //

   X86_IMUL_REG32_PTRREG32PLUSOFFSET("EAX","ESP",0);
    
   /////////////////////////////////////////////////////////
   //  fprintf(stdout," mov	[esp], eax\n" );

   X86_MOV_PTRREG32_REG32("ESP","EAX");
}


void MulF()
{
  //////////////////////////////////////////////
  //fprintf(stdout," fld   QWORD  [esp]	\n");
  //
  //

  X87_FLD_REAL64PLUSOFFSET("ESP",0);

  

 // fprintf(stdout," fmul  QWORD  [ESP+8]\n");

  X87_FMUL_REAL64PLUSOFFSET("ESP",8);

//  fprintf(stdout," ADD ESP, 8 \n");

  X86_ADD_REG32_VALUE32("ESP",8);

//  fprintf(stdout," fstp  QWORD  [esp]\n" );

    X87_FSTP_REAL64PLUSOFFSET("ESP",0);

}


void Div()
{
   ////////////////////////////////////////////////////
   //fprintf(stdout, " pop	ebx		\n");
   //

   Pop("EBX");

   //////////////////////////////////////////////////////
   //
   // fprintf(stdout,"  mov	eax, [esp]\n");
   //

   X86_MOV_REG32_PTRREG32PLUSOFFSET("EAX","ESP",0);

   ///////////////////////////////////////////////
   // fprintf(stdout,"  cdq\n" );
   //
   X86_CDQ();
 
   //////////////////////////////////////////////////////  
   // fprintf(stdout,"  idiv	ebx\n");
   //

   X86_IDIV_REG32("EBX");
   
   ////////////////////////////////////////////////////////
   // fprintf(stdout,"  mov	[esp], eax\n");
   //
   //
   X86_MOV_PTRREG32_REG32("ESP","EAX");
   
}


void DivF()
{

  ////////////////////////////////////////////////
  //fprintf(stdout," fld   QWORD  [esp+8]	\n");
  //

  X87_FLD_REAL64PLUSOFFSET("ESP",8);

 // fprintf(stdout," fdiv  QWORD  [ESP]\n");
  X87_FDIV_REAL64PLUSOFFSET("ESP",0);

//  fprintf(stdout,"ADD ESP, 8 \n");

  X86_ADD_REG32_VALUE32("ESP",8);   

//  fprintf(stdout," fstp  QWORD  [esp]\n" );

  X87_FSTP_REAL64PLUSOFFSET("ESP",0);

}

void Add()
{
   ///////////////////////////////////////////////////
   // fprintf(stdout,"pop	eax		\n");
   //
   Pop("EAX");

   ///////////////////////////////////////////////
   //  fprintf(stdout,"add	[esp], eax\n");
   //
   X86_ADD_PTRREG32PLUSOFFSET_REG32("ESP",0,"EAX");


}


void AddF()
{

  /////////////////////////////////////////////
  //fprintf(stdout," fld   QWORD  [esp]	\n");

  X87_FLD_REAL64PLUSOFFSET("ESP",0);

  //fprintf(stdout," fadd  QWORD  [ESP+8]\n");
  X87_FADD_REAL64PLUSOFFSET("ESP",8);
  
  //fprintf(stdout," ADD  ESP,8 \n"); 

  X86_ADD_REG32_VALUE32("ESP",8);  
  
  /////////////////////////////////////////
  //fprintf(stdout," fstp  QWORD  [esp]\n" );
  X87_FSTP_REAL64PLUSOFFSET("ESP",0);
}

void Sub()
{
 /////////////////////////////////////////////// 
 // fprintf(stdout,"pop	eax \n");
 //
 //

 Pop("EAX");

 ////////////////////////////////////////
 // fprintf(stdout,"neg	eax\n" );
 //

 X86_NEG_REG32("EAX");  
 

 //////////////////////////////////////////////
 // fprintf(stdout,"add	[esp], eax\n");
 //
 X86_ADD_PTRREG32PLUSOFFSET_REG32("ESP",0,"EAX");

 
}

void SubF()
{
  //////////////////////////////////////////// 
  //fprintf(stdout," fld   QWORD  [esp+8]	\n");

  X87_FLD_REAL64PLUSOFFSET("ESP",8);

  //fprintf(stdout," fsub  QWORD [ESP]\n");

  X87_FSUB_REAL64PLUSOFFSET("ESP",0);
  
  //fprintf(stdout," ADD ESP,8 \n");
  X86_ADD_REG32_VALUE32("ESP",8);  

  //fprintf(stdout," fstp  QWORD  [esp]\n" );

  X87_FSTP_REAL64PLUSOFFSET("ESP",0);

}


void EmitPrologue( long num_items )
{
    Push("EBP");   
    X86_MOV_EBP_ESP();     
    X86_SUB_REG32_VALUE32("ESP",num_items*8 );
   
}


void EmitEpilogue( long num_items )
{
    X86_ADD_REG32_VALUE32("ESP",num_items*8  );
    X86_MOV_ESP_EBP();
    Pop("EBP");   
    X86_RET_EAX();
}

void PrintValue()
{
   printf("MOV EAX, printbfr \n");
   printf("push EAX\n");
   printf("call _printf\n"); 
   printf("ADD ESP,8\n");

}


void PrintValueF()
{
   
   
   printf("MOV EAX, printbfrf \n");
   printf("push EAX\n");
   printf("call _printf\n"); 
   printf("ADD ESP,12\n");

}

/////////////////////////////////////////////
//
//
//
//
static TOK_APP isKeyWord( char * Temp)
	 {
        const char *mtemp = Temp; 

        static struct TABLE_KEYWORD
	{
	      char *keywordname;
              TOK_APP m_tok;
	}KEYWORD_TABLE[] = {
	              
                      {"DOUBLE",TOK_DT_DOUBLE},
                      {"SET",TOK_SET},
                      {"RETURN",TOK_RETURN},
                      {"Sin", TOK_SIN},
                      {"Cos",TOK_COS},
                      {"tan",TOK_TAN},
                      {"Abs",TOK_ABS},
                      {"PI",TOK_PI},
                      {"ASIN",TOK_ASIN},
                      {"ACOS",TOK_ACOS},
                      {"ATAN",TOK_ATAN},
                      {"EXP",TOK_EXP},
                      {"LN",TOK_LOG}
                      

		};


 	  for( int i=0; i< sizeof(KEYWORD_TABLE) /
		   sizeof(KEYWORD_TABLE[0]); ++i )
	   {

		    if ( stricmp( KEYWORD_TABLE[i].keywordname ,mtemp )== 0 ){
			      return KEYWORD_TABLE[i].m_tok;
                    }        

         }


     return TOK_NULL_VALUE; 
}

///////////////////////////
//
//



struct CURR_VALUE curr_value;

char *inputstring;

char *input_ptr;
TOK_APP token_app;

SymbolTableManager SymbolTable;

TOK_APP  ExtractLongorDouble();

TOK_APP next_token()
{

 next_tok_label:

   while ( *input_ptr == ' ' || *input_ptr == '\t' )
              input_ptr++;

   switch(*input_ptr)
   {
      
       case  13: 
              input_ptr +=2;
              goto next_tok_label; 
              break;
       case 10:
	      input_ptr +=1;
	      goto next_tok_label;  		 
              break; 
       case '*':
            input_ptr++;
            return TOK_MUL;
            break;
       case '/':
            input_ptr++;
            return TOK_DIV;
            break;
       case '-':
            input_ptr++; 
            return TOK_SUB;
            break;
       case '+':
            input_ptr++; 
            return TOK_ADD;
            break; 
       case '(':
            input_ptr++;
            return TOK_OPAREN;
            break;
       case ')':
            input_ptr++;
            return TOK_CPAREN;
            break; 
       case ';':
            input_ptr++;
            return TOK_SEMICOLON; 
            break;
       case ',':
            input_ptr++;
            return TOK_COMMA; 
            break; 
       case '=':
            input_ptr++;
            return TOK_EQUAL;
            break; 
       case '0':
       case '1':
       case '2':
       case '3':	   
       case '4':
       case '5':
       case '6':
       case '7':
       case '8':
       case '9':
  
       
            if ( *input_ptr >= '0' && *input_ptr <='9' )
            {
               
               //curr_value.x = (double)(*input_ptr - '0');                     
               //input_ptr++; 
               //return TOK_DOUBLE; 

               return ExtractLongorDouble();
               

            }   
           break; 
       default: 

          if (*input_ptr == 0 )
                return TOK_EOS; 
          if (!isalpha(*input_ptr) ) 
	   {
		  return TOK_ILLEGAL;
	   }

           char *m_ptr = input_ptr;
           char TempBuffer[100];
           memset(TempBuffer,0,100);  
	  
           char *unquoted_string = TempBuffer;
	   while ( isalnum(*input_ptr) || *input_ptr == '_' )
		    *unquoted_string++ = *input_ptr++;

           strcpy(curr_value.String,TempBuffer); 
	   TOK_APP ret_val;
	   if ( ( ret_val = isKeyWord(  curr_value.String ) ) 
			   != TOK_NULL_VALUE )
			    return ret_val;

           
				  
             return    TOK_UNQUOTED_STRING;

                 


    } 


}


void StatementSequence()
{

   token_app = next_token();

   while (token_app != TOK_EOS ) 
   {

     switch(token_app)
     {
       case TOK_DT_DOUBLE:
            fprintf(stdout,"parsing decleration\n"); 
            ParseDoubleDeclaration(); 
            EmitPrologue( SymbolTable.GetSymbolCount()); 
            break; 
       case TOK_SET: 
            fprintf(stdout,"parsing assignment\n");  
            ParseSetStatement();
            break; 
       case TOK_RETURN:
            fprintf(stdout,"parsing return \n");  
            ParseReturnStatement(); 
            EmitEpilogue(SymbolTable.GetSymbolCount()); 
            break;
       default:
            fprintf(stdout,"Token = %d\n", (long)token_app );
            fprintf(stdout,"Error in Statement\n");
            exit(0);
    }

     if (TOK_RETURN == token_app )
            break;

   token_app = next_token();


  }

}

/////////////////////////////////////////
//
//
//
//
//
void    ParseDoubleDeclaration()
{
       char var_name[100];
       SYMBOL_INFO loc_symb_info;
       long nm; 

 next_var:
       token_app = next_token();

	if ( token_app != TOK_UNQUOTED_STRING ) 
	{
	     fprintf(stdout,"Variable name expected\n");		                 
	     exit(0);			 
	}

       strcpy( var_name , curr_value.String);

       if ( SymbolTable.LookupSymbol(var_name) )
	{
              fprintf(stdout,"Duplicate Declaration\n");		                                        exit(0);
			    
	}

 
	
        strcpy(loc_symb_info.SymbolName,var_name); 
	loc_symb_info.smb_kind = variable;
	loc_symb_info.smb_form = s_atomic;
        loc_symb_info.smb_type = double_type ;
        SymbolTable.AddSymbol( &loc_symb_info );
	
        if ( (token_app = next_token() ) == TOK_COMMA )           
         {
                  goto next_var;

         }


         if (token_app != TOK_SEMICOLON )
         {

             fprintf(stdout,"Semi Colon expected\n");
             exit(0);

         }  

         


}  

void   ParseSetStatement()
{

       SYMBOL_INFO *m_ptr;
         

  next_var:
       token_app = next_token();
       if ( token_app != TOK_UNQUOTED_STRING ) 
	{
	     fprintf(stdout,"Variable name expected\n");		                 
	     exit(0);			 
	}

        if ( (m_ptr = SymbolTable.LookupSymbol(curr_value.String)) == NULL)
	{	       
	       fprintf(stdout,"Variable not found\n");
               exit(0);
           
        } 


          
       token_app = next_token();

       if ( token_app != TOK_EQUAL )
       {
            fprintf(stdout,"Equal Expected\n");
            exit(0);
       }    

       expr_prime();
    //   token_app = next_token(); 
       if ( token_app != TOK_SEMICOLON )
        {
           fprintf(stdout,"SemiColon Expected\n");
           fprintf(stdout,"The Token parsed = %d\n", token_app);
           exit(0); 
        } else {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
            X87_FSTP_REAL64PLUSOFFSET_EBP("EBP",- (4+ (m_ptr->ADDRESS.offset_from_EBP+1)*8) );
            X86_POP_REG32("EAX"); 
            X86_POP_REG32("EDX"); 
            



        }       

}

void   ParseReturnStatement()
{
      next_var:

        expr_prime();
        if ( token_app != TOK_SEMICOLON )
        {
           fprintf(stdout,"SemiColon Expected\n");
           exit(0); 
        }     

        else 
        {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
            X86_POP_REG32("EAX"); 
            X86_POP_REG32("EDX"); 
            
        }          
 

} 

void ProcessSine()
{

    token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
            X87_FSIN();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          


}

void ProcessCoSine()
{
token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
            X87_FCOS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          


}

void  ProcessTan()
{
token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
            X87_FTAN();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          

}
 
void ProcessAbs()
{

token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
   //         X87_FABS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          

} 
 
void ProcessPI()
{
    token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    token_app = next_token();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
           // X87_FLDPI();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          

}
 
void ProcessAsin()
{
    token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
          //  X87_FABS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          


}



void ProcessAcos()
{
token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
        //    X87_FABS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          



} 
 
void ProcessAtan()
{

token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
            X87_FABS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          


}

void ProcessExp()
{
token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
          // X87_FABS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          


}
 
void ProcessLog()
{
token_app = next_token();
    if (token_app != TOK_OPAREN )
    {

         fprintf(stdout,"( Expected\n");
         exit(0);
    }  
    expr_prime();
    if ( token_app != TOK_CPAREN )
    {
           fprintf(stdout,") Expected\n");
           exit(0); 
    }     
    else 
    {
            X87_FLD_REAL64PLUSOFFSET("ESP",0);
       //     X87_FABS();
            X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
      
            
    }          


}
  

void DispatchFunction(TOK_APP token_app)
{

   switch(token_app)
   {
    case TOK_SIN:
    ProcessSine();    
    break;   
    case TOK_COS:
    ProcessCoSine(); 
    break;
    case TOK_TAN:
    ProcessTan();
    break;  
    case TOK_ABS:
    ProcessAbs(); 
    break;
    case TOK_PI:
    ProcessPI(); 
    break;   
    case TOK_ASIN:
    ProcessAsin();
    break;
    case TOK_ACOS:
    ProcessAcos(); 
    break;
    case TOK_ATAN:
    ProcessAtan();
    break;
    case TOK_EXP:
    ProcessExp();
    break; 
    case TOK_LOG:
    ProcessLog();
    break;
   } 


}

void expr_prime()
{

    token_app = next_token();
    int loc_token; 
    term();   

    while ( token_app == TOK_ADD || token_app  == TOK_SUB )
     {
         loc_token = token_app;

          if (token_app == TOK_ADD)
                 printf(";+\n");
          else
                 printf(";-\n");     
 
          token_app = next_token();   


          term();
         if ( loc_token == TOK_ADD )
          {
              AddF();

          }
          else {

             SubF();

         }  

     }  


  
      

}

void call_expression()
{

     expr_prime();  
     if (token_app == TOK_EOS ) 
    {
      
       // PrintValueF();
       // fprintf(stdout,"ret");
       X87_FLD_REAL64PLUSOFFSET("ESP",0);
       X86_POP_REG32("EAX"); 
       X86_POP_REG32("EDX"); 
       X86_RET_EAX();
               
    }
    else {
        printf("Error in expression");   

    }
    


}


void term()
{
    int loc_token;
  
    factor();
    while (token_app == TOK_MUL || token_app == TOK_DIV )
    {
          loc_token = token_app;
          token_app = next_token(); 
          factor();

          if ( loc_token == TOK_MUL )
          {
              MulF();

          }
          else {

             DivF();

         }  
    }


}


void factor()
{
   if ( token_app == TOK_DOUBLE )
    {
        Push( curr_value.x );
                         
    }
    else if ( token_app == TOK_OPAREN )
    {
         expr_prime();  

         if ( token_app != TOK_CPAREN )
         {
               printf("Error in Token");
               exit(0);
 
         }
         

    }
    else if ( token_app == TOK_UNQUOTED_STRING )  
    {
        SYMBOL_INFO  *m_ptr;
        m_ptr = SymbolTable.LookupSymbol(
			 curr_value.String) ;

        if ( m_ptr == NULL ) 
        {
            fprintf(stdout,"Variable not found\n");
            exit(0);
        } 

        X86_SUB_REG32_VALUE32("ESP",8);
        X87_FLD_REAL64PLUSOFFSET_EBP("EBP",-(4+ (m_ptr->ADDRESS.offset_from_EBP+1)*8));
        X87_FSTP_REAL64PLUSOFFSET("ESP",0); 
        

    }
   else if ((long) token_app > (long ) TOK_FUNC_START && (long) token_app < (long)TOK_FUNC_END)
   {
         DispatchFunction(token_app);            		        
  
   } 
   else {
          printf("Error in Token");
          exit(0);

    }

    token_app = next_token(); 


}



///////////////////////////////////////////////////
// method #5
// Extract numeric valu from the stream
// numericvalue can be long or double ( including
// exponential notation )
//
TOK_APP  ExtractLongorDouble()
{
   char Buffer[50];
   char *m_ptr = Buffer; 
   int  nctr = 0;
   int  is_dec = 0;
   char m_ch;

   while ( isdigit( ( m_ch = *input_ptr )) || 
	   toupper(m_ch) == 'E'   || 
	    m_ch == '.' ) 
   {    
           
      if ( m_ch == '.' ) 
      {
                                      
             if ( is_dec == 1 )
              	 return  TOK_ILLEGAL;    
                  
             is_dec = 1; 
             *m_ptr++ = *input_ptr++; 
             continue;                                         
      }
      else if ( toupper(m_ch) == 'E' ) 
      {
	*m_ptr =0;
	int     sign = 1;
	double  till_value = atof(Buffer);
	double  frac_value = 0.0;
	m_ptr = Buffer;
        input_ptr++;

        if ( *input_ptr == '-' ||  *input_ptr == '+' )
	 {
               if ( *input_ptr == '-' ) 
		      sign = -1;
	       input_ptr++;
	 }
	 if ( !isdigit(*input_ptr) )
	      return TOK_ILLEGAL;
                  		 
	 while ( input_ptr && isdigit(*input_ptr) )
	 	*m_ptr++ = *input_ptr++;
	 
              
 	*m_ptr = 0;
        frac_value = atof(Buffer);
        if ( till_value == 0.0 ) 
        {
	    if ( frac_value == 0.0 ) 
                  return TOK_ILLEGAL;
                         
             curr_value.x = ( sign == 1 ) ?
                     pow(10.0, (double)frac_value ) : 
					 1.0/pow(10.0,(double)frac_value);
             return TOK_DOUBLE; 
					  
        }
	else 
        {
             curr_value.x = (sign==1) ? till_value*pow(10.0, frac_value )                                                      :till_value/pow(10.0,frac_value);
             return TOK_DOUBLE;

        }


    }
			

    *m_ptr++ = *input_ptr++;

                           

  }

  *m_ptr=0;
  TOK_APP Curr_Token;
  if ( is_dec == 0 ) 
  {
    curr_value.x = (double)atol( Buffer );
    Curr_Token = TOK_DOUBLE;
       
          
  }
  else
  {
    curr_value.x  =  atof( Buffer );
    Curr_Token = TOK_DOUBLE; 
          
                     
  }

      
  return Curr_Token; 
          

}





int main(int argc , char **argv)
{

  unsigned long oldprotect;
  CompiledCode rct;
  int j; 

   printf(";;Praseed Pai's Dynamic Expression Evaluator 0.2\n");

   if (argc == 1 )
   {
      printf(";;Default arguement is 2*3+4\n");
      inputstring = "2*3+4";
   }
   else 
   { 
       inputstring = argv[1] ; 

   }

  Code = (unsigned char *) malloc(2048);
  ip=0; 
  memset(Code,0,2048);
  input_ptr = inputstring;
  StatementSequence();
  
  
  
 
  #if 0
  //=========================
  X86_PUSH_EBP();
  X86_MOV_EBP_ESP();
  X86_POP_EBP();
  X86_RET_EAX();
  //==========================
  X86_PUSH_REG32("EBP");
  X86_MOV_EBP_ESP();
  X86_POP_REG32("EBP");
  X86_RET_EAX();
  //=====================
  X86_PUSH_VALUE32(732);  
  X86_POP_REG32("EAX");
  X86_RET_EAX();


  //X86_MOV_PTRREG32_REG32("ESP","EAX"); // MOV [reg] ,reg

//  X86_MOV_PTRREG32PLUSOFFSET_REG32("ESP",0,"EAX");
 // X86_IMUL_REG32_PTRREG32PLUSOFFSET("EAX","ESP",0);
  X87_FLD_REAL64PLUSOFFSET("ESP",0);
  X87_FLD_REAL64PLUSOFFSET("ESP",8);
  X87_FLD_REAL64PLUSOFFSET("ESP",256);
   X87_FADD_REAL64PLUSOFFSET("ESP",0);
  X87_FADD_REAL64PLUSOFFSET("ESP",8);
  X87_FADD_REAL64PLUSOFFSET("ESP",256);
    X86_ADD_REG32_VALUE32("ESP",0);   
  X86_ADD_REG32_VALUE32("ESP",8);   
  X86_ADD_REG32_VALUE32("ESP",256);   
  X87_FSTP_REAL64PLUSOFFSET("ESP",0);
  X87_FSTP_REAL64PLUSOFFSET("ESP",8);
  X87_FSTP_REAL64PLUSOFFSET("ESP",256);
  EmitPrologue(3);
  EmitEpilogue(3);

 

  #endif





  
  VirtualProtect(Code, 2048 , PAGE_EXECUTE, &oldprotect);
  rct  = (double (*)())Code;
  FlushInstructionCache(GetCurrentProcess(),0,0);
  double rt = (*rct)(); 
  VirtualProtect(Code, 2048, oldprotect, &oldprotect);

  fprintf(stdout,"The Result of the function is %g\n",rt);

  for( j=0; j< ip; ++j )
  {
     
   fprintf(stdout,"%x\t",Code[j]);

     if (j%3 == 0 )
          fprintf(stdout,"\n");
  }
  fprintf(stdout,"\n");

  free(Code);
  return 0;

}






