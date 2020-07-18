
#include "MTL_CircularLatch.h"

#include <iostream>
#include <thread>
#include <list>
#include <vector>


using namespace MTL;

const int NUMBER_ITERATIONS = 10;
const int NUMBER_THREADS = 4;

std::vector< int > theVector;

CircularLatch& getLatch()
{
  static CircularLatch theLatch( NUMBER_THREADS );
  return theLatch;
}


void thread( int );


int main( int, char** )
{
  std::list<std::thread> threads;
  for ( int i = 0; i < NUMBER_THREADS; ++i )
  {
    threads.push_back( std::thread( thread, i ) );
  }


  for ( std::list<std::thread>::iterator it = threads.begin(); it != threads.end(); ++it )
  {
    it->join();
  }


  for ( int i = 0; i < NUMBER_ITERATIONS*NUMBER_THREADS; i += NUMBER_THREADS )
  {
    for ( int j = 0; j < NUMBER_THREADS; ++j )
    {
      std::cout << theVector[ i + j ] << '\t';
    }
    std::cout << std::endl;
  }

  return 0;
}

void thread( int N )
{
  CircularLatch& latch = getLatch();

  CircularLatch::Handle handle = latch.requestHandle( N );

  for ( int i = 0; i < NUMBER_ITERATIONS; ++i )
  {
    handle.lock();
    theVector.push_back( N );
//    std::cout << N << std::endl;
    handle.unlock();
  }

  std::cout << "Thread " << N << " Finished" << std::endl;

}

