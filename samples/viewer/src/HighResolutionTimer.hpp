/**************************************************
** HighResolutionTimer.hpp                       **
** ---------------------                         **
**                                               **
** Defines a high resolution timer you can use   **
**    to compile under either Windows or Linux.  **
**    I've _tried_ to implement something that   **
**    works under MacOS X, but I cannot test it, **
**    so you'll need to tell me if it works or   **
**    how to fix it!                             **
** Note: This is almost certainly broken under   **
**    Cygwin...                                  **
**                                               **
** Chris Wyman (2/22/2007)                       **
**************************************************/


/* Defined data type:                                                    */
/* ------------------                                                    */
/*                                                                       */
/* TimerStruct                                                           */
/*        (Under windows this is a nasty, nasty union, under Linux it's  */
/*         a struct timespec, under MacOS, it's currently a struct       */
/*         timespec, but I'm not sure if that'll work).                  */

/* Defined functions:                                                    */
/* ------------------                                                    */
/*                                                                       */
/* inline void GetHighResolutionTime( TimerStruct *t );                  */
/*        Gets the current time and store it in the TimerStruct.  This   */
/*        should have nanosecond resolution on all systems (except maybe */
/*        Cygwin), though the OS may not update the timer ever ns.       */
/*                                                                       */
/* inline float ConvertTimeDifferenceToSec( TimerStruct *end,            */
/*                                          TimerStruct *begin );        */
/*        Takes two TimerStructs and returns a floating point value      */
/*        representing the seconds between the begin and end point.      */
/*        Beware using this over long periods of time (> 0.1 sec) if you */
/*        *really* want nanosecond precision, as your float will lose    */
/*        precision in those bits -- reimplement with a double or an int */



#ifndef HIGHRES_TIMER_HPP
#define HIGHRES_TIMER_HPP


extern "C" {

/* This header defines two functions and one data type used by them,     */
/*    but the implmentation changes, depending on which system is being  */
/*    used.  You may need to modify the #ifdef's to work correctly  on   */
/*    your computer...  I defined them rather arbitrarily, based on what */
/*    has worked for me (or in the case of the MacOS, a guess).          */

// Are we using MS Visual Studio?  
#if defined(WIN32) && defined(_MSC_VER)
	#define USING_MSVC 
#endif

// Are we using MacOS?
#if defined(__APPLE__)
	#define USING_MACOSX
#endif

// Are we using GCC under Linux or other Unixes (including Cygwin)?
#if defined(__GNUC__) && !defined(USING_MACOSX) 
	#define USING_LINUX
#endif



/* the following mess implements these timing mechanisms  */

#if defined(USING_MSVC)  // Use code that works on MS Visual Studio.

        /* this code should link without any work on your part */

	#include <windows.h>
	#pragma comment(lib, "kernel32.lib")
	typedef LARGE_INTEGER TimerStruct;
	inline void GetHighResolutionTime( TimerStruct *t ) 
		{ QueryPerformanceCounter( t ); }
	inline float ConvertTimeDifferenceToSec( TimerStruct *end, TimerStruct *begin ) 
		{ TimerStruct freq; QueryPerformanceFrequency( &freq );  return (end->QuadPart - begin->QuadPart)/(float)freq.QuadPart; }

#elif defined(USING_LINUX)  // Assume we have POSIX calls clock_gettime() 

        /* on some Linux systems, you may need to link in the realtime library (librt.a or librt.so) in 
	   order to use this code.  You can do this by including -lrt on the gcc/g++ command line.
	*/

	#include <time.h>
	typedef struct timespec TimerStruct;
	inline void GetHighResolutionTime( TimerStruct *t ) 
		{ clock_gettime( CLOCK_REALTIME, t ); }
	inline float ConvertTimeDifferenceToSec( TimerStruct *end, TimerStruct *begin ) 
		{ return (end->tv_sec - begin->tv_sec) + (1e-9)*(end->tv_nsec - begin->tv_nsec); }

#elif defined(USING_MACOSX)  // Assume we're running on MacOS X

	/* this code uses calls from the CoreServices framework, so to get this to work you need to
	   add the "-framework CoreServices" parameter g++ in the linking stage. This code was adapted from:
	   http://developer.apple.com/qa/qa2004/qa1398.html
	*/

	#include <CoreServices/CoreServices.h>
	#include <mach/mach.h>
	#include <mach/mach_time.h>
	typedef uint64_t TimerStruct;
	inline void GetHighResolutionTime( TimerStruct *t ) 
		{ *t = mach_absolute_time(); }
	inline float ConvertTimeDifferenceToSec( TimerStruct *end, TimerStruct *begin ) 
		{ uint64_t elapsed = *end - *begin; Nanoseconds elapsedNano = AbsoluteToNanoseconds( *(AbsoluteTime*)&elapsed );
			return float(*(uint64_t*)&elapsedNano) * (1e-9); }
#endif

} //end extern "C"

#endif // end #ifndef HIGHRES_TIMER_HPP
