Add-Ons
=======

This document describes how to create and package your own add-ons.

Structure
---------

An add-on is a directory (never a zip-compressed file!) with the extension ``*.wad``. Add-ons have to be placed in the ``addons`` subdirectory of the Widelands home directory to be found by the game.

The name of an add-on’s directory (including the extension, but without path) is called the add-on’s **internal name** (e.g. ``"fishy.wad"``). Internal names must be unique; that is, no two add-ons may use the same internal name. It is strongly recommended to use only the lowercase characters a-z, the digits 0-9, and ``-_.`` in the internal name.

The ``addons`` File
-------------------

An add-on contains a plain-text ini-style file called ``addons`` with the following entries in the ``global`` section:

* ``name``: The name of the add-on
* ``description``: The long description
* ``author``: The add-on’s author(s) name(s)
* ``version``: The add-on’s version number
* ``category``: One of "tribes", "world", "script", "maps", "map_generator", "campaign", "win_condition", "starting_condition", "ui_plugin", "theme"
* ``requires``: A comma-separated list of the filenames of add-ons required by this add-on.
* ``min_wl_version`` *Optional*: The oldest Widelands version required by this add-on.
* ``max_wl_version`` *Optional*: The newest Widelands version supported by this add-on.
* ``sync_safe`` *Optional*: A boolean indicating that this add-on will not desync in replays and network games. This should not be set by the add-on author.

The entries for ``name``, ``description`` and ``author`` can optionally be marked for translation with '_'. They may also contain richtext tags.

Example:

.. code-block:: ini

   [global]
   name=_("Fishy")
   description=_("Adds the highest amount of fish to every map node that can hold fish.")
   author="Nordfriese"
   version="1.0.0"
   category="script"
   requires=

.. highlight:: default

The add-on directory may additionally contain an optional PNG file ``icon.png`` which will be shown in the add-ons manager. This image should be square and not larger than 64×64 pixels. If it is not present, the add-on’s category’s representative icon will be used instead.

Categories
----------
- `tribes`_
- `world`_
- `script`_
- `maps`_
- `map_generator`_
- `campaign`_
- `win_condition`_
- `starting_condition`_
- `ui_plugin`_
- `theme`_


tribes
~~~~~~
A set of scripts that modify tribe units, namely wares, workers, buildings, immovables, ships, or tribes. You can modify existing units and create new ones.

New units are defined by creating one or more ``register.lua`` files with matching ``init.lua`` files, just like for official units. Modifications to existing units may be performed in the optional ``preload.lua`` and ``postload.lua`` files. It is recommended to use the function ``Descriptions.modify_unit`` in the ``postload.lua`` for this purpose. You also need to use this function to add new units to existing tribes if desired.

Every description type declared in a ``register.lua`` file by an add-on must define either the ``__skip_if_exists`` or the ``__replace_if_exists`` attribute, which defines how to proceed if a description of the same name was previously declared by another add-on.

If the add-on introduces one or more new tribes, it additionally needs to contain a directory called ``tribes`` with one subdirectory for each new tribe. Each subdirectory must contain ``init.lua`` and ``units.lua`` files; for details see the corresponding files in ``data/tribes/initialization/*/*.lua``. A tribe can optionally have a gametips file called ``tips.lua`` which must be located in the same directory as the ``init.lua`` file; for details see the corresponding files in ``data/txts/tips/*.lua``


world
~~~~~
A script that modifies world units, namely terrains, critters, immovables, or resources. You can modify existing units and create new ones.

The structure of world add-ons is identical to the structure of tribe add-ons (``register.lua`` files, ``init.lua`` files, optional ``preload.lua`` and/or ``postload.lua``; no ``tribes`` directory though).

Additionally, a world add-on has to contain a script called ``editor.lua`` which is used to expose any new types to the editor interface. For details see the official editor init script ``data/world/init.lua``. Note that this script needs to be present even if no new units are added.

The random map generator can not use add-ons yet.


script
~~~~~~
A script that is run when the game starts.

The add-on needs to contain a script called ``init.lua`` which is used as the main script, like for scenarios.


maps
~~~~
A set of maps and/or standalone scenarios.

All valid map files (including those in subdirectories) contained in the add-on are offered in map selection screens. It is recommended to place the map files in a subdirectory rather than in the top-level directory.

The add-on can optionally contain an ini-style file called "dirnames" which defines the localized displaynames of the subdirectories in the "global" section. Two subdirectories with the same name always have the same displayname, independent of their parent directories.


map_generator
~~~~~~~~~~~~~
A script that generates a pseudo-random map given user-provided input parameters.

The generator consists of an ``init.lua`` script. The script will be called after the following global variables have been set by the map generation frontend:

- **kClimate**: The climate for the map (one of ``"summer"``, ``"winter"``, ``"wasteland"``, ``"desert"``).
- **kResourceAmount**: Desired amount of resources on the map, in range 0 (low) to 2 (high).
- **kWater**: Desired amount of water on the map, in range 0..100.
- **kLand**: Desired amount of arable land on the map, in range 0..100.
- **kWasteland**: Desired amount of wasteland on the map, in range 0..100.
- **kIslandMode**: A boolean value indicating whether island mode is enabled.
- **kRandomNumber**: A random number to be used for randomness.
- **kMapID**: The map's ID computed from the other parameters.

The generator will be provided with a flat and empty world in which only the map size and the number of players has been set already.
All map features as well as starting positions and other map settings must be added by the generator.



Map generators are required to produce exactly the same map for the same map ID every time.


campaign
~~~~~~~~
One or more complete campaigns.

The add-on needs to contain a script called ``campaigns.lua`` containing the campaign definition. For details see the official campaign script ``data/campaigns/campaigns.lua``.

The campaign’s scenarios are by default assumed to be located in the official ``data/campaigns`` directory. To specify that a scenario is located in an add-on, prefix the name with the add-on’s internal name followed by a colon (e.g. "example-campaign.wad:example.wmf"). You can also include scenarios from other add-ons like this.


win_condition
~~~~~~~~~~~~~
A win condition script.

The add-on needs to contain a script called ``init.lua`` which must follow the same conventions as the files in ``data/scripting/win_conditions/*.lua`` with one exception: The win condition table should specify the optional ``textdomain`` field which should be equal to the add-on’s internal name (see `Internationalisation`_).


starting_condition
~~~~~~~~~~~~~~~~~~
A starting condition script. May define the same starting conditions for any number of tribes.

The add-on needs to contain one or more scripts called ``<tribename>.lua`` which must follow the same conventions as the files in ``data/tribes/initialization/<tribename>/starting_conditions/*.lua`` (see :doc:`autogen_lua_tribes_defining_discovery`).


ui_plugin
~~~~~~~~~
A plugin for the graphical user interface. The add-on needs to contain a script called ``init.lua`` which will be run once upon creating the interactive game base or the editor. The script can then modify the user interface using the :doc:`UI scripting API <autogen_wl_ui>`.


theme
~~~~~
A UI theme. See :doc:`themes` for details.

Keyboard Shortcuts
------------------

Since Widelands version 1.3, add-ons can define custom keyboard shortcuts. To do so, the add-on must create a script called ``shortcuts.lua`` in the add-on directory, which returns an :class:`array` of shortcut definition tables.

Each shortcut definition :class:`table` contains the following keys:

* ``internal_name``: The internal name of the shortcut, for use in scripts. If a shortcut with this name already exists, it will be replaced.
* ``descname``: The human-readable, localized name of the shortcut.
* ``scopes``: An :class:`array` of the scopes in which the shortcut is valid. This is used to detect clashes with other shortcuts. Valid scopes are ``"game"``, ``"editor"``, ``"main_menu"``, and ``"global"``. Each shortcut definition must have at least one scope.
* ``keycode``: The default keyboard shortcut keycode name.
* ``mods`` *Optional*: An :class:`array` of the modifiers keys in the default keyboard shortcut. Valid modifiers are ``"control"`` (also known as ``"ctrl"``), ``"shift"``, ``"alt"``, and ``"gui"`` (also known as ``"super"``, ``"meta"``, ``"cmd"``, ``"command"``, or ``"windows"``).


Restrictions
------------

The order of add-ons matters. Add-ons can be reordered in the in-game add-ons manager. Enabled add-ons are executed from top to bottom. If, for example, you enable one add-on A that adds a new worker type that requires experience and *below* another add-on B that modifies all workers’ experience thresholds, the new worker’s experience is modified by B because B is loaded later than A. If B is listed *above* A, the new worker’s experience is *not* modified by B.

In the editor, world (but not tribes) add-ons are run, allowing you to create maps with new worlds. The information which add-ons a map was created with is stored in the map file. When opening a map in the editor or starting a new game, the world add-ons required by the map are enabled and all other world add-ons disabled. Therefore map designers need to choose the add-ons they want to use prior to launching the editor; the choice can not be modified later. Their choice of world add-ons is also enforced whenever someone starts a game on that map. Script add-ons are ignored by the editor. Tribes add-ons are also ignored; therefore it is not possible to recommend an add-on-defined tribe as the default tribe for a player.

When loading a game, the game activates the tribes- and world add-ons the game was originally started with, and disable all others. Script add-ons are ignored on loading. Starting and win conditions as well as maps and campaign/scenario scripts are stored in the savegame independently from the add-on that defines them.

When loading a map or savegame, Widelands displays a warning message if any required add-on is installed at the wrong version or missing altogether. In the latter case, it is not possible to load the map or savegame.

In multiplayer games, all players need to enable the same add-ons at the same version in the same order for the game to work without desyncs. The host’s choice of add-ons is therefore enforced whenever a client joins a game.


Upgrading
---------

It is possible for users to upload new versions of their add-ons to the server, where the new version number needs to be greater than the version present on the server. The version previously stored on the server is replaced with the new one. The in-game add-ons manager allows users to upgrade installed add-ons with one click when a new version is available.

Add-on upgrades may break compatibility between versions; therefore, savegames and replays warn when the game was started with a different add-on version than the installed one. In such cases, the game will often still work correctly, but this can not be guaranteed.


Verification
------------

Add-ons can potentially contain harmful or offensive content. The Widelands development team moderates add-ons uploaded to the server: Add-ons containing malicious content will be deleted, the other add-ons will be marked as "verified". The moderators also decide which add-ons may be marked as ``sync_safe`` and assign every add-on a code quality rating. The in-game add-ons manager displays an indicator next to each add-on whether it was verified by the developers yet and what quality rating it received. By default, only verified add-ons with Good or better quality are displayed; users can change this behaviour in the add-ons manager’s filter settings.


Internationalisation
--------------------

For Add-On Developers
~~~~~~~~~~~~~~~~~~~~~

The name of the textdomain for an add-on is identical to the add-on’s internal name (e.g. ``fishy.wad``). The strings in the add-on config file, as well as map elemental data for Map Set add-ons, are fetched from this textdomain. All Lua scripts shipped with the add-on need to explicitly set the said textdomain. Note that you need to use ``push_textdomain("internal-addon-name.wad", true)`` to ensure that the textdomain is looked for among the add-ons-specific translation files rather than in the locale directory shipped with the official game.

Please keep the following guidelines in mind to ensure your add-on is well translatable:

- Do not concatenate sentence snippets (use placeholders instead).
- Always use ``ngettext`` when working with plural forms.
- Use translation markup wisely. All strings meant to be translated should be fetched with ``_("Translate me")`` or  ``pgettext("context", "Translate me")``. Richtext format characters and other strings not meant to be translated should not be marked for translation.
- When any strings might be unclear (e.g. sentence snippets, placeholders), please add a ``TRANSLATORS`` comment above the string.

The Widelands Development Team may occasionally contact add-on developers to inform them about any questions or feedback from the translators.

Technical Info
~~~~~~~~~~~~~~

In order to not have to release a new version whenever translations change, translation files are provided by the server independently from the add-ons. The "Widelands Add-Ons" Transifex project contains one resource for every add-on present on the server. The Transifex catalogue for each add-on is updated automatically whenever a new version is uploaded to the server.

The server keeps a repository of all add-on ``*.mo`` files which are automatically compiled from the latest Transifex translations regularly. Downloading or upgrading an add-on automatically downloads and installs the latest translations files for this add-on for all languages. Each add-on has a translations version number in addition to the add-on version number; this allows the game to determine whether the translations for an installed add-on can be upgraded.

Uploading
---------

The only supported way to upload an add-on is to use the in-game add-ons manager. Log in with your Widelands website user profile and online gaming password (i.e., the same credentials as for the metaserver), and use the Upload section in the add-ons manager’s last tab. If you previously submitted an add-on with the same name and lower version number, the new upload will be made available as an upgrade. You can upload screenshots for your add-ons in the same way.

When providing an upgrade, always ensure that your modifications are based on the version that was downloaded from the server rather than your original sources, as the maintainers may make minor maintenance modifications to the versions stored there.

Please note that up- or downloading few large files is several orders of magnitude faster than up- or downloading many small files. If your add-on contains new graphics for units, it is therefore recommended to use spritesheets instead of sprite files.

By uploading, you agree to publish your creation under the terms of the GNU General Public License (GPL) version 2 (the same license under which Widelands itself is distributed). For more information on the GPL, please refer to ‘About Widelands’ → ‘License’ in the Widelands main menu. It is forbidden to upload add-ons containing harmful or malicious content or spam. By uploading an add-on, you assert that the add-on is of your own creation or you have the add-on’s author(s) permission to submit it in their stead. The Widelands Development Team will review your add-on soon after uploading. In case they have further inquiries, they will contact you via a PM on the Widelands website; therefore please check the inbox of your online user profile page frequently.

You can only upload upgrades and screenshots for your own add-ons (unless you are a server administrator).

If you run into problems (e.g. the server refuses to accept an upload) or have advanced needs such as deletion of your add-ons or collaborating on someone else’s add-on, you can also ask in the Widelands forum under https://www.widelands.org/forum/topic/5073/.

Website Maps
------------

Website maps are also made available for download through the add-ons manager. They are not, however, packaged as add-ons, nor stored on the add-ons server, but rather passed through as raw WMF files from the website's archive.
Downloaded website maps are stored in the ‘maps/Downloaded’ directory under the Widelands home directory, just like manually downloaded maps should be.
Translations and server database integration (comments, ratings, download counter) are not yet implemented for website maps.
