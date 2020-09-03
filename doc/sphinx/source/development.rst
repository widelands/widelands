Developers Information
======================

This file is simply a collection of thoughts, useful pieces of
information, etc.


Parallel Simulation
-------------------
The game logic is supposed to be affected only by the following factors:

- the pseudo RNG (which is completely determined by the initial seed)

- the starting conditions

- the sequence of player commands sent to the Cmd_Queue

As long as the gameplay logic stays the same (no code changes / no changes
to conf files), and the above factors remain the same, the outcome must
be the same.

This property of the Widelands game logic is extremely important for two
reasons:

- Multiplayer by parallel simulation:

  Every host keeps the complete state of the simulation in memory, and only
  player commands are exchanged to keep network traffic low. The properties
  above guarantuee that there is no desync.
  
- Demo recording

  By creating a savegame followed by a sequence of player commands, it is
  possible to create a demo recording of a game (similar to Age of Empires,
  for example)


Multithreading
--------------
The game is multithreaded – logic progression, AI decisions and game rendering
are performed in parallel.

Some pieces of the code assume that the game state does not change while they
are executing. For this, use the ``struct MutexLock`` (see documentation in
``base/multithreading.h``). Take care to lock a mutex only for as long as
really necessary, otherwise you defeat the whole point of multithreading.
When in doubt, first write your code without obtaining a lock, and if it
crashes, create a lock and extend its scope one line at a time until you
can no longer reproduce the crash.

If you ever notice that the user interface is responding slowly or hangs,
you have probably created a lock with far too large a scope.

Static variables that could be modified by multiple threads
concurrently are a no-go.

Some of our external libraries are not thread-safe. Therefore, images must
not be loaded or rendered by any thread other than the one that performed
the initialization of the image-related libaries. This also goes for pre-
rendering text, which is also a kind of graphics I/O.

If you need to perform a calculation instantly but you may be in the wrong
thread, you can use a ``NoteDelayedCheck``, like this:

      .. code-block:: c++

         int result = 0;
         bool done = false;
         NoteDelayedCheck::instantiate(this, [this, &result, &done]() {
            result = non_thread_safe_function();
            done = true;
         });
         while (!done) {
            SDL_Delay(20);
         }


Why std::set<SomePointer*> is a bad idea
----------------------------------------
Well, actually it's only a bad idea in the game logic, here's why.

std::set<Foo*> uses pointer comparisons to sort the set. Pointers aren't
system-independent (in fact, they might even differ from one run to the
next on the same system). Therefore, this construct *must not* be used in
the game logic.
Similar constructs are affected as well. Basically, never use pointer
arithmetics for decision-making.

