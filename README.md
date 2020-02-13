
# MTL - Multi Threading Library

## Basics

Some useful functionality and classes for implementing pthreads into devloping code simply and 
easily.

- Mutex Class

  Defines an object that uses the pthreads mutex functionality to make data classes thread safe.
  It must be inherited by the data class such that the functions *lock()*, *trylock()* and 
  *unlock()* are available.

- Synchronisation object

  Waits for the completion of provided threads before allowing the code execution to continue.

- Queue

  Specify a number of threads and a function to perform on the elements of the queue and each 
  element in the queue will be passed to the function in a new thread, depeneding on the number
  specified.

- List

  Specify a list of data objects, a set of functions to perform on the data objects and a control
  function. Designed to implement the *mapping* part of a map-reduce framework. The control function
  allows for elements to be removed or furthed edited (the *reduce* part) before the next object is
  processed.


## TODO
- Fix the bug thats causing the list test ot freeze when *go(...)* is called.



