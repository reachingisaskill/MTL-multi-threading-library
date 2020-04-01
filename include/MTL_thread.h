
#ifndef __MULTITHREADING_MUTEX_H__
#define __MULTITHREADING_MUTEX_H__

#include <pthread.h>

#include "stdexts.h"


namespace MTL
{

  typedef void* (*posixThreadT)( void* );

  template < class T >
  class thread
  {
    friend void* posixThreadFunction( void* );

    enum statusT { status_running, status_stopped };

    struct functionWrapper
    {
      void (*userFunction)( const T* );
      const T* data;
    };

    private:
      posixThreadT _theFunction;
      T* _theData;
      pthread_t _thread;
      pthread_attr_t _attribute;
      statusT _status;

    protected:
    public:
      thread( posixThreadT );

      void go( T* );

      void waitOn();

  };

  template < class T >
  void* posixThreadFunction( void* data )
  {
    try
    {
      thread::functionWrapper userData = reinterpret_cast< thread::functionWrapper* >( data );
      userData.userFunction( userData.data );
    }
    catch( std::exception e )
    {
    }
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Member Functions

  template < class T >
  thread< T >::thread( posixThreadT func ) :
    _theFunction( func ),
    _theData( nullptr ),
    _thread(),
    _attribute(),
    _status( status_stopped )
  {
  }


  template < class T >
  void thread< T >::go( T* data )
  {
    _theData = data;

    // Set thread attributes
    pthread_attr_init( &_attribute );
    pthread_attr_setdetatchstate( &_attribute, PTHREAD_CREATE_JOINABLE );

    // Build data object
    functionWrapper wrapper;
    wrapper.userFunction = _theFunction;
    wrapper.data = _theData;

    int create_status = pthread_create( &_thread, &_attribute, &posixThreadFunction< T >, reinterpret_cast< void* > &wrapper );

    if ( create_status )
    {
      std::cerr << "Create error : " << create_status << " in thread: " << _thread << std::endl;
    }
    else
    {
      _status = status_running;
    }
  }


  template < class T >
  void thread< T >::waitOn()
  {
    void* status = nullptr;

    int join_status = pthread_join( _thread, status );
    if ( join_status )
    {
      std::cerr << "Join Error: " << join_status << " in thread: " << _thread << std::endl;
    }
    else
    {
    }

    _status = status_stopped;
  }

}

#endif // __MULTITHREADING_MUTEX_H__

