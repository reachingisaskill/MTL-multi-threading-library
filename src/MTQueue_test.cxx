
#include <iostream>

#include "MTL.h"

#include <cstdio>
#include <unistd.h>

#define MAX_NUM 1000


typedef MTL::multiThreadedQueue< int > queueT;

static queueT queue;

static timespec sleepTime = { 0, 100 };


void* popper( void* );
void* pusher( void* );



#ifndef DISABLE_PTHREAD

int main( int, char** )
{

  std::cout << "Testing the Multi Threaded Queue Class\n";

  pthread_t tid1;
  pthread_t tid2;
  pthread_t tid3;
  pthread_t tid4;

  printf( "CREATION\n\n" );

  pthread_create( &tid2, 0, pusher, 0 );
  pthread_create( &tid1, 0, popper, 0 );
  pthread_create( &tid4, 0, pusher, 0 );
  pthread_create( &tid3, 0, popper, 0 );

  void* close1;
  void* close2;
  void* close3;
  void* close4;

  printf( "JOINING\n\n" );

  pthread_join( tid2, &close2 );
  pthread_join( tid4, &close4 );

  printf( "FLUSHING\n\n" );
  queue.flush();

  pthread_join( tid1, &close1 );
  pthread_join( tid3, &close3 );

  printf( "FINSIHED\n" );

  std::cout << std::endl;
  return 0;
}


void* pusher( void* )
{

  for ( unsigned int i = 0; i < MAX_NUM; ++i )
  {
    queue.push( i );
    // sleep( 1 );
    nanosleep( &sleepTime, 0 );
  }

  return 0;
}

void* popper( void* )
{

  int value;
  while ( queue.waitPop( value ) )
  {
    // printf( "POP  %i\n", value );
    nanosleep( &sleepTime, 0 );
  }

  /*
  while ( queue.wait() )
  {
    queue.pop();
    // printf( "POP  %i\n", value );
  }
  */

  return 0;
}

#else

int main( int, char** )
{

  std::cout << "Testing the Multi Threaded Queue Class\nNo Multithreading!\n";

  printf( "RUNNING\n\n" );

  pusher( 0 );
  pusher( 0 );
  pusher( 0 );
  pusher( 0 );
  pusher( 0 );
  popper( 0 );
  popper( 0 );
  popper( 0 );
  pusher( 0 );
  pusher( 0 );
  popper( 0 );
  popper( 0 );
  popper( 0 );
  pusher( 0 );
  popper( 0 );
  pusher( 0 );
  popper( 0 );
  pusher( 0 );
  pusher( 0 );
  popper( 0 );
  pusher( 0 );

  printf( "FLUSHING\n\n" );
  queue.flush();

  printf( "FINSIHED\n" );

  std::cout << std::endl;
  return 0;
}


void* pusher( void* )
{

  for ( unsigned int i = 0; i < MAX_NUM; ++i )
  {
    queue.push( i );
    // sleep( 1 );
    nanosleep( &sleepTime, 0 );
  }

  return 0;
}

void* popper( void* )
{

  int value;
  while ( queue.waitPop( value ) )
  {
    // printf( "POP  %i\n", value );
    nanosleep( &sleepTime, 0 );
  }

  /*
  while ( queue.wait() )
  {
    queue.pop();
    // printf( "POP  %i\n", value );
  }
  */

  return 0;
}

#endif

