.. _scenario_tutorial:

Scenario Tutorial
=================

This section describes how to get a map ready for scripting addition and how
to write simple Lua scripts. It goes trough all the concepts required to get
you started writing cool widelands scenarios and campaigns.

Designing the Map
-----------------

Wideland's map files are plain directories containing various different files
that describe a map. Normally, widelands saves those files in one zip archive
file with the file extension ``.wmf``. To add scripting capabilities to maps,
we have to create new text files in the ``scripting/`` directory of the map,
therefore we want to have the map as a plain directory, so that we can
easily add new scripting files. There are two ways to achieve this:

   1. In the widelands main menu open the tab *"Options → Saving"* and uncheck
      the option *"Compress widelands data files..."*. Widelands will now save maps
      (and savegames) as plain directories.

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

   print("###############################")
   print("Hello World")
   print("###############################")

Save this file inside the maps directory as ``scripting/init.lua``.

Now we try to start this scenario. We can either directly select the map when
starting a new single player game and mark the scenario box or we manually
tell widelands to open our map as a scenario directly from the console.
This is very convenient if you need to test drive the same map over and over
again: Open up a command line (Terminal under Mac OS X/Linux, Cmd under
Windows) and ``cd`` into the widelands directory.  Now start up widelands and
add the scenario switch like so:

.. code-block:: sh

   ./widelands --scenario=path/to/map/mapname.wmf

Widelands should start up and immediately load a map. Look at it's output on
the cmdline (Under Windows, you might have to look at ``stdout.txt``) and you
should see our text been printed::

   ###############################
   Hello World
   ###############################

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

Lua only has one fundamental data type called :class:`table`. A Table is what
Python calls a dictionary and other languages call a Hashmap. It basically
contains a set of Key-Value combinations. There are two ways to access the
values in the table, either by using the ``d[key]`` syntax or by using the
``d.key`` syntax.  For the later, ``key`` must be a string:

.. code-block:: lua

   d = {             -- d is a table with key=value pairs
      value_a = 23,
      b  = 34,
      90 = "Hallo"
   }

   d.value_a         -- is 23
   d['value_a']      -- the same
   d['b']            -- the same as d.b

   d[90]             -- is "Hallo"

   b.90              -- this is illegal (key is not a string)

Tables that are indexed with integers starting from 1 are called
:class:`arrays` throughout the documentation. Lua also accepts them as
something special, for example it can determine their length via the #
operator and they can be specially created:

.. code-block:: lua

   a = { [1] = "Hi", [2] = "World" }
   b = { "Hi", "World" }            -- b has the same content than a

   print(#a)                        -- will print 2, the amount of key/value pairs in a 

Calling conventions
^^^^^^^^^^^^^^^^^^^

Calling a function is Lua is straight forward, the only thing that comes as a
surprise for most programmers is that Lua throws values away without notice.

.. code-block:: lua

   function f(a1, a2, a3) print("Hello World:", a1, a2, a3) end

   f()                        -- Prints 'Hello World: nil  nil  nil'
   f("a", "house")            -- Prints 'Hello World: a  house  nil'
   f("a", "house", "blah")    -- Prints 'Hello World: a  house  blah'

   f("a", "a", "a", "a", "a") -- Prints 'Hello World: a  a  a'

The same also goes for return values.

.. code-block:: lua

   function f() return 1, 2, 3 end

   a = f()        -- a == 1
   a,b = f()      -- a == 1, b == 2
   a,b,c,d = f()  -- a == 1, b == 2, c == 3, d == nil

Lua allows to optionally leave out the parentheses of a function call in
certain situations. This is considered bad style and sometimes results in
ambiguous statements. It is recommended to always use parentheses in a 
function call.

Coroutines
^^^^^^^^^^

The most important feature of Lua that widelands is using are coroutines. We
use them watered down and very simple, but their power is enormous. In
Widelands use case, a coroutine is simply a function that can interrupt it's
execution and give control back to widelands at any point in time. After
it is awoken again by widelands, it will resume at precisely the same point
again. Let's dive into an example right away:

.. code-block:: lua

   include "scripting/coroutine.lua"

   function print_a_word(word)         -- a function we'll use to create a coroutine
      while true do
         print(word)
         sleep(1000)                   -- important call, see the note below
      end
   end

   run(print_a_word, "Hello World!")   -- constructs a new coroutine from the function print_a_word and an argument, and immediately launches this coroutine

If you put this code into our ``init.lua`` file from the earlier example, you
will see "Hello World!" begin printed every second on the console. Let's
digest this example. The first line imports the :doc:`autogen_auxiliary_coroutine`
script from the auxiliary Lua library that comes bundled with widelands. We use two
functions from this in the rest of the code, namely :func:`sleep` and
:func:`run`.

Then we define a simple function :func:`print_a_word` that takes one argument
and enters an infinite loop: it prints the argument, then sleeps for a second.
The :func:`sleep` function puts the coroutine to sleep and tells widelands to
wake the coroutine up again after 1000 ms have passed. The coroutine will then
continue its execution directly after the sleep call, that is it will enter
the loop's body again.

All we need now is to get this function started and this is done via the
:func:`run` function: it takes as first argument a function and then any
number of arguments that will be passed on to the given function. The
:func:`run` will construct a coroutine and hand it over to widelands for
periodic execution.

These are all of the essential tools we need to write cool scenario scripts
for widelands.

.. note::

   Keep in mind that widelands won't do anything else while the coroutine is
   running, that's why the call to :func:`sleep` is very important.
   If a call to :func:`sleep` is missing, widelands will hang – or even the
   whole operating system may stall for an extended period of time - until
   Widelands is force-closed.

   If you plan to do long running tasks always add some calls to 
   :func:`sleep` here and there so that widelands can act and update the user
   interface.

Let's consider a final example on how coroutines can interact with each other.

.. code-block:: lua

   include "scripting/coroutine.lua"

   function print_a()
      while 1 do
         print(a)
         sleep(1000)
      end
   end

   function change_a()
      while true do
         if a == "Hello" then
            a = "World"
         else
            a = "Hello"
         end
         sleep(1333)
      end
   end

   a = "Hello"                -- global variable
   run(print_a)
   run(change_a)

The first coroutine will print out the current value of a, the second changes
the value of the variable ``a`` asynchronously. So we see in this example that
coroutines share the same environment and can therefore use global variables
to communicate with each other.

Scope of Variables
^^^^^^^^^^^^^^^^^^

In the last example the used variable named ``a`` is in the global scope.
`Global scope` means that this variable can be accessed (and changed) in all
functions and files the scenario uses. This can lead to bad errors if in one
part of the scenario the value of the variable get changed while in other parts
of the scenario the value of the variable get calculated. E.g. the example given
above will overwrite the global function :meth:`a` and further calls to
:meth:`a` will not work as expected anymore. To prevent such bad errors it is
recommended to:

1. give global variables a descriptive (possibly unique) name, e.g.::

      player_1 = wl.Game().players[1]

2. use always the keyword ``local`` for variables used in functions and files,
   e.g.::

      local a = "Hello"          -- the scope of this 'a' is the file where it is defined; global 'a' is not changed
      local function change_a()  -- the scope of this function is the file; it can't be called from outside the file
         local a = "World"       -- the scope of this 'a' is the function, both local 'a' and global 'a' are not changed
         print(a)                -- will print "World"
      end
      print(a)                   -- will print "Hello"

For a step by step tutorial for scenarios take a look at the
`Scenario Tutorial <https://www.widelands.org/wiki/Scenario%20Tutorial/>`_ in
our wiki.

Preparing Strings for Translation
---------------------------------

If you want your scenario to be translatable into different languages, it is
important to keep in mind that languages differ widely in their grammar. This
entails that word forms and word order will change, and some languages have
more than one plural form. So, here are some pointers for good string design.
For examples for the formatting discussed here, have a look at 
``data/maps/MP Scenarios/Island Hopping.wmf/scripting/multiplayer_init.lua`` in
the source code.

Marking a String for Translation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Use the function :meth:`_` to mark a string for translation, e.g.

.. code-block:: lua

   print(_("Translate me"))

Strings that contain number variables have to be treated differently; cf.
the `Numbers in Placeholders`_ section below.

Translator Comments
^^^^^^^^^^^^^^^^^^^

If you have a string where you feel that translators will need a bit of help
to understand what it does, you can add a translator comment to it. Translator
comments are particularly useful when you are working with placeholders,
because you can tell the translator what the placeholder will be replaced
with. Translator comments need to be inserted into the code in the line
directly above the translation. Each line of a translator comment has to be
prefixed by ``-- TRANSLATORS:``, like this:

.. code-block:: lua

   -- TRANSLATORS: This is just a test string
   -- TRANSLATORS: With a multiline comment
   print(_("Hello Word"))


Working with Placeholders
^^^^^^^^^^^^^^^^^^^^^^^^^

If you have multiple variables in your script that you wish to include
dynamically in the same string, please use ordered placeholders to give
translators control over the word order. We have implemented a special Lua
function for this called :meth:`string.bformat` that works just like the
``boost::format`` function in C++. Example:

.. code-block:: lua

   local world = _("world")                              -- Will print in Gaelic: "saoghal"
   local hello = _("hello")                              -- Will print in Gaelic: "halò"
   -- TRANSLATORS: %1$s = hello, %2$s = world
   print  (_("The %1$s is '%2$s'")):bformat(hello, world) -- Will print in Gaelic: "Is 'halò' an saoghal"


Numbers in Placeholders
^^^^^^^^^^^^^^^^^^^^^^^

Not all languages' number systems work the same as in English. For example, the
Gaelic word for "cat" conveniently is "cat", and this is how its plural works:
`0 cat`, `1 or 2 chat`, `3 cait`, `11 or 12 chat`, `13 cait`, `20 cat`...
So, instead of using ``_`` to fetch the translation, any string containing a
placeholder that is a number should be fetched with :meth:`ngettext` instead.
First, you fetch the correct plural form, using the number variable and ``ngettext``:

.. code-block:: lua

   pretty_plurals_string = ngettext("There is %s world" , "There are %s worlds", number_of_worlds)


Then you still need to format the string with your variable:

.. code-block:: lua

   print pretty_plurals_string:bformat(number_of_worlds)

If you have a string with multiple numbers in it that would trigger plural
forms, split it into separate strings that you can fetch with ``ngettext``.
You can then combine them with ``bformat`` and ordered placeholders.


Handling Long Strings
^^^^^^^^^^^^^^^^^^^^^

If you have a really long string, e.g. a dialog stretching over multiple
sentences, check if there is a logical place where you could split this into
two separate strings for translators. We don't have a "break after x characters"
rule for this; please use common sense here. It is easier for translators to
translate smaller chunks, and if you should have to change the string later on,
e.g. to fix a typo, you will break less translations. The strings will be put
into the translation files in the same order as they appear in the source code,
so the context will remain intact for the translators.

Note that simply concatenating two translatable strings with ``..``
does not work in all languages. To achieve the desired effect, use
the function :meth:`join_sentences` with the two sentences as
arguments, or :meth:`paragraphdivider` if you wish the text to
continue on a new paragraph.

Never assemble a single localized sentence with the ``..``
operator. Use :meth:`string.bformat` with appropriate
placeholder substitutions for such cases.

Also, please hide all formatting control characters from our translators. This
includes :ref:`richtext tags <richtext.lua>` as well as new lines in the code!
For an example, have a look at ``data/campaigns/atl01.wmf/scripting/texts.lua``.


.. vim:ft=rst:spelllang=en:spell
