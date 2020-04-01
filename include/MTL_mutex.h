
#ifndef __MULTITHREADING_MUTEX_H__
#define __MULTITHREADING_MUTEX_H__

#include "pthread.h"
#include "time.h"
#include "stdexts.h"

//#include <cstdio>
//#include <cstring>



namespace MTL
{

  typedef void* (*posixThreadT)( void* );


  class condition;
  class mutex_base;


  ////////////////////////////////////////////////////////////////////////////////////////////////////


  class condition
  {
    private:
      pthread_cond_t _condition;

    protected:
      pthread_cond_t& getCondition() { return _condition; }

    public:
      condition();

      virtual ~condition();

      virtual int waitOn( mutex_base& );
      virtual int waitFor( mutex_base&, timespec );

      virtual int signal();

      virtual int broadcast();
  };


  //////////////////////////////////////////////////


  class mutex_base
  {
    friend class condition;
    private:

    protected:
      mutex_base(); // TODO Init Attributes somehow

      virtual pthread_mutex_t& getMutex() = 0;

    public:

      virtual ~mutex_base();

      virtual int lock();
      virtual int tryLock();
      virtual int unlock();

  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////


  class mutex : public mutex_base
  {
    private:
      pthread_mutex_t _mutex;

    protected:
      pthread_mutex_t& getMutex() { return _mutex; }

    public:
      mutex();
      virtual ~mutex();
  };


  //////////////////////////////////////////////////


  template < class T >
  class mutex_static : public mutex_base
  {
    private:
      static pthread_mutex_t _mutex;
      static bool set;

    protected:
      pthread_mutex_t& getMutex() { return _mutex; }

    public:
      mutex_static();

      virtual ~mutex_static();

      bool isInit() const { return mutex_static< T >::set; }
  };

  template < class T > pthread_mutex_t mutex_static< T >::_mutex;
  template < class T > bool mutex_static< T >::set = false;


  //////////////////////////////////////////////////


  template < class T >
  class mutex_variable : public mutex
  {
    private :
      T _data;

    public :
      mutex_variable();
      mutex_variable( T );

      virtual ~mutex_variable();

      virtual T& get() { return _data; }
      virtual const T& get() const { return _data; }
  };


  //////////////////////////////////////////////////


  template < class T >
  class mutex_static_variable : public mutex_static< T >
  {
    private :
      T _data;

    public :
      mutex_static_variable();
      mutex_static_variable( T );

      virtual ~mutex_static_variable();

      virtual T& get() { return _data; }
      virtual const T& get() const { return _data; }
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////


  class mutex_condition : public mutex, public condition
  {
    private:
      pthread_cond_t _condition;

    public:
      mutex_condition();

      virtual ~mutex_condition();

      virtual int waitOn();
      virtual int lockWaitOn();
      virtual int waitFor( timespec );
      virtual int lockWaitFor( timespec );

      virtual int lockSignal();
      virtual int trySignal();

      virtual int lockBroadcast();
      virtual int tryBroadcast();
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////


  template < class T >
  mutex_static< T >::mutex_static()
  {
    if ( ! mutex_static< T >::set )
    {
      pthread_mutexattr_t attr;
      pthread_mutexattr_init( &attr );
      pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_NORMAL );
      pthread_mutexattr_setrobust( &attr, PTHREAD_MUTEX_ROBUST );
      pthread_mutex_init( &mutex_static< T >::_mutex, &attr );
      mutex_static< T >::set = true;
    }
  }

  template < class T >
  mutex_static< T >::~mutex_static()
  {
    pthread_mutex_destroy( &_mutex );
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////


  template < class T >
  mutex_variable< T >::mutex_variable() :
    _data()
  {
  }

  template < class T >
  mutex_variable< T >::mutex_variable( T o ) :
    _data( o )
  {
  }

  template < class T >
  mutex_variable< T >::~mutex_variable()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////


  template < class T >
  mutex_static_variable< T >::mutex_static_variable() :
    _data()
  {
  }

  template < class T >
  mutex_static_variable< T >::mutex_static_variable( T obj ) :
    _data( obj )
  {
  }

  template < class T >
  mutex_static_variable< T >::~mutex_static_variable()
  {
  }

}

#endif // __MULTITHREADING_MUTEX_H__

