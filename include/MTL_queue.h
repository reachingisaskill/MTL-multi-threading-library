
#ifndef __STDEXTS__MULTI_THREADED_QUEUE_H__
#define __STDEXTS__MULTI_THREADED_QUEUE_H__

#include "MTL_mutex.h"


#ifndef THREADING_SLEEP_TIME
#define THREADING_SLEEP_TIME 10000000
#endif


namespace MTL
{

  template < class T >
  class multiThreadedQueue : public stdexts::non_copyable
  {
    class listItem
    {
      friend class multiThreadedQueue;

      private:
        T _data;

        listItem* _next;

      protected:
        listItem( T );

      public:
        virtual ~listItem();

        listItem* insert( listItem* );
        listItem* remove();

        listItem* next() { return _next; }
        const listItem* next() const { return _next; }

        const T& get() const { return _data; }
        T& get() { return _data; }
    };

    private:

#ifndef DISABLE_PTHREAD
      static const struct timespec _sleepTime;

      mutable mutex_variable< listItem* > _head;
      mutable mutex_variable< listItem* > _tail;
      mutable mutex_variable< bool > _finished;
      mutable mutex_condition _cond;
#else
      listItem* _head;
      listItem* _tail;
#endif

    public:
      multiThreadedQueue();
      multiThreadedQueue( T );

      ~multiThreadedQueue();

      void push( T );
      T pop();
      bool waitPop( T& );
#ifndef DISABLE_PTHREAD
      void flush() const;
#else
      void flush();
#endif
      bool finished() const;

      bool wait() const;
      void waitTimed() const;

      bool isEmpty() const;
      bool notEmpty() const;
      operator bool () const;

      unsigned int count();
  };


#ifndef DISABLE_PTHREAD
  template < class T >
  const struct timespec multiThreadedQueue< T >::_sleepTime = { 0, THREADING_SLEEP_TIME };
#endif

}

#include "MTL_queue-cont.h"

#endif // __STDEXTS__MULTITHREADED_QUEUE_H__

