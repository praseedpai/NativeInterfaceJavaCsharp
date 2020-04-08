#include <stdio.h>


int main( int argc , char **argv )
{
   FILE *fp = fopen("test.bin", "wb" );

   if (fp == 0 ) { return -1; }

   int i=0;
   while (i < 10) {

       fwrite( &i,1,4,fp);
       i++;
   }

   fclose(fp);

}