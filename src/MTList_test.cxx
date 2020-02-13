
#include <iostream>
#include <vector>
#include <unistd.h>

#include "MTL.h"


typedef MTL::multiThreadedList< int, 2 > listTypeT;

listTypeT::statusT funcC( int& );
void func1( int& );
void func2( int& );

int main( int, char** )
{
  std::cout << "\nTesting the posix thread system.\n";
  std::cout << "Version 2. Multi Threaded List\n";

  const unsigned int NUM = 10;

  listTypeT list;

  for ( unsigned int i = 0; i < NUM; ++i )
    list.push( i );

  list.setControlFunction( funcC );
  list.setThreadFunction( 0, func1 );
  list.setThreadFunction( 1, func2 );
//  list.setThreadFunction( 2, func1 );
//  list.setThreadFunction( 3, func2 );
//  list.setThreadFunction( 4, func1 );

  list.go( 10 );

  std::cout << "RESULTS:" << std::endl;
  listTypeT::listItem* curr = list.start();

  for ( unsigned i = 0; i < NUM; ++i )
  {
    std::cout << curr->get() << ' ';
    curr = curr->next();
  }

  std::cout << std::endl;
  return 0;
}

listTypeT::statusT funcC( int& i )
{
  i = i - 1;
  //std::cout << "HERE Control\n";

  return listTypeT::status_continue;
}

void func1( int& i )
{
  //std::cout << "HERE 1\n";
  i = i + 1;
}

void func2( int& i )
{
  //std::cout << "HERE 2\n";
  i = i + 1;
}


/*

typedef particle testType;

void* thread1( void* );
void* thread2( void* );

  particle P1;
  particle P2;

  std::cout << "Created Test Particle\n";
  P1.print();
  P2.print();

  pthread_t tid1;
  pthread_t tid2;

  pthread_create( &tid1, NULL, thread1, (void*) &P1 );
  pthread_create( &tid2, NULL, thread2, (void*) &P2 );

  void* close1;
  void* close2;

  pthread_join( tid1, &close1 );
  pthread_join( tid2, &close2 );

  std::cout << "Test Particle At Finish\n";
  P1.print();
  P2.print();


void* thread1( void* arg )
{
  std::cout << "Thread 1 Start\n";

  testType* p = (testType*) arg;
  p->waitLock();
  p->value1 = 10;
  sleep( 2 );
  p->unlock();

  std::cout << "Thread 1 End\n";
  return 0;
}

void* thread2( void* arg )
{
  std::cout << "Thread 2 Start\n";

  testType* p = (testType*) arg;
  p->waitLock();
  p->value1 = 20;
  sleep( 2 );
  p->unlock();

  std::cout << "Thread 2 End\n";
  return 0;
}
*/


/*
#include "test_basics.h"

////////////////////////////////////////////////////////////////////////////////

// typedef std::vector< particle > ContainerT;
typedef std::vector< mutex< particle > > ContainerT;

pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;
// particle P;
ContainerT particles;

////////////////////////////////////////////////////////////////////////////////



void* thread1( void* );
void* thread2( void* );
void* thread3( void* );


int main( int, char** )
{
  std::cout << "\nTesting the posix thread system.\n";

  pthread_t tid1;
  pthread_t tid2;
  pthread_t tid3;


  std::cout << "Init Particles.\n";

  for ( unsigned int i = 0; i < 3; ++i )
    particles.push_back( particle() );


  for ( ContainerT::iterator it = particles.begin(); it != particles.end(); ++it )
    it->get().print();

  int init_ret1 = pthread_create( &tid1, NULL, thread1, NULL );
  int init_ret2 = pthread_create( &tid2, NULL, thread2, NULL );
  int init_ret3 = pthread_create( &tid3, NULL, thread3, NULL );

  void* close1;
  void* close2;
  void* close3;

  int end_ret1 = pthread_join( tid1, &close1 );
  int end_ret2 = pthread_join( tid2, &close2 );
  int end_ret3 = pthread_join( tid3, &close3 );

  std::cout << "Finished.\n";

  for ( ContainerT::iterator it = particles.begin(); it != particles.end(); ++it )
    it->get().print();

  std::cout << std::endl;
  return 0;
}

void* thread1( void* )
{
  std::cout << "Thread 1 Start\n";

  // while ( pthread_mutex_trylock( &globalMutex ) );
//  while ( pthread_mutex_trylock( &particles[0].mutex ) );
//  particles[0].value1 = 10;
//  sleep( 1 );
//  pthread_mutex_unlock( &globalMutex );
  
  particles[0].waitLock();
  particles[0].get().value1 = 10;
  sleep( 1 );
  particles[0].unlock();

  std::cout << "Thread 1 End\n" << std::flush;
  return (void*)(0);
}

void* thread2( void* )
{
  std::cout << "Thread 2 Start\n";

  // while ( pthread_mutex_trylock( &globalMutex ) );
  // while ( pthread_mutex_trylock( &particles[1].mutex ) );
  // particles[1].value2 = 20;
  // sleep( 2 );
  // pthread_mutex_unlock( &globalMutex );

  particles[0].waitLock();
  particles[0].get().value2 = 20;
  sleep( 2 );
  particles[0].unlock();

  std::cout << "Thread 2 End\n" << std::flush;
  return (void*)(0);
}

void* thread3( void* )
{
  std::cout << "Thread 3 Start\n";

  // while ( pthread_mutex_trylock( &globalMutex ) );
  // while ( pthread_mutex_trylock( &particles[2].mutex ) );
  // particles[2].value3 = 30;
  // sleep( 3 );
  // pthread_mutex_unlock( &globalMutex );

  particles[0].waitLock();
  particles[0].get().value3 = 30;
  sleep( 3 );
  particles[0].unlock();

  std::cout << "Thread 3 End\n" << std::flush;
  return (void*)(0);
}
*/


