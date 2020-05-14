
#ifndef MTL_THREAD_SAFE_LINKED_LIST_H__
#define MTL_THREAD_SAFE_LINKED_LIST_H__

#include <mutex>
#include <iostream>


namespace MTL
{
  typedef std::unique_lock< std::mutex > UniqueLock;
  typedef std::lock_guard< std::mutex > GuardLock;

  /*
   * A singly-linked list implementation where each element is mutexed.
   * this allows parallel reads AND writes AND deletions to happen.
   * In order to facilitate this it MUST be a singly linked STACK and iterators
   * can only travel FORWARD!
   */

  template < class TYPE >
  class MutexedList
  {

    private :

      class Element
      {
        public :
          std::mutex mutex_data;
          std::mutex mutex_next;

          TYPE data;
          Element* next;

          Element( TYPE d, Element* n = nullptr ) : data( d ), next( n ) {}
      };

    public:

      class Iterator
      {
          Iterator( const Iterator& ) = delete;
          Iterator& operator=( const Iterator& ) = delete;
          Iterator& operator=( Iterator&& ) = delete;
        private:
          Element* _element;
          Element** _previous;
          UniqueLock _lock_element;
          UniqueLock _lock_pointer;

        public:
          Iterator();

          Iterator( Element*, Element**, UniqueLock&& );

          Iterator( Iterator&& );

          ~Iterator();

          bool next();

          bool isGood();

          void erase();

          void destroy();

          TYPE& get();
      };


    private:
      std::mutex _mutexStart;
      Element* _start;

    public:
      MutexedList();

      ~MutexedList();

      // Push onto front of list
      void push( TYPE );

      // Read/pop from front of list
      void pop();
      bool front( TYPE& );

      Iterator begin();
//      Iterator end(); // Not accessible as a stack - use Iterator::isGood()

      void clear();


      size_t size() const;

  };


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Template member function definitions

  template < class TYPE >
  MutexedList< TYPE >::MutexedList() :
    _mutexStart(),
    _start( nullptr )
  {
  }


  template < class TYPE >
  MutexedList< TYPE >::~MutexedList()
  {
    this->clear();
  }


  template < class TYPE >
  void MutexedList< TYPE >::clear()
  {
    // Lock the end of the list - No one else can start iterating
    UniqueLock startLock( _mutexStart );

    // Decapitate the list so that a new one might be started
    MutexedList::Element* list_start = _start;
    MutexedList::Element* current = list_start;
    _start = nullptr;

    // Release the start pointer lock
    startLock.unlock();

    while ( current != nullptr )
    {
      // Aquire the current element mutexes
      UniqueLock lock_current( current->mutex_data );
      UniqueLock lock_pointer( current->mutex_next );

      if ( current->next == nullptr )
      {
        // Remove the locks before deletion
        lock_pointer.unlock();
        lock_current.unlock();

        // Delete the last current element
        delete current;
        current = nullptr;
      }
      else
      {
        // Lock the next element - don't need to do anything - just make sure no one else can access it
        // This is ensured as we still have a lock on the head of the stack
        UniqueLock lock_next( current->next->mutex_data );

        // Find the next element
        Element* next = current->next;

        // Remove the locks before deletion
        lock_next.unlock();
        lock_pointer.unlock();
        lock_current.unlock();

        // Delete the current element and update
        delete current;
        current = next;
      }
    }
  }


  template < class TYPE >
  void MutexedList< TYPE >::push( TYPE val )
  {
    // Lock the start of the stack
    GuardLock startLock( _mutexStart );

    // Create a new element
    MutexedList::Element* element = new MutexedList::Element( val, _start );

    // Update the start pointer
    _start = element;
  }


  template < class TYPE >
  void MutexedList< TYPE >::pop()
  {
    // Lock the start marker
    GuardLock startGuard( _mutexStart );

    // Return if empty list
    if ( _start == nullptr ) return;

    // Check that we can own the first element and its next-pointer
    UniqueLock firstLock( _start->mutex_data );
    UniqueLock firstPointerLock( _start->mutex_next );

    // Find the next element
    MutexedList::Element* next = _start->next;

    // We then release them because we know no-one is coming behind us - we own the start pointer
    firstPointerLock.unlock();
    firstLock.unlock();

    // Delete the start element and update the pointer.
    delete _start;
    _start = next;
  }
  

  template < class TYPE >
  bool  MutexedList< TYPE >::front( TYPE& value )
  {
    // Lock the start marker
    GuardLock startGuard( _mutexStart );

    if ( _start == nullptr ) return false;

    // Lock the first element
    UniqueLock firstLock( _start->mutex_data );

    // Return the stored value
    value = _start->data;

    firstLock.unlock();
    return true;
  }


  template < class TYPE >
  typename MutexedList<TYPE>::Iterator  MutexedList< TYPE >::begin()
  {
    return MutexedList<TYPE>::Iterator( _start, &_start, UniqueLock( _mutexStart ) );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Iterator member functions

  template < class TYPE >
  MutexedList< TYPE >::Iterator::Iterator() :
    _element( nullptr ),
    _previous( nullptr ),
    _lock_element(),
    _lock_pointer()
  {
  }


  template < class TYPE >
  MutexedList< TYPE >::Iterator::Iterator( Element* element, Element** prev, UniqueLock&& prev_lock ) :
    _element( nullptr ),
    _previous( prev ),
    _lock_element(),
    _lock_pointer( std::move( prev_lock ) )
  {
    if ( element != nullptr )
    {
      _lock_element = UniqueLock( element->mutex_data );
      _element = element;
    }
    else
    {
      _previous = nullptr;
      _lock_pointer.unlock();
    }
  }


  template < class TYPE >
  MutexedList< TYPE >::Iterator::Iterator( Iterator&& it ) :
    _element( std::move( it._element ) ),
    _previous( std::move( it._previous ) ),
    _lock_element( std::move( it._lock_element ) ),
    _lock_pointer( std::move( it._lock_pointer ) )
  {
  }


  template < class TYPE >
  MutexedList< TYPE >::Iterator::~Iterator()
  {
    this->destroy();
  }


  template < class TYPE >
  bool MutexedList< TYPE >::Iterator::next()
  {
    // If iterator is now invalid
    if ( _element == nullptr ) return false;

    // Change the previous element pointer - lock-pointer is no longer needed
    _previous = &_element->next;

    // Move lock-pointer to the current *next* pointer
    _lock_pointer = UniqueLock( _element->mutex_next );

    // if *next* pointer good
    if ( _element->next != nullptr )
    {
      _element = _element->next;
      _lock_element = UniqueLock( _element->mutex_data );

      return true;
    }
    else
    { 
      // Make the iterator bad.
      _element = nullptr;
      _previous = nullptr;
      _lock_pointer.unlock();
      _lock_element.unlock();

      return false;
    }
  }


  template < class TYPE >
  bool  MutexedList< TYPE >::Iterator::isGood()
  {
    return ( _element != nullptr );
  }


  template < class TYPE >
  TYPE&  MutexedList< TYPE >::Iterator::get()
  {
    return _element->data;
  }


  template < class TYPE >
  void  MutexedList< TYPE >::Iterator::erase()
  {
    // If the iterator is bad
    if ( _element == nullptr )
    {
      return;
    }

    // Aquire the next pointer - ensure no one is still using it
    UniqueLock nextLock( _element->mutex_next );

    // find the next element
    Element* next = _element->next;

    // Release it again
    nextLock.unlock();
    // Release the element lock before deletion
    _lock_element.unlock();

    delete _element;
    _element = next;

    // If this isn't the start of the stack
    if ( _previous != nullptr )
    {
      // Update the previous pointer
      (*_previous) = _element;
    }

    // If the new element exists
    if ( _element != nullptr )
    {
      _lock_element = UniqueLock( _element->mutex_data );
    }
    else
    { // Make the iterator bad.
      _element = nullptr;
      _lock_pointer.unlock();
    }
  }


  template < class TYPE >
  void  MutexedList< TYPE >::Iterator::destroy()
  {
    if ( _element != nullptr )
    {
      _element = nullptr;
      _previous = nullptr;
      _lock_pointer.unlock();
      _lock_element.unlock();
    }
  }

}

#endif // MTL_THREAD_SAFE_LINKED_LIST_H__

