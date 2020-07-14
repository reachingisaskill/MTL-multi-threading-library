
#ifndef MTL_THREAD_SAFE_CIRCULAR_LATCH_H__
#define MTL_THREAD_SAFE_CIRCULAR_LATCH_H__

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace MTL
{

  class CircularLatch
  {
    // Make it a signleton - same as the mutex objects
    CircularLatch( const CircularLatch& ) = delete;
    CircularLatch( CircularLatch&& );
    CircularLatch& operator=( const CircularLatch& ) = delete;
    CircularLatch& operator=( CircularLatch&& ) = delete;

    typedef std::atomic< std::thread::id > AtomicThreadID;

    private :

      class ListItem
      {
        // Make it a singleton
        ListItem( const ListItem& ) = delete;
        ListItem( ListItem&& ) = delete;
        ListItem& operator=( const ListItem& ) = delete;
        ListItem& operator=( ListItem&& ) = delete;

        private:
          // Unique identifier for the owning thread
          const std::thread::id _id;

          // Reference to the mutex in question
          std::mutex& _theMutex;

          // Pointer to the next item in the list
          ListItem* _next;

          // ID of the current thread allowed access
          AtomicThreadID& _currentThread;

        public:
          // Construction
          ListItem( std::mutex&, std::thread::id, ListItem*, AtomicThreadID& );

          // Destruction
          ~ListItem();

          // Appends a ListItem onto the next pointer
          ListItem* append( ListItem* );

          // Return a copy of the next pointer
          ListItem* next() const { return _next; }

          // "blocks" the calling thread until the mutex is aquired
          void aquire();

          // Allows the next item in the list to lock the mutex.
          void pass();

          // Returns true if this ListItem owns a locked mutex
          bool ownsMutex() const;

      };


    public :

      class Handle
      {
        // Non-copyable and non-assignable
        Handle( const Handle& ) = delete;
        Handle& operator=( const Handle& ) = delete;
        Handle& operator=( Handle&& ) = delete;

        private:
          ListItem& _listItem;

          bool _isLocked;

        public:
          explicit Handle( ListItem& );

          Handle( Handle&& );

          ~Handle();

          bool isLocked() const { return _isLocked; }

          void lock();

          void unlock();
      };


    private :
      // The primary mutex that controls access to the primary data 
      std::mutex _theMutex;

      // Atomic variable that records which thread has control
      AtomicThreadID _currentThread;

      // Mutex that controls access to the list items
      std::mutex _listMutex;

      // The expected number of handles
      const unsigned int _totalHandles;

      // Pointer to the start of a circular list of handles
      ListItem* _listEnd;
      
      // Condition variable ensuring that all handles are configured before they are used
      std::condition_variable _startCondition;

      // Count the current number of handles
      unsigned int _currentNumberHandles;

    public :
      // Constructor
      CircularLatch( unsigned int );

      // Destructor
      ~CircularLatch();

      // Request a handle to the latch
      // Order of latching threads is defined by the order in which this function is called
      Handle requestHandle();

      // Waits on a condition variable until the expected number of handles have been requested.
      void wait();
  };


}

#endif // MTL_THREAD_SAFE_CIRCULAR_LATCH_H__

