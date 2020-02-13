
#ifndef __MTL__SYNC_CONT_H__
#define __MTL__SYNC_CONT_H__


namespace MTL
{

#ifndef DISABLE_PTHREAD

  template < class T, int N >
  multiThreadedSync< T, N >::multiThreadedSync() :
    _data()
  {
    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadFunctions[n] = 0;
      _threads[n] = 0;
      _threadData[n].parent = this;

      _threadData[n].status = status_invalid;
      _threadData[n].waiting = false;
    }
  }

  template < class T, int N >
  multiThreadedSync< T, N >::multiThreadedSync( T obj ) :
    _data( obj )
  {
    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadFunctions[n] = 0;
      _threads[n] = 0;
      _threadData[n].parent = this;

      _threadData[n].status = status_invalid;
      _threadData[n].waiting = false;
    }
  }


  template < class T, int N >
  multiThreadedSync< T, N >::~multiThreadedSync()
  {
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::setFunction( int i, threadFunctionT f )
  {
    if ( f == 0 ) return 61; // ENODATA
    if ( i >= N || i < 0) return 22; // EINVAL

    int res;
    if ( res = _threadData[i].lock(), res ) return res;

    _threadFunctions[i] = f;
    _threadData[i].status = status_pending;
    _threadData[i].unlock();

    return 0;
  }


  template < class T, int N >
  typename multiThreadedSync< T, N >::statusT multiThreadedSync< T, N >::getStatus( int i )
  {
    if ( i >= N || i < 0 ) return status_invalid;

    if ( _threadData[i].lock() ) return status_error;

    typename multiThreadedSync< T, N >::statusT stat = _threadData[i].status;
    _threadData[i].unlock();

    return stat;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::init()
  {
    for ( unsigned int n = 0; n < N; ++n )
    {
      if ( _threadFunctions[n] == 0 ) return 61; // ENODATA
    }

    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadData[n].lock();

      _threadData[n].waiting = true; // Don't jump the gun!


      int result = pthread_create( &_threads[n], 0, multiThreadedSync< T, N >::slaveThread, (void*)&_threadData[n] );
      if ( result )
      {
        _threadData[n].status = status_error;
        _threadData[n].unlock();
        return result;
      }


      _threadData[n].status = status_stopped;
      _threadData[n].stop = false;
      _threadData[n].function = &_threadFunctions[n];

      _threadData[n].unlock();
    }

    return 0;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::go() // TODO Check Error Status
  {

    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadData[n].lock();

      if ( _threadData[n].status == status_running ) 
      {
        _threadData[n].unlock();
        return 114; // EALREADY
      }
      if ( _threadData[n].status != status_stopped )
      {
        _threadData[n].unlock();
        return 22; // EINVAL
      }

      _threadData[n].unlock();
    }

    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadData[n].lock();

      _threadData[n].stop = false;
      _threadData[n].waiting = false;
      _threadData[n].status = status_running;

      _threadData[n].cond.signal();

      _threadData[n].unlock();
    }

    return 0;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::wait()
  {
    _threadFinished.lock();

    bool stillRunning;

    while ( true )
    {
      stillRunning = false;

      for ( unsigned int n = 0 ; n < N; ++n )
      {
        _threadData[n].lock();

        if ( _threadData[n].status == status_running ) stillRunning = true;
        else if ( _threadData[n].status != status_stopped ) 
        {
          _threadData[n].unlock();
          _threadFinished.unlock();
          return 22; // EINVAL
        }

        _threadData[n].unlock();
      }

      if ( ! stillRunning ) break;

      _threadFinished.waitOn();
    }

    _threadFinished.unlock();

    return 0;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::close() // TODO Check Error Status
  {
    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadData[n].lock();
      if ( _threadData[n].status != status_stopped )
      {
        _threadData[n].unlock();
        return 22; // EINVAL
      }
      _threadData[n].unlock();
    }

    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadData[n].lock();
      _threadData[n].stop = true;
      _threadData[n].waiting = false;
      _threadData[n].cond.signal();
      _threadData[n].unlock();
    }

    bool closeError = false;
    void* result = 0;

    for ( unsigned int n = 0; n < N; ++n )
    {
      void* result = 0;

      pthread_join( _threads[n], &result );
      if ( result ) closeError = true;
    }

    if ( closeError ) return (long int)( result );
    return 0;
  }


#else


  template < class T, int N >
  multiThreadedSync< T, N >::multiThreadedSync() :
    _data()
  {
  }

  template < class T, int N >
  multiThreadedSync< T, N >::multiThreadedSync( T obj ) :
    _data( obj )
  {
  }


  template < class T, int N >
  multiThreadedSync< T, N >::~multiThreadedSync()
  {
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::setFunction( int i, threadFunctionT f )
  {
    if ( f == 0 ) return 61; // ENODATA
    if ( i >= N || i < 0 ) return 22; // EINVAL

    _threadFunctions[i] = f;

    return 0;
  }


  template < class T, int N >
  typename multiThreadedSync< T, N >::statusT multiThreadedSync< T, N >::getStatus( int i )
  {
    if ( i >= N || i < 0 ) return status_invalid;
    return status_stopped;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::init()
  {
    for ( unsigned int n = 0; n < N; ++n )
    {
      if ( _threadFunctions[n] == 0 ) return 61; // ENODATA
    }

    return 0;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::go()
  {
    for ( unsigned int n = 0; n < N; ++n )
    {
      (*_threadFunctions[n])( _data );
    }

    return 0;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::wait()
  {
    return 0;
  }


  template < class T, int N >
  int multiThreadedSync< T, N >::close()
  {
    return 0;
  }



#endif

}

#endif // __MTL__SYNC_CONT_H__

