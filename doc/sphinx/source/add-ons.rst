Add-Ons
=======

The Widelands add-on system is still under development. In particular, Widelands does not yet have a server where add-ons can be up- and downloaded.
This document describes how to create your own add-ons.

Structure
---------

An add-on is either a directory or a zip-compressed file with the extension ``*.wad``. Add-ons have to be placed in the ``addons`` subdirectory of the home directory to be found by the game.

The ``addons`` File
-------------------

An add-on contains a plain-text ini-style file called ``addons`` with the following entries in the ``global`` section:

* ``name``: The untranslated name of the add-on
* ``i18n_name``: Identical to ``name`` but marked for translation with '_'
* ``description``: The untranslated long description
* ``i18n_description``: Identical to ``description`` but marked for translation with '_'
* ``author``: The add-on's author name
* ``version``: The version number (1 for new add-ons)
* ``category``: One of "tribes", "world", "script", "maps", "campaign", "win_condition", "starting_condition"
* ``requires``: A comma-separated list of the filenames of add-ons required by this add-on. Currently requirements are not yet implemented and this value is ignored.

Example:

.. code-block:: ini

   [global]
   name="Fishy"
   i18n_name=_"Fishy"
   description="Adds the highest amount of fish to every map node that can hold fish."
   i18n_description=_"Adds the highest amount of fish to every map node that can hold fish."
   author="Nordfriese"
   version="1"
   category="script"
   requires=

Note: Only ``name`` and ``description`` can be marked for translation.

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


tribes
~~~~~~
A script that modifies tribe units, such as wares, workers, or buildings. You can modify existing units and create new ones.

The add-on needs to contain a script called ``init.lua`` which will be run after the default tribes were loaded.
For details compare to the official tribe loading scripts in ``data/tribes/**/init.lua``.

If the add-on introduces one or more new tribes, it will additionally need to contain a scipt called ``preload.lua``.
For details compare to the official tribe preload script ``data/tribes/preload.lua``.

world
~~~~~
A script that modifies world units, such as terrains and resources. You can modify existing units and create new ones.

The add-on needs to contain a script called ``init.lua`` which will be run after the default world was loaded.
For details compare to the official world loading scripts in ``data/world/**/init.lua``.


script
~~~~~~
A script that will be run when the game starts.

The add-on needs to contain a script called ``init.lua`` which will be used as the main script, like for scenarios.


maps
~~~~
A set of maps and/or standalone scenarios.

All valid map files (including those in subdirectories) contained in the add-on will be offered in map selection screens.

Note that it is not possible to mark the names of subdirectories for translation.


campaign
~~~~~~~~
One or more complete campaigns.

The add-on needs to contain a script called ``campaigns.lua`` containing the campaign definition.
For details compare to the official campaign script ``data/campaigns/campaigns.lua``.

The campaign scenarios is by default assumed to be located in the official ``data/campaigns`` directory. To specify that it is located in an add-on, prefix the name with the add-on's internal name followed by a colon (e.g. "example-campaign.wad:example.wmf").


win_condition
~~~~~~~~~~~~~
A win condition script.

The add-on needs to contain a script called ``init.lua`` which must follow the same conventions as the files in ``data/scripting/win_conditions/*.lua`` with one exception: The win condition table should specify the optional ``textdomain`` field which should be equal to the add-ons internal name (see `Translating`_).


starting_condition
~~~~~~~~~~~~~~~~~~
A starting condition script. May define the same starting conditions for any number of tribes.

The add-on needs to contain one or more scripts called ``<tribename>.lua``
which must follow the same conventions as the files in ``data/tribes/scripting/starting_conditions/*/*.lua``.


Restrictions
------------

The order of add-ons matters. Add-ons can be reordered in the in-game add-ons manager. Enabled add-ons will be executed from top to bottom. If you enable one add-on A that adds a new worker type that requires experience and another add-on B that modifies all workers' experience thresholds, the new worker's experience will be modified by B if and only if B is loaded later than A.

In the editor, world (but not tribes) add-ons will be run, allowing you to create maps with new worlds. The information which add-ons a map was created with is stored in the map file. When opening a map in the editor or starting a new game, the world add-ons required by the map will be enabled and all other world add-ons disabled. Therefore map designers need to choose the add-ons they want to use prior to launching the editor; the choice can not be modified later. Their choice of world add-ons will also be enforced whenever someone starts a game on that map. Script add-ons are ignored by the editor. Tribes add-ons are also ignored; therefore it is not possible to recommend an add-on-defined tribe as the default tribe for a player.

When loading a game, the game will activate the tribes- and world add-ons the game was originally started with, and disable all others. Script add-ons are ignored on loading. Starting and win conditions as well as maps and campaign/scenario scripts are stored in the savegame independently from the add-on that defines them.

In multiplayer games, all players need to enable the same add-ons at the same version in the same order for the game to work without desyncs. No checks for this are implemented so far.


Upgrading
---------

Add-ons newly uploaded to the server (when we have one…) will have to define their version number as 1. It will be possible for users to upload new versions of their add-ons to the server, where the new version number needs to be 1 greater than the version present on the server. The version previously stored on the server will be replaced with the new one. The in-game add-ons manager allows users to upgrade installed add-ons with one click when a new version is available.

Add-on upgrades may break compatibility between versions; therefore, savegames and replays warn when the game was started with a different add-on version than the installed one. In such cases, the game will often still work correctly, but this can not be guaranteed.


Verification
------------

Add-ons can potentially contain harmful or offensive content. The Widelands development team will attempt to moderate add-ons uploaded to the server: Add-ons containing malicious content will be deleted, the other add-ons will be marked as "verified". The in-game add-ons manager displays an indicator next to each add-on whether it was verified by the developers yet. By default, only verified add-ons are displayed; users can change this behaviour in the add-on manager's Filter tab.


Translating
-----------

In order to not have to release a new version whenever translations change, translation files will be provided by the server independently from the add-ons. There will be a project "Widelands Add-Ons" on Transifex which will contain one resource for every add-on present on the server. The Transifex catalogue for each add-on will be updated automatically whenever a new version is uploaded to the server.

The textdomain for an add-on is called ``internal-addon-name.wad``. The strings in the add-on config file, as well as map elemental data for Map Set add-ons, will be fetched from this textdomain. All Lua scripts shipped with the add-on will need to explicitly set the said textdomain. NOTE that you need to use the special function ``set_addon_textdomain("internal-addon-name.wad")`` to ensure that the textdomain will be looked for among the add-ons-specific translation files rather than in the locale directory shipped with the official game.

The server will keep a repository of all add-on MO files which will be automatically compiled from the latest Transifex translations weekly. Downloading or upgrading an add-on will automatically download and install the latest translations files for this add-on for all languages. Each add-on has a translations version number in addition to the add-on version number; this allows the game to figure out whether the translations for an installed add-on can be upgraded.


License
-------

Unless a license is explictly specified, all add-ons are released under the same license as Widelands itself, that is, the GNU General Public License (GPL) v2. An add-on may specify a different license by including a file called LICENSE and mentioning the license in the description text.
