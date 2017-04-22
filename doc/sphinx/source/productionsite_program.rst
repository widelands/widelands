Productionsite Program Reference
================================
Productionsites can have programs that will be executed by the game engine. Each productionsite must have a program named ``work``, which will be started automatically when the productionsite is created in the game, and then repeated until the productionsite is destroyed.

Programs are defined as Lua tables. Each program must be declared as a subtable in the productionsite's Lua table called ``programs`` and have a unique table key. The entries in a program's subtable are the translatable ``descname`` and the table of ``actions`` to execute, like this::

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            <list of actions>
         }
      },
   },

The translations for ``descname`` can also be fetched by ``pgettext`` to disambiguate. We recommend that you do this whenever workers are referenced, or if your tribes have multiple wares with the same name::

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("atlanteans_building", "recruiting soldier"),
         actions = {
            <list of actions>
         }
      },
   },

A program can call another program, for example::

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration",
            "call=produce_snack",
            "return=skipped"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            <list of actions>
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            <list of actions>
         }
      },
   },

A program's actions consist of a sequence of commands. A command is written as ``<type>=<parameters>``::


   produce_snack = {
      -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
      descname = _"preparing a snack",
      actions = {
         "return=skipped unless economy needs snack",
         "sleep=5000",
         "consume=barbarians_bread fish,meat beer",
         "play_sound=sound/barbarians/taverns inn 100",
         "animate=working 22000",
         "sleep=10000",
         "produce=snack"
      }
   },


The different command types and the parameters that they take are explained below.


Command Types
^^^^^^^^^^^^^
- `return`_
- `call`_
- `worker`_
- `sleep`_
- `animate`_
- `consume`_
- `produce`_
- `mine`_
- `check_soldier`_
- `train`_
- `play_sound`_


return
------
Returns from the program.

Parameter syntax::

    parameters        ::= return_value [condition_part]
    return_value      ::= Failed | Completed | Skipped
    Failed            ::= failed
    Completed         ::= completed
    Skipped           ::= skipped
    condition_part    ::= when_condition | unless_condition
    when_condition    ::= when condition {and condition}
    unless_condition  ::= unless condition {or condition}
    condition         ::= negation | economy_condition | workers_condition
    negation          ::= not condition
    economy_condition ::= economy economy_needs
    workers_condition ::= workers need_experience
    economy_needs     ::= needs ware_type
    need_experience   ::= need experience

Parameter semantics:

``return_value``
    If return_value is Failed or Completed, the productionsite's
    statistics is updated accordingly. If return_value is Skipped, the
    statistics are not affected.
``condition``
    A boolean condition that can be evaluated to true or false.
``condition_part``
    If omitted, the return is unconditional.
``when_condition``
    This will cause the program to return when all conditions are true.
``unless_condition``
    This will cause the program to return unless some condition is true.
``ware_type``
    The name of a ware type (defined in the tribe). A ware type may only
    appear once in the command.
``economy_needs``
    The result of this condition depends on whether the economy that this
    productionsite belongs to needs a ware of the specified type. How
    this is determined is defined by the economy.

Aborts the execution of the program and sets a return value. Updates the productionsite's statistics depending on the return value.

.. note:: If the execution reaches the end of the program, the return value is implicitly set to Completed.


call
----
Calls a program of the productionsite.

Parameter syntax::

  parameters                 ::= program [failure_handling_directive]
  failure_handling_directive ::= on failure failure_handling_method
  failure_handling_method    ::= Fail | Repeat | Ignore
  Fail                       ::= fail
  Repeat                     ::= repeat
  Ignore                     ::= ignore

Parameter semantics:

``program``
    The name of a program defined in the productionsite.
``failure_handling_method``
    Specifies how to handle a failure of the called program.

    - If ``failure_handling_method`` is ``fail``, the command fails (with the same effect as executing ``return=failed``).
    - If ``failure_handling_method`` is ``repeat``, the command is repeated.
    - If ``failure_handling_method`` is ``ignore``, the failure is ignored (the program is continued).

``failure_handling_directive``
    If omitted, the value ``Ignore`` is used for ``failure_handling_method``.

worker
------
Calls a program of the productionsite's main worker.

Parameter syntax::

    parameters ::= program

Parameter semantics:

``program``
    The name of a program defined in the productionsite's main worker.

sleep
-----
Does nothing.

Parameter syntax::

  parameters ::= duration

Parameter semantics:

``duration``
    A natural integer. If 0, the result from the most recent command that
    returned a value is used.

Blocks the execution of the program for the specified duration.

animate
-------
Runs an animation.

Parameter syntax::

  parameters ::= animation duration

Parameter semantics:

``animation``
    The name of an animation (defined in the productionsite).
``duration``
    A natural integer. If 0, the result from the most recent command that
    returned a value is used.

Starts the specified animation for the productionsite. Blocks the execution of the program for the specified duration. (The duration does not have to equal the length of the animation. It will loop around. The animation will not be stopped by this command. It will run until another animation is started.)

consume
-------
Consumes wares from the input storages.

Parameter syntax::

  parameters ::= group {group}
  group      ::= ware_type{,ware_type}[:count]

Parameter semantics:

``ware_type``
    The name of a ware type (defined in the tribe).
``count``
    A positive integer. If omitted, the value 1 is used.

For each group, the number of wares specified in count is consumed. The consumed wares may be of any type in the group.

If there are not enough wares in the input storages, the command fails (with the same effect as executing ``return=failed``). Then no wares will be consumed.

Selecting which ware types to consume for a group so that the whole command succeeds is a constraint satisfaction problem. The implementation does not implement an exhaustive search for a solution to it. It is just a greedy algorithm which gives up instead of backtracking. Therefore the command may fail even if there is a solution.

However it may be possible to help the algorithm by ordering the groups carefully. Suppose that the input storage has the wares ``a:1, b:1`` and a consume command has the parameters ``a,b:1 a:1``. The algorithm tries to consume its input wares in order. It starts with the first group and consumes 1 ware of type ``a`` (the group becomes satisfied). Then it proceeds with the second group, but there are no wares of type ``a`` left to consume. Since there is no other ware type that can satisfy the group, the command will fail. If the groups are reordered so that the parameters become ``a:1 a,b:1``, it will work. The algorithm will consume 1 ware of type ``a`` for the first group. When it proceeds with the second group, it will not have any wares of type ``a`` left. Then it will go on and consume 1 ware of type ``b`` for the second group (which becomes satisfied) and the command succeeds.

.. note:: It is not possible to reorder ware types within a group. ``a,b`` is equivalent to ``b,a`` because in the internal representation the ware types of a group are sorted.

produce
-------
Produces wares.

Parameter syntax::

  parameters ::= group {group}
  group      ::= ware_type[:count]

Parameter semantics:

``ware_type``
    The name of a ware type (defined in the tribe). A ware type may only
    appear once in the command.
``count``
    A positive integer. If omitted, the value 1 is used.

For each group, the number of wares specified in count is produced. The produced wares are of the type specified in the group. How the produced wares are handled is defined by the productionsite.

mine
----
Takes resources from the ground. It takes as arguments first the resource
name, after this the radius for searching for the resource around the building
field. The next values is the percentage of starting resources that can be dug
out before this mine is exhausted. The next value is the percentage that this
building still produces something even if it is exhausted. And the last value
is the percentage chance that a worker is gaining experience on failure - this
is to guarantee that you can eventually extend a mine, even though it was
exhausted for a while already.

check_soldier
-------------
Returns failure unless there are a specified amount of soldiers with specified level of specified properties. This command type is subject to change.

train
-----
Increases the level of a specified property of a soldier. No further documentation available.

play_sound
----------
Plays a soundFX.

Parameter syntax::

  parameters ::= soundFX [priority]

Parameter semantics:

``soundFX``
    The filename of a soundFX (relative to the productionsite's directory).
``priority``
    An integer. If omitted, 127 is used.

Plays the specified soundFX with the specified priority. Whether the soundFX is actually played is determined by the sound handler.
