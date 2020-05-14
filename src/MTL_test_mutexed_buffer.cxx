
#include "MTL_MutexedBuffer.h"

#include <random>
#include <thread>
#include <vector>

using namespace MTL;


const unsigned int MAX_ITERATIONS = 10000;
const unsigned int NUM_THREADS = 4;

void pusher();

void popper();


MutexedBuffer< int > the_buffer;


int main( int, char** )
{
  int test_value;


  the_buffer.push( 1 );
  std::cout << the_buffer.size() << std::endl;
  the_buffer.push( 2 );
  std::cout << the_buffer.size() << std::endl;
  the_buffer.push( 3 );
  std::cout << the_buffer.size() << std::endl;
  the_buffer.push( 4 );

  std::cout << the_buffer.size() << std::endl;

  the_buffer.pop( test_value );
  std::cout << test_value << std::endl;
  the_buffer.pop( test_value );
  std::cout << test_value << std::endl;
  the_buffer.pop( test_value );
  std::cout << test_value << std::endl;
  the_buffer.pop( test_value );
  std::cout << test_value << std::endl;


  std::cout << the_buffer.size() << std::endl;


  std::vector< std::thread > threads;
  threads.reserve( NUM_THREADS*2 );


  for ( unsigned int i = 0; i < NUM_THREADS; ++i )
  {
    threads.push_back( std::thread( pusher ) );
    threads.push_back( std::thread( popper ) );
  }


  for ( unsigned int i = 0; i < NUM_THREADS*2; ++i )
  {
    threads[i].join();
  }

  return 0;
}


void pusher()
{
  std::random_device rd;
  std::mt19937 gen( rd() );
  std::uniform_int_distribution<> dis( 1, 10 );
  unsigned int num = MAX_ITERATIONS;


  for ( unsigned int i = 0; i < num; ++i )
  {
    the_buffer.push( dis( gen ) );
  }

}

void popper()
{
  std::random_device rd;
  std::mt19937 gen( rd() );
  std::uniform_int_distribution<> dis( 0, 20 );

  unsigned int num = MAX_ITERATIONS;
  int test;

  for ( unsigned int i = 0; i < num; ++i )
  {
    the_buffer.pop( test );

    if ( dis( gen ) == 0 )
    {
      the_buffer.clear();
    }
  }
}


