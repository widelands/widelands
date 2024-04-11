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

- Replay recording

  By creating a savegame followed by a sequence of player commands, it is
  possible to create a replay (demo) recording of a game (similar to Age of Empires,
  for example)


Why ``std::set<SomePointer*>`` is a bad idea
--------------------------------------------
Well, actually it's only a bad idea in the game logic, here's why.

``std::set<Foo*>`` uses pointer comparisons to sort the set. Pointers aren't
system-independent (in fact, they might even differ from one run to the
next on the same system). Therefore, this construct *must not* be used in
the game logic.
Similar constructs are affected as well. Basically, never use pointer
arithmetics for decision-making.

