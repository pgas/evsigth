Proof of concept of an application class that starts a libevent loop
The goal is to support:
- multiple threads
- exit cleanly when a signal is sent, handling the signal in a dedicated thread
