Add-Ons
=======

The Widelands add-on system is still under development. In particular, Widelands does not yet have a server where add-ons can be up- and downloaded. This document describes how to create and package your own add-ons.

Structure
---------

An add-on is a directory (never a zip-compressed file!) with the extension ``*.wad``. Add-ons have to be placed in the ``addons`` subdirectory of the Widelands home directory to be found by the game.

The ``addons`` File
-------------------

An add-on contains a plain-text ini-style file called ``addons`` with the following entries in the ``global`` section:

* ``name``: The name of the add-on
* ``description``: The long description
* ``author``: The add-on’s author(s) name(s)
* ``version``: The version number (1 for new add-ons)
* ``category``: One of "tribes", "world", "script", "maps", "campaign", "win_condition", "starting_condition", "theme"
* ``requires``: A comma-separated list of the filenames of add-ons required by this add-on.

The entries for ``name``, ``description`` and ``author`` can optionally be marked for translation with '_'. They may also contain richtext tags.

Example:

.. code-block:: ini

   [global]
   name=_"Fishy"
   description=_"Adds the highest amount of fish to every map node that can hold fish."
   author="Nordfriese"
   version="1"
   category="script"
   requires=

.. highlight:: default

Categories
----------
- `tribes`_
- `world`_
- `script`_
- `maps`_
- `campaign`_
- `win_condition`_
- `starting_condition`_
- `theme`_


tribes
~~~~~~
A set of scripts that modify tribe units, namely wares, workers, buildings, immovables, ships, or tribes. You can modify existing units and create new ones.

New units are defined by creating one or more ``register.lua`` files with matching ``init.lua`` files, just like for official units. Modifications to existing units may be performed in the optional ``preload.lua`` and ``postload.lua`` files. It is recommended to use the function ``Descriptions.modify_unit`` in the ``postload.lua`` for this purpose. You will also need to use this function to add new units to existing tribes if desired.

If the add-on introduces one or more new tribes, it will additionally need to contain a directory called ``tribes`` with one subdirectory for each new tribe. Each subdirectory must contain ``init.lua`` and ``units.lua`` files; for details see the corresponding files in ``data/tribes/initialization/*/*.lua``. **Warning:** Custom tribes in add-ons are still an experimental feature and very unstable at the moment.


world
~~~~~
A script that modifies world units, namely terrains, critters, immovables, or resources. You can modify existing units and create new ones.

The structure of world add-ons is identical to the structure of tribe add-ons (``register.lua`` files, ``init.lua`` files, optional ``preload.lua`` and/or ``postload.lua``; no ``tribes`` directory though).

Additionally, a world add-on has to contain a script called ``editor.lua`` which will be used to expose any new types to the editor interface. For details see the official editor init script ``data/world/init.lua``. Note that this script needs to be present even if no new units are added.

The random map generator can not use add-ons yet.


script
~~~~~~
A script that will be run when the game starts.

The add-on needs to contain a script called ``init.lua`` which will be used as the main script, like for scenarios.


maps
~~~~
A set of maps and/or standalone scenarios.

All valid map files (including those in subdirectories) contained in the add-on will be offered in map selection screens.

Note that it is not possible yet to mark the names of subdirectories for translation.


campaign
~~~~~~~~
One or more complete campaigns.

The add-on needs to contain a script called ``campaigns.lua`` containing the campaign definition. For details see the official campaign script ``data/campaigns/campaigns.lua``.

The campaign's scenarios are by default assumed to be located in the official ``data/campaigns`` directory. To specify that a scenario is located in an add-on, prefix the name with the add-on’s internal name followed by a colon (e.g. "example-campaign.wad:example.wmf"). You can also include scenarios from other add-ons like this.


win_condition
~~~~~~~~~~~~~
A win condition script.

The add-on needs to contain a script called ``init.lua`` which must follow the same conventions as the files in ``data/scripting/win_conditions/*.lua`` with one exception: The win condition table should specify the optional ``textdomain`` field which should be equal to the add-on’s internal name (see `Translating`_).


starting_condition
~~~~~~~~~~~~~~~~~~
A starting condition script. May define the same starting conditions for any number of tribes.

The add-on needs to contain one or more scripts called ``<tribename>.lua`` which must follow the same conventions as the files in ``data/tribes/initialization/*/starting_conditions/*.lua``.


theme
~~~~~
A UI theme. This type of add-on is not implemented yet.


Restrictions
------------

The order of add-ons matters. Add-ons can be reordered in the in-game add-ons manager. Enabled add-ons will be executed from top to bottom. If, for example, you enable one add-on A that adds a new worker type that requires experience and another add-on B that modifies all workers’ experience thresholds, the new worker’s experience will be modified by B if and only if B is loaded later than A.

In the editor, world (but not tribes) add-ons will be run, allowing you to create maps with new worlds. The information which add-ons a map was created with is stored in the map file. When opening a map in the editor or starting a new game, the world add-ons required by the map will be enabled and all other world add-ons disabled. Therefore map designers need to choose the add-ons they want to use prior to launching the editor; the choice can not be modified later. Their choice of world add-ons will also be enforced whenever someone starts a game on that map. Script add-ons are ignored by the editor. Tribes add-ons are also ignored; therefore it is not possible to recommend an add-on-defined tribe as the default tribe for a player.

After installing an add-on that contains new terrain textures, the game needs to be restarted. Otherwise the new terrains will not be rendered correctly.

When loading a game, the game will activate the tribes- and world add-ons the game was originally started with, and disable all others. Script add-ons are ignored on loading. Starting and win conditions as well as maps and campaign/scenario scripts are stored in the savegame independently from the add-on that defines them.

In multiplayer games, all players need to enable the same add-ons at the same version in the same order for the game to work without desyncs. No checks for this are implemented yet.


Upgrading
---------

Add-ons newly uploaded to the server (when we have one…) will have to define their version number as 1. It will be possible for users to upload new versions of their add-ons to the server, where the new version number needs to be 1 greater than the version present on the server. The version previously stored on the server will be replaced with the new one. The in-game add-ons manager allows users to upgrade installed add-ons with one click when a new version is available.

Add-on upgrades may break compatibility between versions; therefore, savegames and replays warn when the game was started with a different add-on version than the installed one. In such cases, the game will often still work correctly, but this can not be guaranteed.


Verification
------------

Add-ons can potentially contain harmful or offensive content. The Widelands development team will attempt to moderate add-ons uploaded to the server: Add-ons containing malicious content will be deleted, the other add-ons will be marked as "verified". The in-game add-ons manager displays an indicator next to each add-on whether it was verified by the developers yet. By default, only verified add-ons are displayed; users can change this behaviour in the add-on manager’s Filter tab.


Translating
-----------

In order to not have to release a new version whenever translations change, translation files will be provided by the server independently from the add-ons. The "Widelands Add-Ons" Transifex project contains one resource for every add-on present on the server. The Transifex catalogue for each add-on is updated automatically whenever a new version is uploaded to the server.

The textdomain for an add-on is called ``internal-addon-name.wad``. The strings in the add-on config file, as well as map elemental data for Map Set add-ons, will be fetched from this textdomain. All Lua scripts shipped with the add-on will need to explicitly set the said textdomain. Note that you need to use ``push_textdomain("internal-addon-name.wad", true)`` to ensure that the textdomain will be looked for among the add-ons-specific translation files rather than in the locale directory shipped with the official game.

The server will keep a repository of all add-on ``*.mo`` files which will be automatically compiled from the latest Transifex translations regularly. Downloading or upgrading an add-on will automatically download and install the latest translations files for this add-on for all languages. Each add-on has a translations version number in addition to the add-on version number; this allows the game to determine whether the translations for an installed add-on can be upgraded.
