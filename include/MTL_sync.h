
#ifndef __MTL__SYNC_H__
#define __MTL__SYNC_H__

#include "MTL_mutex.h"


namespace MTL
{

  template < class T, int N >
  class multiThreadedSync : public stdexts::non_copyable
  {

    public:
      enum statusT { status_invalid, status_error, status_running, status_pending, status_stopped };
      typedef void (*threadFunctionT) ( T& );

    private:
#ifndef DISABLE_PTHREAD
      static void* slaveThread( void* );

      struct threadDataT : public mutex
      {
        multiThreadedSync* parent;
        threadFunctionT* function;
        condition cond; // Thread start next run signal
        statusT status; // Thread Status from thread
        bool waiting; // Waiting for next run signal from control
        bool stop; // Halt signal from control
      };
#endif



    private:

      T _data;
      threadFunctionT _threadFunctions[N];
#ifndef DISABLE_PTHREAD
      pthread_t _threads[N];
      threadDataT _threadData[N];
      mutex_condition _threadFinished;
#endif

    public:
      multiThreadedSync();
      multiThreadedSync( T );

      ~multiThreadedSync();

      void setData( T d );
      void getData( T d );

      int setFunction( int, threadFunctionT );
      statusT getStatus( int );

      int init();
      int go();
      int wait();
      int close();
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef DISABLE_PTHREAD

  template < class T, int N >
  void* multiThreadedSync< T, N >::slaveThread( void* arg )
  {
    threadDataT* data = (threadDataT*)arg;

    data->lock();

    multiThreadedSync< T, N >* parent = data->parent;
    threadFunctionT* function = data->function;

    data->unlock();


    while ( true )
    {

      data->lock();
      while ( data->waiting == true )
      {
        data->cond.waitOn( *data );
      }

      if ( data->stop == true )
      {
        data->status = status_pending;
        data->unlock();
        break;
      }

      data->waiting = true;
      // data->status = status_running;
      data->unlock();

      // Call Functions here.

      (*function)( parent->_data );

      data->lock();
      data->status = status_stopped;
      data->unlock();

      parent->_threadFinished.lockSignal();

    }

    return 0;
  }

#endif // NDEF DISABLE_PTHREAD

}

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MTL_sync-cont.h"

#endif // __MTL__SYNC_H__

