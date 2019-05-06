# WebServer
High concurrency &amp; Linux platform 

Use the epoll model to listen for events, small files are sent using the sendfile (thread pool), others are sent using aio, and php is parsed with fastcgi.The program uses a memory pool to reduce memory overhead.Also developed shared memory and process locks. 
  
Currently only supports https, and does not support asp. 
   
Returning the content-type in the header still can't return other more types.

When using virtual machines (cpu: 2 core, ram: 2 g, system: ubuntu 16.4) and apachebench for testing, far beyond apache. And nginx≈50ms, my≈60ms complete 50% (-c 10000,-n 20000).
