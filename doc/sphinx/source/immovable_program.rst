.. _immovable_programs:

Immovable Program Reference
===========================
Immovables can have programs that will be executed by the game engine. Programs are required to allow workers to interact with an immovable (e.g. a tree will need a "fall" program to allow woodcutters to remove the tree).

It is not mandatory for immovables to define programs. If the immovable defines a program named ``program``, this program will be started as the main program on creation. Immovables without such a program will simply display their main animation indefinitely.

Programs are defined as Lua tables. Each program must be declared as a subtable in the immovable's Lua table called ``programs`` and have a unique table key. The entries in a program's subtable are the ``actions`` to execute, like this::

   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 5",
         "seed=alder_summer_sapling 180",
      },
      fall = {
         "remove=",
      },
   },

The different command types and the parameters that they take are explained below.

.. highlight:: default

Command Types
^^^^^^^^^^^^^
- `animate`_
- `transform`_
- `grow`_
- `remove`_
- `seed`_
- `construct`_
- `playsound`_


animate
-------
Runs an animation.

Parameter syntax::

  parameters ::= animation duration

Parameter semantics:

``animation``
    The name of an animation (defined in the immovable).
``duration``
    A natural integer specifying the duration in milliseconds.

Starts the specified animation for the immovable. Blocks the execution of the program for the specified duration. (The duration does not have to equal the length of the animation. It will loop around. The animation will not be stopped by this command. It will run until another animation is started.)

transform
---------
Delete this immovable and instantly replace it with a different immovable.

Parameter syntax::

  parameters ::= [bob] name [chance]

Parameter semantics:

``name``
    The name of the immovable to turn into. If the ``bob`` flag is given, this refers to any kind of bob; otherwise to an immovable.
``chance``
    A natural integer in [1,254] defining the chance that the transformation succeeds. The game will generate a random number between 0 and 255 and the program step succeeds if and only if this number is less than ``chance``. Otherwise, the next program step is triggered. If ``chance`` is omitted, the transformation will always succeed.

Deletes the immovable (preventing subsequent program steps from being called) and replaces it with an immovable or bob of the given name. The chance that this program step succeeds can be specified (by default, the step will always succeed).

grow
----
Delete this immovable and instantly replace it with a different immovable with a chance depending on terrain affinity.

Parameter syntax::

  parameters ::= name

Parameter semantics:

``name``
    The name of the immovable to turn into.

Deletes the immovable (preventing subsequent program steps from being called) and replaces it with an immovable of the given name. The chance that this program step succeeds depends on how well this immovable's terrain affinity matches the terrains it grows on. If the growth fails, the next program step is triggered. This command may be used only for immovables with a terrain affinity.

remove
------
Delete this immovable.

Parameter syntax::

  parameters ::= [chance]

Parameter semantics:

``chance``
    A natural integer in [1,254] defining the chance that removing succeeds. The game will generate a random number between 0 and 255 and the program step succeeds if and only if this number is less than ``chance``. Otherwise, the next program step is triggered. If ``chance`` is omitted, the removal will always succeed.

Deletes the immovable (preventing subsequent program steps from being called). The chance that this program step succeeds can be specified (by default, the step will always succeed).

seed
----
Create a new immovable nearby with a chance depending on terrain affinity.

Parameter syntax::

  parameters ::= name factor

Parameter semantics:

``name``
    The name of the immovable to create.
``factor``
    A natural integer in [1,254]. The radius within which the immovable will seed is not limited and is determined by repeatedly generating a random number between 0 and 255 and comparing it with ``factor`` until the comparison fails.

Finds a random location nearby and creates a new immovable with the given name there with a chance depending on *this* immovable's terrain affinity. The chance that such a location will be searched for in a higher radius can be influenced. Note that this program step will consider only *one* random location, and it will only seed there if the terrain is well suited. This command may be used only for immovables with a terrain affinity.

construct
---------
Blocks execution until enough wares have been delivered to this immovable by a worker.

Parameter syntax::

  parameters ::= animation build decay

Parameter semantics:

``animation``
    The animation to display while the immovable is being constructed.
``build``
    The duration of each construction step in milliseconds for visualising the construction progress. Used only in drawing code.
``decay``
    When no construction material has been delivered for this many milliseconds, the construction progress starts to gradually reverse.

Blocks execution of subsequent programs until enough wares have been delivered to this immovable by a worker. The wares to deliver are specified in the immovable's ``buildcost`` table which is mandatory for immovables using the ``construct`` command. If no wares are being delivered for a while, the progress gradually starts to reverse, increasing the number of wares left to deliver. If the immovable keeps decaying, it will eventually be removed.

playsound
---------
Plays a sound effect.

Parameter syntax::

  parameters ::= soundFX [priority]

Parameter semantics:

``filepath``
    The path/base_filename of a soundFX (relative to the data directory).
``priority``
    An integer. If omitted, 127 is used.

Plays the specified soundFX with the specified priority. Whether the soundFX is actually played is determined by the sound handler.
