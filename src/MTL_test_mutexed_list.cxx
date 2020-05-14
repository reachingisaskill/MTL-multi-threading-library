
#include "MTL_MutexedStack.h"

#include <random>
#include <thread>
#include <vector>

using namespace MTL;


const unsigned int MAX_ITERATIONS = 1000000;
const unsigned int NUM_THREADS = 2;

void thread_func();

void iterator_func();

MutexedList< int > the_list;

int main( int, char** )
{
  int test_value;

  {
    MutexedList< int >::Iterator it = the_list.begin();
    it.erase();
  }

  the_list.push( 1 );
  the_list.push( 1 );
  the_list.push( 1 );
  the_list.push( 1 );
  the_list.front( test_value );

  MutexedList< int >::Iterator it = the_list.begin();

  if ( it.isGood() )
  {
    it.next();
    it.next();
    it.erase();
  }

  it.destroy();

  the_list.clear();


  std::vector< std::thread > threads;
  threads.reserve( NUM_THREADS+2 );

  threads.push_back( std::thread( iterator_func ) );
  for ( unsigned int i = 0; i < NUM_THREADS; ++i )
  {
    threads.push_back( std::thread( thread_func ) );
  }
  threads.push_back( std::thread( iterator_func ) );


  for ( unsigned int i = 0; i < NUM_THREADS+2; ++i )
  {
    threads[i].join();
  }

  return 0;
}


void iterator_func()
{
  std::random_device rd;
  std::mt19937 gen( rd() );
  std::uniform_int_distribution<> dis( 1, 10 );
  unsigned int num = MAX_ITERATIONS;


  for ( unsigned int i = 0; i < num; ++i )
  {
    MutexedList< int >::Iterator it = the_list.begin();

    if ( it.isGood() )
    {
      do
      {
        if ( dis( gen ) > 5 )
        {
          it.erase();
        }
      }
      while ( it.next() );
    }
  }

}


void thread_func()
{
  std::random_device rd;
  std::mt19937 gen( rd() );
  std::uniform_int_distribution<> dis( 1, 10 );

  int test;

  for ( unsigned int i = 0; i < MAX_ITERATIONS; ++i )
  {
    int value = dis( gen );

    switch ( value )
    {
      case 1 :
      case 2 :
      case 3 :
      case 4 :
        the_list.push( 1 );
        break;

      case 5 :
      case 6 :
        the_list.pop();
        break;

      case 7 :
      case 8 :
      case 9 :
        the_list.front( test );
        break;

      case 10 :
        the_list.clear();
        break;

      default:
        continue;
        break;
    }
  }
}

