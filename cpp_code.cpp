#include <stdio.h>

#include <cstring>
#include <Windows.h>

using namespace std;

extern "C" __declspec(dllexport) int __stdcall Add(int a, int b) {
	

	return a + b;
}



////////////////////////////////////////
//
// ANSI version of Length function...
//
//
extern "C" __declspec(dllexport)  int __stdcall StrLenA(char *r)
{
	return strlen(r);
}

////////////////////////////////
//
// Unicode version of Strlen
//
//
extern "C" __declspec(dllexport)  int __stdcall StrLenW(wchar_t *r)
{
	return wcslen(r);
}
//////////////////////////////
//
// Computes the minimum value in an int array
//
//

extern "C" __declspec(dllexport)  int __stdcall
MinArray(int* pData, int length)
{

	int minData = pData[0];
	for (int pos = 1; pos < length; pos++)
	{

		if (pData[pos] < minData)
			minData = pData[pos];
	}

	return minData;
}
///////////////////////////////////////////
//
//
// Computes the minimum for a double array
//
//

extern "C" __declspec(dllexport)  double __stdcall
MinArrayD(double* pData, int length)
{

	double minData = pData[0];
	for (int pos = 1; pos < length; pos++)
	{

		if (pData[pos] < minData)
			minData = pData[pos];
	}

	return minData;
}
//////////////////////////////////////////
//
// Computes the arithematic mean of an array
//
//
//

extern "C" __declspec(dllexport)  double __stdcall
Average(double* pData, int length)
{

	double minData = pData[0];
	for (int pos = 1; pos < length; pos++)
	{

		minData += pData[pos];
	}
	return minData / length;
}


// 
// This structure will be passed between 
// C# and C++ 
//
//

struct EventData
{

	int I;

	char * Message;

};



extern "C"  __declspec(dllexport)  bool __stdcall PutEventData(
	EventData *ptr)
{



	printf("%s\n", ptr->Message);

	printf("%d\n", ptr->I);
	return false;




}

extern "C" __declspec(dllexport) void __stdcall ListOfSquares(
	long (__stdcall  *square_callback) (int rs)
	)

{


	for (int i = 0; i<10; ++i)
	{


		double ret = (*square_callback)(i);

		printf("Printing from C++ ... %g\n", ret);
	}


}

extern "C" __declspec(dllexport)  bool __stdcall  StringCopy(char *t,
	const char *src)
{


	if (t == 0 || src == 0)
		return false;



	if (*src == 0)

		return false;


	while (*t++ = *src++);
	//strcpy(t, src);



	return true;


}


