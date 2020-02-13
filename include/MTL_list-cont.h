
#ifndef __STDEXTS__MULTITHREADED_LINKED_LIST_CONT_H__
#define __STDEXTS__MULTITHREADED_LINKED_LIST_CONT_H__


namespace MTL
{

  template < class T, int N >
  multiThreadedList< T, N >::multiThreadedList() :
    _list( 0 ),
    _lastCount( 0 )
  {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_NORMAL );
    pthread_mutexattr_setrobust( &attr, PTHREAD_MUTEX_ROBUST );
    pthread_mutex_init( &_mutex, &attr );
    _controlThread = 0;
    _controlFunction = 0;
    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadFunctions[n] = 0;
      _threads[n] = 0;

      _threadData[n].threadWaiting = false;
      _threadData[n].controlWaiting = false;
      pthread_mutex_init( &_threadData[n].mutex, &attr );
      pthread_cond_init( &_threadData[n].condition, 0 );
    }
  }

  template < class T, int N >
  multiThreadedList< T, N >::multiThreadedList( T obj ) :
    _list( new typename multiThreadedList< T, N >::listItem( obj ) ),
    _lastCount( 1 )
  {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_NORMAL );
    pthread_mutexattr_setrobust( &attr, PTHREAD_MUTEX_ROBUST );
    pthread_mutex_init( &_mutex, &attr );
    _controlThread = 0;
    _controlFunction = 0;
    for ( unsigned int n = 0; n < N; ++n )
    {
      _threadFunctions[n] = 0;
      _threads[n] = 0;

      _threadData[n].threadWaiting = false;
      _threadData[n].controlWaiting = false;
      pthread_mutex_init( &_threadData[n].mutex, attr );
      pthread_cond_init( &_threadData[n].condition, 0 );
    }
  }

  template <  class T, int N >
  multiThreadedList< T, N >& multiThreadedList< T, N >::push( T obj )
  {
    if ( _list != 0 )
    {
      listItem* temp = new typename multiThreadedList< T, N >::listItem( obj );
      temp->_prev = _list->_prev;
      temp->_next = _list;
      _list->_prev->_next = temp;
      _list->_prev = temp;
    }
    else
      _list = new typename multiThreadedList< T, N >::listItem( obj );

    return *this;
  }

  template <  class T, int N >
  unsigned int multiThreadedList< T, N >::count()
  {
    long unsigned int counter = 0;
    const listItem* currentItem = _list;
    if ( currentItem == 0 ) return 0;
    do
    {
      ++counter;
      currentItem = currentItem->next();
    } 
    while ( currentItem != _list );

    _lastCount = counter;
    return counter;
  }

  //////////////////////////////////////////////////////////////////////////////

  template <  class T, int N >
  multiThreadedList< T, N >::listItem::listItem( T d, multiThreadedList< T, N >::listItem* prev, multiThreadedList< T, N >::listItem* next ) :
    _data( d ),
    _prev( ( prev ? prev : this ) ),
    _next( ( next ? next : this ) ),
    _status( multiThreadedList< T, N >::status_continue )
  {
    if ( _prev ) _prev->_next = this;
    if ( _next ) _next->_prev = this;
  }

  template <  class T, int N >
  multiThreadedList< T, N >::listItem::~listItem()
  {
  }

  template <  class T, int N >
  typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::_clear()
  {
    typename multiThreadedList< T, N >::listItem* next = _next;
    while ( next != this ) next = next->remove();
    this->remove();
    return 0;
  }

  /*
  template <  class T, unsigned int N >
  typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::append( multiThreadedList< T, N >::listItem* item )
  {
    if ( _next )
    {
      _next->_prev = item;
      item->_next = _next;
    }
    item->_prev = this;
    _next = item;

    return _next;
  }
  */

  template <  class T, int N >
  typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::remove()
  {
    if ( _next ) _next->_prev = this->_prev;
    if ( _prev ) _prev->_next = this->_next;

    typename multiThreadedList< T, N >::listItem* next = _next;
    this->_prev = 0;
    this->_next = 0;

    delete this;
    return next;
  }

  template <  class T, int N >
  typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::next()
  {
    return _next;
  }

  template <  class T, int N >
  const typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::next() const
  {
    return _next;
  }

  template <  class T, int N >
  typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::prev()
  {
    return _prev;
  }

  template <  class T, int N >
  const typename multiThreadedList< T, N >::listItem* multiThreadedList< T, N >::listItem::prev() const
  {
    return _prev;
  }

}

#endif // __STDEXTS__MULTITHREADED_LINKED_LIST_CONT_H__

