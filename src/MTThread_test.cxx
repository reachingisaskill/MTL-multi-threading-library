
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "MTL.h"

void function( const std::string );


int main( int, char** )
{
  std::string the_string( "This is a test string" );
  std::thread t( &function, the_string );

  std::cout << "Thread running, please wait...\n" << std::endl;

  t.join();
  
  return 0;
}



void function( const std::string data )
{
  std::this_thread::sleep_for( std::chrono::milliseconds( 5000 ) );
  std::cout << data;
}

