
#ifndef MTL_THREAD_SAFE_BUFFER_H_
#define MTL_THREAD_SAFE_BUFFER_H_

#include <mutex>
#include <iostream>


namespace MTL
{
  typedef std::unique_lock< std::mutex > UniqueLock;
  typedef std::lock_guard< std::mutex > GuardLock;

  /*
   * A singly-linked queue implementation where the entry and exit are mutexed.
   * This allows parallel pushes AND pops to happen.
   * List interrogation is not permitted. No iterators.
   */
  template < class TYPE >
  class MutexedBuffer
  {
    private :
      // Define the element container struct
      struct Element
      {
        std::mutex mutex;
        TYPE data;
        Element* next;

        Element( TYPE d ) : data( d ), next( nullptr ) {}
      };

      // Control the start and end only
      std::mutex _mutexStart;
      std::mutex _mutexEnd;
      // Start and end markers
      Element* _start;
      Element* _end;

      // Count the number of elements
      size_t _elementCount;
      std::mutex _mutexCounter;

    public:
      MutexedBuffer();

      ~MutexedBuffer();

      // Push onto front of the buffer
      void push( TYPE );

      // Pop from back of the buffer
      bool pop( TYPE& );

//      bool next( TYPE& );

      void clear();

      size_t size();

      bool empty();

  };


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Template member function definitions

  template < class TYPE >
  MutexedBuffer< TYPE >::MutexedBuffer() :
    _mutexStart(),
    _mutexEnd(),
    _start( nullptr ),
    _end( nullptr )
  {
  }


  template < class TYPE >
  MutexedBuffer< TYPE >::~MutexedBuffer()
  {
    this->clear();
  }


  template < class TYPE >
  void MutexedBuffer< TYPE >::clear()
  {
    // Lock the other side of the buffer. No one can push
    UniqueLock startLock( _mutexStart );

    // Lock the counter
    UniqueLock counterLock( _mutexCounter );

    // Lock the end of the buffer - No one can pop
    UniqueLock endLock( _mutexEnd );

    // Remove the pointers to the buffer so that a new one might be started in the mean time
    MutexedBuffer::Element* current = _end;
    MutexedBuffer::Element* next = nullptr;
    _start = nullptr;
    _end = nullptr;

    // Set the size to zero
    _elementCount = 0;

    // Release the start
    endLock.unlock();

    // Release the counter
    counterLock.unlock();

    // Release the end
    startLock.unlock();

    // Now we don't need to mutex anything. No one can have a reference to any of this data
    while ( current != nullptr )
    {
      // Cache the next element
      next = current->next;

      // Delete the current element
      delete current;
      current = next;
    }
  }


  template < class TYPE >
  size_t MutexedBuffer< TYPE >::size()
  {
    GuardLock lock( _mutexCounter );
    return _elementCount;
  }


  template < class TYPE >
  bool MutexedBuffer< TYPE >::empty()
  {
    GuardLock lock( _mutexCounter );
    return _elementCount == 0;
  }


  template < class TYPE >
  void MutexedBuffer< TYPE >::push( TYPE val )
  {
    // Create a new element
    MutexedBuffer::Element* element = new MutexedBuffer::Element( val );

    // Lock the start of the buffer
    UniqueLock startLock( _mutexStart );

    // Lock the counter to check the status
    UniqueLock counterLock( _mutexCounter );

    switch ( _elementCount )
    {
      case 0 :
        {
          UniqueLock endLock( _mutexEnd );
          _start = element;
          _end = element;
          ++_elementCount;
          endLock.unlock();
          counterLock.unlock();
        }
        break;

      case 1 : // If there's only one element - block the pop
        {
          UniqueLock endLock( _mutexEnd );
          ++_elementCount;
          counterLock.unlock();

          // Update the start/end pointer and release the end lock - no longer critical
          _start->next = element;
          endLock.unlock();

          // set the start element
          _start = element;
        }
        break;

      default :
        {
          ++_elementCount;
          // Update the start/end pointer and release the end lock - no longer critical
          _start->next = element;
          counterLock.unlock();

          // set the start element
          _start = element;
        }
        break;

    }

    startLock.unlock();
  }


//  template < class TYPE >
//  void MutexedBuffer< TYPE >::pop()
//  {
//    // Lock the start marker
//    UniqueLock endLock( _mutexEnd );
//
//    // Return if empty buffer
//    if ( _end == nullptr ) return;
//
//    // Check that we can own the last element
//    UniqueLock lastLock( _end->mutex );
//
//    // Grab the element
//    MutexedBuffer::Element* element = _end;
//
//    // Grab the counter
//    UniqueLock counterLock( _mutexCounter );
//
//    // update the end pointer
//    _end = _end->previous;
//    
//    // Update the counter
//    --_elementCount;
//
//    // Release the locks
//    counterLock.unlock();
//    lastLock.unlock();
//    endLock.unlock();
//
//    // Delete the element
//    delete _end;
//  }
  

  template < class TYPE >
  bool MutexedBuffer< TYPE >::pop( TYPE& data )
  {
    // Grab the counter
    UniqueLock counterLock( _mutexCounter );

    switch ( _elementCount )
    {
      case 0 :
        counterLock.unlock();
        return false;
        break;

//      case 1 :
//        {
//          // Aquire the end pointer - no one else can pop
//          UniqueLock endLock( _mutexEnd );
//
//          // Update and release the counter
//          --_elementCount;
//          counterLock.unlock();
//
//
//          // Check that we can own the last element
//          UniqueLock lastLock( _end->mutex );
//
//          // Grab the element
//          MutexedBuffer::Element* element = _end;
//
//          // update the end pointer
//          _end = _end->next;
//
//          // Release the end pointer
//          lastLock.unlock();
//          endLock.unlock();
//
//          // Copy the data and remove the element
//          data = element->data;
//          delete element;
//        }
//        break;


      default :
        {
          // Aquire the end pointer - no one else can pop
          UniqueLock endLock( _mutexEnd );

          // Update and release the counter
          --_elementCount;
          counterLock.unlock();


//          // Check that we can own the last element
//          UniqueLock lastLock( _end->mutex ); // Is this optional?

          // Grab the element
          MutexedBuffer::Element* element = _end;

          // update the end pointer
          _end = _end->next;

          // Release the end pointer
          endLock.unlock();

//          lastLock.unlock();

          // Copy the data and remove the element
          data = element->data;
          delete element;
        }
        break;
    }
    return true;
  }
  
}

#endif // MTL_THREAD_SAFE_BUFFER_H_

