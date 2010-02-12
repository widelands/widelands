Scripting Tutorial
==================

This section describes how to get a map ready for scripting addition and how
to write simple Lua scripts. It goes trough all the concepts required to get
you started writing cool widelands scenarios and campaigns. 

Designing the Map
-----------------

Widelands Map Files are plain directories containing various different files
that describe a map. Normally, widelands saves those files in one zip archive
file with the file extension ``.wmf``. To add scripting capabilities to maps,
we have to create new text files in the ``scripting/`` directory of the map,
therefore we want to have the map as a plain directory. There are two ways to
achieve this:

   1. Set the nozip option in the advanced options inside of Widelands.
      Widelands will now save maps (and savegames) as plain directories.
   2. Manually unpack the zip file. To do this, do the following:

      1. Rename the file: ``map.wmf``-> ``map.zip``
      2. Unpack this zipfile, a folder ``map.wmf`` will be created.

Now create a new directory called ``scripting/`` inside the map folder.

Hello World
-----------

The language widelands is using for scripting is called `Lua`_. It is a simple
scripting language that is reasonable fast and easy to embed in host
applications.  We will now learn how to start the map as a scenario and how to
add a simple Lua script to it.  For this create a new text file and write the
following inside:

.. code-block:: lua
   
   print "###############################"
   print "Hello World"
   print "###############################"

Save this file inside the maps directory as ``scripting/init.lua``.

.. note::

   When we talk about text files, we mean the very basic representation of
   characters that a computer can understand. That means that any fancy word
   (Word, OpenOffice and there like) processor will likely produce the wrong
   file format when saving. Make sure to use a plain text editor (like Notepad
   under Windows, nedit under Linux and TextEdit under Mac OS X). If you have
   programmed before, you likely already have found your favorite editor....

Now we try to start this scenario. We can either directly select the map
and mark the scenario box to start it as a scenario or we manually
start widelands to open it directly from the console. This is very convenient
for testing cycles: Open up a command line (Terminal under Mac OS X/Linux, Cmd
under Windows) and ``cd`` into the widelands directory.  Now start up
widelands and add the scenario switch:

.. code-block:: sh

   ./widelands --scenario=path/to/map/mapname.wmf

Widelands should start up and immediately load a map. Look at it's output on
the cmdline (Under Windows, you might have to look at ``stdout.txt``) and you
should see our text been printed::

   ###############################"
   Hello World"
   ###############################"

So what we learned is that widelands will run the script
``scripting/init.lua`` as soon as the map is finished loading. This script is
the entry point for all scripting inside of widelands. 

.. _`Lua`: http://www.lua.org/ 

A Lua Primer
------------

This section is intentionally cut short. There are excellent tutorials in
`Luas Wiki`_ and there is a complete book free online: `Programming in Lua`_.
You should definitively start there to learn Lua. 

This section only contains the parts of Lua that I found bewildering and it
also defines a few conventions that are used in this documentation. 

.. _`Luas Wiki`: http://lua-users.org/wiki/TutorialDirectory
.. _`Programming in Lua`: http://www.lua.org/pil/


Data Types
^^^^^^^^^^

Lua only has one Fundamental data type called Table. A Table is what Python
calls a dictionary and other languages call a Hashmap. It basically contains a
set of Key-Value combinations. There are two ways to access the values in the
array, either by using the ``d[key]`` synatx or by using the ``d.key`` syntax. For the 
later, ``key`` must be a string:

.. code-block:: lua

   d = { 
      value_a = 23,
      b  = 34,
      90 = "Hallo"
   }

   d.value_a -- is 23
   d['value_a'] -- the same
   d['b'] -- the same as d.b

   d[90]  -- is "Hallo"

   b.90 -- this is illegal


Calling conventions
^^^^^^^^^^^^^^^^^^^

Calling a function is Lua is straight forward, the only thing that comes as a
surprise for most programmers is that Lua throws values away without notice.

.. code-block:: lua
   
   function f(a1, a2, a3) print("Hello World:", a1, a2, a3) end

   f() --- Prints 'Hello World: nil  nil  nil'
   f("a", "house", "blah") --- Prints 'Hello World: a  house  blah'

   f("a", "a", "a", "a", "a") --- Prints 'Hello World: a  a  a'

The same also goes for return values.

.. code-block:: lua

   function f() return 1, 2, 3 end

   a = f()  -- a == 1
   a,b = f() -- a == 1, b == 2
   a,b,c,d = f() -- a == 1, b == 2, c == 3, d == nil


Coroutines
^^^^^^^^^^

TODO


.. vim:ft=rst:spelllang=en:spell
