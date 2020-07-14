
#include "MTL_CircularLatch.h"

#include <iostream>


namespace MTL
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // CircularLatch::ListItem Member function definitions

  CircularLatch::ListItem::ListItem( std::mutex& m, std::thread::id id, ListItem* n, AtomicThreadID& thread ) :
    _id( id ),
    _theMutex( m ),
    _next( n ),
    _currentThread( thread )
  {
  }


  CircularLatch::ListItem::~ListItem()
  {
    // Possible to destroy object whilst owning a mutex in owning thread.
    // I don't think I can protect against it very easily.
    // Hopefully the user isn't an idiot...
  }


  CircularLatch::ListItem* CircularLatch::ListItem::append( ListItem* li )
  {
    _next = li;
    return _next;
  }


  void CircularLatch::ListItem::aquire()
  {
    // Wait until it's our turn
    while ( _currentThread.load( std::memory_order_relaxed ) != _id );

    // Wait until its successful
//    while ( ! _theMutex.try_lock() );

    // Once the thread variable is updated this must be available!
    _theMutex.lock();
  }


  void CircularLatch::ListItem::pass()
  {
    // Release the mutex
    _theMutex.unlock();

    // Store the next thread id in the current thread variable
    _currentThread.store( _next->_id, std::memory_order_relaxed );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // CircularLatch::Handle Member function definitions

  CircularLatch::Handle::Handle( ListItem& li ) :
    _listItem( li ),
    _isLocked( false )
  {
  }


  CircularLatch::Handle::Handle( Handle&& h ) :
    _listItem( h._listItem ),
    _isLocked( std::move( h._isLocked ) )
  {
  }


  CircularLatch::Handle::~Handle()
  {
  }


  void CircularLatch::Handle::lock()
  {
    _listItem.aquire();
    _isLocked = true;
  }


  void CircularLatch::Handle::unlock()
  {
    _listItem.pass();
    _isLocked = false;
  }

////////////////////////////////////////////////////////////////////////////////////////////////////
  // CircularLatch Member function definitions

  CircularLatch::CircularLatch( unsigned int N ) :
    _theMutex(),
    _currentThread(),
    _listMutex(),
    _totalHandles( N ),
    _listEnd( nullptr ),
    _startCondition(),
    _currentNumberHandles( 0 )
  {
  }


  CircularLatch::~CircularLatch()
  {
    if ( _listEnd != nullptr )
    {
      ListItem* current = _listEnd->next();
      do
      {
        ListItem* next = current->next();
        delete current;
        current = next;
      }
      while ( current != _listEnd );
    }
  }


  CircularLatch::Handle CircularLatch::requestHandle()
  {
    // Lock access to the list
    std::unique_lock< std::mutex > lock( _listMutex );

    std::thread::id id = std::this_thread::get_id();

    // Create and append the new list item
    ListItem* newItem;
    if ( _listEnd == nullptr )
    {
      newItem = new ListItem( _theMutex, id, nullptr, _currentThread );
      _listEnd = newItem->append( newItem );
      _currentThread = id;
    }
    else
    {
      newItem = new ListItem( _theMutex, id, _listEnd->next(), _currentThread );
      _listEnd = _listEnd->append( newItem );
    }

    // Create a handle for the item
    Handle handle( *_listEnd );

    // Increment the counter
    _currentNumberHandles += 1;

    std::cout << "NUMBER = " << _currentNumberHandles << std::endl;

    // If that's all, we notify the condition variable
    if ( _currentNumberHandles == _totalHandles )
    {
      lock.unlock();
      _startCondition.notify_all();
    }
    else
    {
      lock.unlock();
    }

    // Return the handle as an rvalue
    return handle; 
  }


  void CircularLatch::wait()
  {
    // Wait for the start condition to be reached
    std::unique_lock< std::mutex > lock( _listMutex );
    _startCondition.wait( lock, [this]() -> bool { return (this->_currentNumberHandles == this->_totalHandles); } );
    lock.unlock();

    std::cout << "GO!" << std::endl;
  }

}

