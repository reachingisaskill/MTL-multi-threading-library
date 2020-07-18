
#include "MTL_CircularLatch.h"

#include <iostream>


namespace MTL
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // CircularLatch::ListItem Member function definitions

  CircularLatch::ListItem::ListItem( std::thread::id id, ListItem* n, AtomicThreadID& thread ) :
    _id( id ),
    _next( n ),
    _currentThread( thread )
  {
  }


  CircularLatch::ListItem::~ListItem()
  {
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
  }


  void CircularLatch::ListItem::pass()
  {
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
    std::cout << "Destructing" << std::endl;
    if ( _listEnd != nullptr )
    {
      ListItem* current = _listEnd;
      do
      {
        ListItem* next = current->next();
        delete current;
        std::cout << " delete " << std::endl;
        current = next;
      }
      while ( current != _listEnd );
    }
  }


  CircularLatch::Handle CircularLatch::requestHandle( unsigned int handleNumber )
  {
    // Lock access to the list
    std::unique_lock< std::mutex > lock( _listMutex );

    // Wait until the number of handles equals the requested handleNumber
    _startCondition.wait( lock, [this, handleNumber]()->bool{ return ( this->_currentNumberHandles == handleNumber ); } );


    // Unique thread identifier
    std::thread::id id = std::this_thread::get_id();

    // Create and append the new list item
    if ( _listEnd == nullptr )
    {
      ListItem* newItem = new ListItem( id, nullptr, _currentThread );
      _listEnd = newItem->append( newItem );
      _currentThread = id;
    }
    else
    {
      ListItem* newItem = new ListItem( id, _listEnd->next(), _currentThread );
      _listEnd = _listEnd->append( newItem );
    }

    // Create a handle for the item
    Handle handle( *_listEnd );

    // Increment the counter
    _currentNumberHandles += 1;

    std::cout << "NUMBER = " << _currentNumberHandles << std::endl;

    // Notify the update
    _startCondition.notify_all();

    // If that's all, we notify the condition variable, otherwise we wait on it.
    _startCondition.wait( lock, [this]() -> bool { return (this->_currentNumberHandles == this->_totalHandles); } );
    std::cout << "GO : " << handleNumber << std::endl;
    lock.unlock();

//    // If that's all, we notify the condition variable, otherwise we wait on it.
//    if ( _currentNumberHandles == _totalHandles )
//    {
//      std::cout << "GO : " << handleNumber << std::endl;
//      lock.unlock();
//    }
//    else
//    {
//      _startCondition.wait( lock, [this]() -> bool { return (this->_currentNumberHandles == this->_totalHandles); } );
//      std::cout << "GO : " << handleNumber << std::endl;
//      lock.unlock();
//    }

    // Return the handle
    return handle; 
  }

}

