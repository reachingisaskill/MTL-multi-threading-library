
#ifndef __STDEXTS__MULTITHREADED_LINKED_LIST_H__
#define __STDEXTS__MULTITHREADED_LINKED_LIST_H__

#include "stdexts.h"

#include "MTL_defs.h"
#include "MTL_mutex.h"

//#define DISABLE_PTHREAD


namespace MTL
{


  template < class T, int N >
  class multiThreadedList : public stdexts::non_copyable
  {

    public:
      class listItem;
      enum statusT { status_error, status_continue, status_delete, status_stop };
      typedef statusT (*controlFunctionT)( T& );
      typedef void (*threadFunctionT)( T& );

      static void* slaveThread( void* );
      static void* controlThread( void* );


    private:
      struct threadInfoT;
      struct threadDataT;
//      friend void* slaveThread< T, N >( void* );
//      friend void* controlThread< T, N >( void* );

      struct threadInfoT
      {
        unsigned long int numberReps;
        int threadNumber;
        controlFunctionT controlFunc;
        threadFunctionT threadFunc;
        multiThreadedList< T, N >* list;
        threadDataT* threadData;
      };

      struct threadDataT
      {
        pthread_mutex_t mutex;
        pthread_cond_t condition;
        bool threadWaiting;
        bool controlWaiting;
      };



    public:
      class listItem : public mutex
      {
        friend class multiThreadedList;
        friend void* controlThread( void* );

        private:
          T _data;

          listItem* _prev;
          listItem* _next;

          statusT _status;

        protected:
          listItem( T, listItem* = 0, listItem* = 0 );

          listItem* _clear();

        public:
          virtual ~listItem();

          listItem* remove();

          listItem* next();
          const listItem* next() const;
          listItem* prev();
          const listItem* prev() const;

          T& get() { return _data; }
          const T& get() const { return _data; }
      };

    private:

      controlFunctionT _controlFunction;
      threadFunctionT _threadFunctions[N];
      pthread_t _threads[N];
      pthread_t _controlThread;
      threadDataT _threadData[N];
      pthread_mutex_t _mutex;

      listItem* _list;

      long unsigned int _lastCount;

    public:
      multiThreadedList();
      multiThreadedList( T );

      ~multiThreadedList() { if ( _list ) _list->_clear(); }

      multiThreadedList& push( T );

      listItem* start() { return _list; }
      const listItem* start() const { return _list; }

      bool isEmpty() const { return ( _list == 0 ); }
      operator bool () const { return ( _list != 0 ); }

      unsigned int count();
      const unsigned long int& lastCount() const { return _lastCount; }


      void setControlFunction( controlFunctionT f ) { _controlFunction = f; }
      void setThreadFunction( int i, threadFunctionT f ) { _threadFunctions[i] = f; }

      void go( unsigned long int );
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////


  template < class T, int N >
  void* multiThreadedList< T, N >::slaveThread( void* arg )
  {
    typename multiThreadedList< T, N >::threadInfoT* info = (typename multiThreadedList< T, N >::threadInfoT*) arg;
    unsigned long int numberReps = info->numberReps;
    const int NUM = info->threadNumber;

    pthread_mutex_lock( &info->threadData[ NUM ].mutex );

    for ( unsigned long int i = 0; i < numberReps; ++i )
    {
      typename multiThreadedList< T, N >::listItem* currentItem = info->list->start();
      typename multiThreadedList< T, N >::listItem* start = info->list->start();

      do
      {
        currentItem->lock();

        (info->threadFunc)( currentItem->get() );

        currentItem = currentItem->next();

        currentItem->prev()->unlock();

      } while ( currentItem != start );

      info->threadData[ NUM ].threadWaiting = true;
      bool ready = false;

      do
      {
        pthread_cond_wait( &info->threadData[ NUM ].condition, &info->threadData[ NUM ].mutex );

        if ( ( ! info->threadData[ NUM ].controlWaiting ) && ( ! info->threadData[ NUM ].threadWaiting ) )
          ready = true;

      } while ( ! ready );

    }
    pthread_mutex_unlock( &info->threadData[ NUM ].mutex );

    return 0;
  }

  template < class T, int N >
  void* multiThreadedList< T, N >::controlThread( void* arg )
  {
    typename multiThreadedList< T, N >::threadInfoT* info = ( typename multiThreadedList< T, N >::threadInfoT*) arg;

    bool closing = false;
    unsigned long int numberReps = info->numberReps;

    for ( unsigned int i = 0; (i < numberReps) && ( ! closing ); ++i )
    {
      for ( int n = 0; n < N; ++n )
      {
        pthread_mutex_lock( &info->threadData[n].mutex );
      }
      /*
      unsigned int numWaiting = 0;
      unsigned int n = 0;

      while ( numWaiting < N )
      {
        if ( ! pthread_mutex_trylock( &info->threadData[n].mutex ) )
        {
          if ( ! info->threadData[n].threadWaiting )
          {
            pthread_mutex_unlock( &info->threadData[n].mutex );
          }
          else if ( ! info->threadData[n].controlWaiting ) 
          {
            info->threadData[n].controlWaiting = true;
            ++numWaiting;
          }
        }

        if ( ++n == N ) n = 0;
      }
      */



      typename multiThreadedList< T, N >::listItem* currentItem = info->list->start();
      typename multiThreadedList< T, N >::listItem* start = info->list->start();

      unsigned long int counter = 0;

      do
      {
        // currentItem->waitLock();

        typename multiThreadedList< T, N >::statusT result = (info->controlFunc)( currentItem->get() );

        switch ( result )
        {
          case multiThreadedList< T, N >::status_delete :
            currentItem = currentItem->remove();
            break;

          case multiThreadedList< T, N >::status_stop :
            closing = true;
            break;

          case multiThreadedList< T, N >::status_continue :
          default :
            currentItem = currentItem->next();
            ++counter;
            break;
        }

        // currentItem->prev()->unlock();

      }
      while ( currentItem != start );

      info->list->_lastCount = counter;
      // info->list->count();



      for ( unsigned int n = 0; n < N; ++n )
      {
        info->threadData[n].threadWaiting = false;
        info->threadData[n].controlWaiting = false;
        pthread_cond_signal( &info->threadData[n].condition );
        pthread_mutex_unlock( &info->threadData[n].mutex );
      }
    }

    return 0;
  }

#ifndef DISABLE_PTHREAD

  template < class T, int N >
  void multiThreadedList< T, N >::go( unsigned long int num )
  {
    pthread_mutex_lock( &this->_mutex );
    multiThreadedList< T, N >::threadInfoT infos[N+1];

    std::cout << "Starting Slaves\n";
    for ( int n = 0; n < N; ++n )
    {
      if ( _threadFunctions[n] == 0 ) throw "YOU TWAT!";

      infos[n].numberReps = num;
      infos[n].threadNumber = n;
      infos[n].controlFunc = this->_controlFunction;
      infos[n].threadFunc = this->_threadFunctions[n];
      infos[n].list = this;
      infos[n].threadData = this->_threadData;

      pthread_create( &_threads[n], 0, multiThreadedList< T, N >::slaveThread, &infos[n] );
    }

    std::cout << "Starting Control\n";

    infos[N].numberReps = num;
    infos[N].threadNumber = -1;
    infos[N].controlFunc = this->_controlFunction;
    infos[N].threadFunc = 0;
    infos[N].list = this;
    infos[N].threadData = this->_threadData;

    pthread_create( &_controlThread, 0, multiThreadedList< T, N >::controlThread, &infos[N] );

    std::cout << "Running...\n\n";

    for ( int n = 0; n < N; ++n )
      pthread_join( _threads[n], 0 );
    pthread_join( _controlThread, 0 );

    std::cout << "Finished!\n" << std::endl;

    pthread_mutex_unlock( &this->_mutex );
  }

#else

  template < class T, int N >
  void multiThreadedList< T, N >::go( unsigned long int num )
  {
    std::cout << "No Multi Threading Being Used\n";

    bool closing = false;

    for ( unsigned long int i = 0; ( i < num ) && ( ! closing ); ++i )
    {
      typename multiThreadedList< T, N >::listItem* currentItem = start();
      const typename multiThreadedList< T, N >::listItem* const start = this->start();

      // -----   SlaveFunctions    -----

      for ( int i = 0; i < N; ++i )
      {
        do
        {
          (_threadFunctions[i])( currentItem->get() );

          currentItem = currentItem->next();
        }
        while ( currentItem != start );
      }

      // -----  Control Functions  -----
      currentItem = this->start();
      unsigned long int counter = 0;
      do
      {
        typename multiThreadedList< T, N >::statusT result = (_controlFunction)( currentItem->get() );
        unsigned long int counter = 0;

        switch ( result )
        {
          case multiThreadedList< T, N >::status_delete :
            currentItem = currentItem->remove();
            break;

          case multiThreadedList< T, N >::status_stop :
            closing = true;
            break;

          case multiThreadedList< T, N >::status_continue :
          default :
            currentItem = currentItem->next();
            ++counter;
            break;
        }
      }
      while ( currentItem != start );

      _lastCount = counter;
    }

    std::cout << "Finished!\n" << std::endl;
  }

#endif // DISABLE_PTHREAD

}

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MTL_list-cont.h"

#endif // __STDEXTS__MULTITHREADED_LINKED_LIST_H__

