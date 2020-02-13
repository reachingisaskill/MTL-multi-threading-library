
#include <iostream>

#include <unistd.h>
#include <cstdio>
#include <cstring>

#include "MTL.h"


#define MAX_NUM 5

static const int THE_INT = 101;

static MTL::multiThreadedSync< const int&, 3 > syncher( THE_INT );



void func1( const int& val );
void func2( const int& val );
void func3( const int& val );


int main( int, char** )
{
  std::cout << "Multi Threaded Synchroniser Testing\n" << std::endl;


  syncher.setFunction( 0, func1 );
  syncher.setFunction( 1, func2 );
  syncher.setFunction( 2, func3 );

  int result;

  if ( result = syncher.init(), result )  printf( "FAILED INIT  : %i - %s\n", result, strerror( result ) );

  for ( unsigned int i = 0; i < MAX_NUM; ++i )
  {
    printf( "RUN - %i\n", i );

    if ( result = syncher.go(), result )    printf( "FAILED GO    : %i - %s\n", result, strerror( result ) );

    // sleep( 1 );

    if ( result = syncher.wait(), result )  printf( "FAILED WAIT  : %i - %s\n", result, strerror( result ) );
  }

  if ( result = syncher.close(), result ) printf( "FAILED CLOSE : %i - %s\n", result, strerror( result ) );


  std::cout << std::endl;
  return 0;
}


void func1( const int& val )
{
  sleep( 1 );
  printf( "THREAD : %lu : VAL = %i\n", pthread_self(), val );
}

void func2( const int& val )
{
  sleep( 2 );
  printf( "THREAD : %lu : VAL = %i\n", pthread_self(), val );
}

void func3( const int& val )
{
  sleep( 3 );
  printf( "THREAD : %lu : VAL = %i\n", pthread_self(), val );
}

