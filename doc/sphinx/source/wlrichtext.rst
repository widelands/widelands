.. _wlrichtext:

The Widelands Rich Text System
==============================

All texts that can be displayed to the user can be marked up to be laid out
and nicely formatted. This allows for changing of font sizes, weights, colors
and for the embedding of images. This documents how to format a string to be
recognized as rich text and which attributes are understood.

The Widelands rich text language is inspired by HTML and therefore uses a
syntax very similar to it. To tell our rendering system to go into richtext mode,
start your text with ``<rt>`` and end it with ``</rt>``.

.. toctree::
   :maxdepth: 2

   General Markup Functions <autogen_auxiliary_richtext.rst>
   Scenario Markup Functions <autogen_auxiliary_richtext_scenarios.rst>
   Richtext Tags, their Attributes and Restrictions <autogen_rt_tags.rst>


Code Example
------------

Here is a simplified version of a fancy message displayed to the user:

.. code-block:: lua

   body =
      "<rt>                                     -- This is richtext
         <p>                                    -- Start a paragraph
            <font size=18 bold=1 color=D1D1D1>  -- Set font size and color
               Big font to create a header
            </font>
         </p>
         <p>
            <font size=12>
               Normal paragraph, just with a bit more text to show how it looks like.
            </font>
         </p>
         -- The following content should be spread across a full line
         <div width=100%>
            <div>
               <p>
                  <font size=12>
                  <img src=images/wui/menus/menu_toggle_menu.png>
                  </font>
               </p>
            </div>
            <div width=*>                       -- Fill up the remaining space
               <p>
                  <font size=12>
                     Another normal paragraph
                  </font>
               </p>
            </div>
         </div>
         ...
      </rt>"

This is quite a lot of hacking, so we have :ref:`Lua convenience functions <richtext.lua>`
set up. We recommend that you always use those while scripting in Lua, which will
also give us consistency in style throughout Widelands.
Using the convenience functions will also result in cleaner code when
including translation markup (the ``_([[Some text]])`` or ``_("Some text")`` function).

A code example how to use the convenience functions and their attributes is given in the
documentation for the :ref:`richtext convenience functions <lua_formatting_example>`.

:ref:`Return to index<richtext.lua>`
