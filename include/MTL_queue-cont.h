
#ifndef __MTL__QUEUE__CONT_H__
#define __MTL__QUEUE__CONT_H__

namespace MTL
{

  // listItem

  template < class T >
  multiThreadedQueue< T >::listItem::listItem( T d ) :
    _data( d ),
    _next( 0 )
  {
  }

  template < class T >
  multiThreadedQueue< T >::listItem::~listItem()
  {
  }

  template < class T >
  typename multiThreadedQueue< T >::listItem* multiThreadedQueue< T >::listItem::insert( multiThreadedQueue< T >::listItem* item )
  {
    if ( this )
    {
      _next = item;
      return _next;
    }
    else
      return item;
  }

  template < class T >
  typename multiThreadedQueue< T >::listItem* multiThreadedQueue< T >::listItem::remove()
  {
    typename multiThreadedQueue< T >::listItem* next = _next;
    delete this;
    return next;
  }



  // The Queue

#ifndef DISABLE_PTHREAD

  template < class T >
  multiThreadedQueue< T >::multiThreadedQueue() :
    _head( 0 ),
    _tail( 0 ),
    _finished( false )
  {
  }

  template < class T >
  multiThreadedQueue< T >::multiThreadedQueue( T obj ) :
    _head( new typename multiThreadedQueue< T >::listItem( obj ) ),
    _tail( _head.get() ),
    _finished( false )
  {
  }

  template < class T >
  multiThreadedQueue< T >::~multiThreadedQueue()
  {
    if ( _head.get() )
    {
      // Signal poppers!
      _cond.lockSignal();

      _head.lock();
      _tail.lock();

      typename multiThreadedQueue< T >::listItem* current = _tail.get();
      while ( current != 0 ) 
      {
        current = current->remove();
      }

      _tail.unlock();
      _head.unlock();
    }
  }

  template < class T >
  void multiThreadedQueue< T >::push( T obj )
  {
    _head.lock();
    _tail.lock();

    if ( _tail.get() == 0 )
    {
      _head.get() = new typename multiThreadedQueue< T >::listItem( obj );

      _tail.get() = _head.get();
    }
    else
      _head.get() = _head.get()->insert( new typename multiThreadedQueue< T >::listItem( obj ) );

    _tail.unlock();
    _head.unlock();

    // _cond.trySignal();
    _cond.lockSignal();
  }

  template < class T >
  T multiThreadedQueue< T >::pop()
  {
    _tail.lock();

    T temp = _tail.get()->get();
    _tail.get() = _tail.get()->remove();

    _tail.unlock();
    return temp;
  }

  template < class T >
  bool multiThreadedQueue< T >::waitPop( T& value )
  {
    // int res;
    // if ( ( res = _cond.lock(), res ) ) printf( "ERROR: %i - %s\n", res, strerror(res) );
    _cond.lock();

    bool finish;
    bool empty;

    while ( ( finish = this->finished(), empty = this->isEmpty(), ( ! finish ) && empty ) )
    {
      this->_cond.waitOn();
    }

    if ( empty ) 
    {
      _cond.unlock();
      return false;
    }

    _tail.lock();

    value = _tail.get()->get();
    _tail.get() = _tail.get()->remove();

    _tail.unlock();
    _cond.unlock();

    return true;
  }

  template < class T >
  unsigned int multiThreadedQueue< T >::count()
  {
    _head.lock();
    _tail.lock();

    unsigned int counter = 0;
    typename multiThreadedQueue< T >::listItem* current = _head.get();
    while ( current != 0 ) 
    {
      current = current->next();
      ++counter;
    }

    _tail.unlock();
    _head.unlock();

    return counter;
  }

  template < class T >
  void multiThreadedQueue< T >::flush() const 
  {
    _finished.lock();
    _finished.get() = true;
    _finished.unlock();

    _cond.lockBroadcast();
  }

  template < class T >
  bool multiThreadedQueue< T >::finished() const
  {
    _finished.lock();
    bool tmp = _finished.get();
    _finished.unlock();
    return tmp;
  }

  template < class T >
  bool multiThreadedQueue< T >::wait() const
  {
    while ( this->isEmpty() )
    {
      this->_cond.lockWaitOn();
    }
    return ( ! this->finished() );
  }

  template < class T >
  void multiThreadedQueue< T >::waitTimed() const
  {
    while ( this->isEmpty() )
    {
      this->_cond.lockWaitFor( _sleepTime );
    }
  }


  template < class T >
  bool multiThreadedQueue< T >::notEmpty() const
  {
    _tail.lock();
    const typename multiThreadedQueue< T >::listItem* tail = _tail.get();
    _tail.unlock();

    if ( tail != 0 ) return true;
    else return false;
  }

  template < class T >
  bool multiThreadedQueue< T >::isEmpty() const
  {
    return ( ! this->notEmpty() );
  }

  template < class T >
  multiThreadedQueue< T >::operator bool () const
  {
    return this->notEmpty();
  }


#else    ///////////////////////////////////////////////////////////////////// 

  template < class T >
  multiThreadedQueue< T >::multiThreadedQueue() :
    _head( 0 ),
    _tail( 0 )
  {
  }

  template < class T >
  multiThreadedQueue< T >::multiThreadedQueue( T obj ) :
    _head( new typename multiThreadedQueue< T >::listItem( obj ) ),
    _tail( _head )
  {
  }

  template < class T >
  multiThreadedQueue< T >::~multiThreadedQueue()
  {
    if ( _head )
    {
      typename multiThreadedQueue< T >::listItem* current = _tail;
      while ( current != 0 ) 
      {
        current = current->remove();
      }
    }
  }

  template < class T >
  void multiThreadedQueue< T >::push( T obj )
  {
    if ( _tail == 0 )
    {
      _head = new typename multiThreadedQueue< T >::listItem( obj );
      _tail = _head;
    }
    else
      _head = _head->insert( new typename multiThreadedQueue< T >::listItem( obj ) );
  }

  template < class T >
  T multiThreadedQueue< T >::pop()
  {
    T temp = _tail->get();
    _tail = _tail->remove();
    return temp;
  }

  template < class T >
  bool multiThreadedQueue< T >::waitPop( T& value )
  {
    if ( this ->isEmpty() ) return false;
    else 
    {
      value = this->pop();
      return true;
    }
  }

  template < class T >
  unsigned int multiThreadedQueue< T >::count()
  {
    unsigned int counter = 0;
    typename multiThreadedQueue< T >::listItem* current = _head;
    while ( current != 0 ) 
    {
      current = current->next();
      ++counter;
    }

    return counter;
  }

  template < class T >
  void multiThreadedQueue< T >::flush()
  {
    if ( _head )
    {
      typename multiThreadedQueue< T >::listItem* current = _tail;
      while ( current != 0 ) 
      {
        current = current->remove();
      }
    }
    _head = 0;
    _tail = 0;
  }

  template < class T >
  bool multiThreadedQueue< T >::finished() const
  {
    return this->isEmpty();
  }

  template < class T >
  bool multiThreadedQueue< T >::wait() const
  {
    return ( ! this->finished() );
  }

  template < class T >
  void multiThreadedQueue< T >::waitTimed() const
  {
    return;
  }


  template < class T >
  bool multiThreadedQueue< T >::notEmpty() const
  {
    if ( _tail != 0 ) return true;
    else return false;
  }

  template < class T >
  bool multiThreadedQueue< T >::isEmpty() const
  {
    return ( ! this->notEmpty() );
  }

  template < class T >
  multiThreadedQueue< T >::operator bool () const
  {
    return this->notEmpty();
  }


#endif



}

#endif // __MTL__QUEUE__CONT_H__

