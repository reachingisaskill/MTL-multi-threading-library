
#include "MTL_mutex.h"


namespace MTL
{

  mutex_base::mutex_base()
  {
  }

  mutex_base::~mutex_base()
  {
  }

  int mutex_base::tryLock()
  {
    return pthread_mutex_trylock( &this->getMutex() );
  }

  int mutex_base::lock()
  {
    return pthread_mutex_lock( &this->getMutex() );
  }

  int mutex_base::unlock()
  {
    return pthread_mutex_unlock( &this->getMutex() );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////


  mutex::mutex()
  {
  }

  mutex::~mutex()
  {
    pthread_mutex_destroy( &_mutex );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////


  condition::condition() 
  {
    pthread_cond_init( &_condition, 0 );
  }

  condition::~condition() 
  {
    pthread_cond_destroy( &_condition );
  }

  int condition::waitOn( mutex_base& mut )
  {
    return pthread_cond_wait( &_condition, &mut.getMutex() );
  }

  int condition::waitFor( mutex_base& mut, timespec time )
  {
    return pthread_cond_timedwait( &_condition, &mut.getMutex(), &time );
  }

  int condition::signal()
  {
    return pthread_cond_signal( &_condition );
  }

  int condition::broadcast()
  {
    return pthread_cond_broadcast( &_condition );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////


  mutex_condition::mutex_condition() :
    mutex(),
    condition()
  {
  }

  mutex_condition::~mutex_condition() 
  {
  }

  int mutex_condition::waitOn()
  {
    return condition::waitOn( *this );
  }

  int mutex_condition::lockWaitOn()
  {
    this->lock();
    int res = this->waitOn();
    this->unlock();
    return res;
  }

  int mutex_condition::waitFor( timespec time )
  {
    return condition::waitFor( *this, time );
  }

  int mutex_condition::lockWaitFor( timespec time )
  {
    this->lock();
    int res = this->waitFor( time );
    this->unlock();
    return res;
  }

  int mutex_condition::lockSignal()
  {
    this->lock();
    int res = this->signal();
    this->unlock();
    return res;
  }

  int mutex_condition::trySignal()
  {
    int res = this->tryLock();
    if ( res ) 
    {
      return res;
    }
    else
    {
      res = this->signal();
      this->unlock();
    }
    return res;
  }

  int mutex_condition::lockBroadcast()
  {
    this->lock();
    int res = this->broadcast();
    this->unlock();
    return res;
  }

  int mutex_condition::tryBroadcast()
  {
    int res = this->tryLock();
    if ( res ) return res;
    else
    {
      res = this->broadcast();
      this->unlock();
    }
    return res;
  }

}

