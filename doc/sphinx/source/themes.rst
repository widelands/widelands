.. _themes:

Themes
======

A theme defines the appearance and style of Widelands' user interface.

A theme's directory needs to use the following directory structure:

* base_directory/
    * init.lua
    * wui/
        * button.png
        * windows/
            * background.png
            * bottom.png
            * close.png
            * left.png
            * maximize.png
            * minimize.png
            * pin.png
            * right.png
            * top.png
            * unpin.png
    * fsmenu/
        * button.png
        * windows/
            * background.png
            * bottom.png
            * close.png
            * left.png
            * maximize.png
            * minimize.png
            * pin.png
            * right.png
            * top.png
            * unpin.png
    * loadscreens/
        * logo.png
        * gametips.png
        * splash.jpg
        * editor.png
        * mainmenu/
            * image1.png
            * image2.jpg
            * ...
        * gameloading/
            * desert/
                * image1.png
                * image2.jpg
                * ...
            * summer/
                * image1.png
                * ...
            * wasteland/
                * image1.png
                * ...
            * winter/
                * image1.png
                * ...

The names of all files and directories in ``loadscreens`` as well as the names ``wui/`` and ``fsmenu``
are hardcoded, except for the names and amounts of the image files in ``loadscreens/mainmenu`` and
in the subdirectories of ``loadscreens/gameloading/``. These directories may contain any number of
images. Supported image formats are JPG (large, photographic pictures) and PNG (all other images).

Required Files
--------------

**loadscreens/logo.png**
    Will be used for the large 'Widelands' logo banner displayed in the main menu.

**loadscreens/gametips.png**
    Will be used as background for the tips shown while starting or loading a game.

**loadscreens/splash.png**
    The splash screen shown when starting Widelands.

**loadscreens/editor.png**
    The splash screen shown when starting the map editor.

**loadscreens/mainmenu/**
    Any number of images that will be used as backgrounds for the main menu. If more than one
    image is present, they will be exchanged in frequent intervals. If no images are found,
    the splashscreen will be used as background instead.

**loadscreens/gameloading/**
    Contains one subdirectory for each of the four map themes 'desert', 'summer', 'wasteland', and
    'winter'. Each of these directories may contain any number of images. Whenever a map is loaded,
    a random image from the subdirectory corresponding to the map's theme will be used as
    background image for the loading screen. If a theme contains no images, the Widelands logo
    will be used as background instead.

If any of the above images or directories is missing, a fallback image will be used.

**wui/** and **fsmenu/**
    These directories contain images for UI elements for items in the game and editor ('wui') and
    in the main menu ('fsmenu'). The filenames and -paths may be changed, though the correct paths
    need to be stated in the ``init.lua`` file. It is recommended to use the default filenames
    as stated above.

    * **button.png** is used as the background for buttons and several other UI elements. The image is tiled to fit the element's width and height.
    * **windows/close.png** is used as the icon for the Close button in a window's top-right corner.
    * **windows/pin.png** is used as the icon for the Pin button in a window's top-left corner.
    * **windows/unpin.png** is used as the icon for the Unpin button in a pinned window's top-left corner.
    * **windows/minimize.png** is used as the icon for the minimize button in a window's top-left corner.
    * **windows/maximize.png** is used as the icon for the unminimize button in a minimized window's top-left corner.
    * **windows/background.png** is used as the window background. The image is tiled to fit the window's width and height.
    * **windows/top.png**, **windows/bottom.png**, **windows/left.png**, and **windows/right.png** are used as the window's upper, lower, left, and right border respectively. The images are tiled to fit the window's length.

``init.lua``
------------

This Lua script returns a table with descriptions for all UI elements. The table needs to contain the following keys:

* **minimum_font_size** (int): Fonts will not be scaled below this size.
* **minimap_icon_frame** (RGB): The color for the frame of minimap previews in the Choose Map/Savegame screens.
* **background_focused** (RGBA): The color for the focus overlays of the focused UI element.
* **background_semi_focused** (RGBA): The color for the focus overlays of the focused UI element's parent elements.
* **windows** (table): Style definitons for `windows`_
* **buttons** (table): Style definitons for `buttons`_
* **sliders** (table): Style definitons for `sliders`_
* **tabpanels** (table): Style definitons for `tabpanels`_
* **editboxes** (table): Style definitons for one-line and multi-line `editboxes`_
* **dropdowns** (table): Style definitons for `dropdowns`_
* **scrollbars** (table): Style definitons for `scrollbars`_
* **statistics_plot** (table): Style definitons for in-game `statistics plots`_
* **building_statistics** (table): Style definitons for the in-game `building statistics`_ menu
* **progressbar** (table): Style definitons for `progress bars`_
* **tables** (table): Style definitons for `tables`_
* **wareinfo** (table): Style definitons for `ware statistics`_ windows
* **fonts** (table): Style definitons for all `fonts`_ commonly used in Widelands

RGB colors are arrays with three entries representing the red, green, and blue components (in this order).
RGBA colors are arrays with four entries representing the red, green, blue, and alpha components (in this order).
Example:

.. code-block:: lua

   minimap_icon_frame = {255, 220, 0},
   background_focused = {240, 240, 240, 200},

windows
~~~~~~~

The ``windows`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu windows respectively. Each subtable requires the following keys:

* **window_border_focused** (RGBA): The color to blend over the border of the window if it has focus.
* **window_border_unfocused** (RGBA): The color to blend over the border of the window if it does not have focus.
* **background**, **border_top**, **border_bottom**, **border_right**, **border_left**, **button_close**, **button_pin**, **button_unpin**, **button_minimize**, **button_unminimize** (strings): The paths to the corresponding icons. See above for the recommended directory structure.

buttons
~~~~~~~

The ``buttons`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu buttons respectively. Each subtable contains three sub-subtables ``primary``, ``secondary``, and `menu`` for the three button types. Each sub-subtable contains two sub-sub-subtables ``enabled`` and ``disabled`` for the button's enabled and disabled variant. Each of these contains a key ``font`` with a :ref:`font definition table<theme_fonts>` and a table ``background`` with the keys ``image``, typically pointing to the button background image as described above, and an RGB color ``color`` which will be blended with the image.

sliders
~~~~~~~

The ``sliders`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu sliders respectively. The ``fsmenu`` subtable contains a single sub-subtable ``menu``; the ``wui`` subtable contains two sub-subtables ``light`` and ``dark``. Each of these contains tables ``background`` and ``font`` like for `buttons`_.

tabpanels
~~~~~~~~~

The ``tabpanels`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu tabpanels respectively. The ``fsmenu`` subtable contains a single sub-subtable ``menu``; the ``wui`` subtable contains two sub-subtables ``light`` and ``dark``. Each of these contains tables ``image`` and ``color`` like for the backgrounds of `buttons`_.

editboxes
~~~~~~~~~

The ``editboxes`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu editboxes respectively. Each subtable contains tables ``background`` and ``font`` like for `buttons`_. The ``background`` table requires an additional argument ``margin`` of type ``int`` which will be used for the editbox's padding.

dropdowns
~~~~~~~~~

The ``dropdowns`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu dropdowns respectively, each of which contains a single sub-subtable ``menu``. Each sub-subtable contains tables ``image`` and ``color`` like for the backgrounds of `buttons`_.

scrollbars
~~~~~~~~~~

The ``scrollbars`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu scrollbars respectively, each of which contains a single sub-subtable ``menu``. Each sub-subtable contains tables ``image`` and ``color`` like for the backgrounds of `buttons`_.

statistics plots
~~~~~~~~~~~~~~~~

The ``statistics_plot`` table contains two subtables ``colors`` and ``fonts``. The ``colors`` table defines two RGB colors named ``axis_line`` and ``zero_line`` for the two axis of a statistics plot. The ``fonts`` tables needs to define :ref:`font definition tables<theme_fonts>` named ``y_min_value`` and ``y_max_value`` (for the y-axis labels) and ``x_tick`` (for the x-axis labels).

building statistics
~~~~~~~~~~~~~~~~~~~

The ``building statistics`` table contains two font definition tables ``census_font`` and ``statistics_font`` which will be used for the in-game census and statistics strings shown over buildings; a table ``colors`` with eight RGB colors ``low``, ``medium``, and ``high`` (for production sites), ``low_alt``, ``medium_alt``, and ``high_alt`` (alternative colors for situations where more contrast is required), ``construction`` (for objects under construction), and ``neutral`` (e.g. for military sites' soldier string); and a table ``statistics_window`` with an integer value ``editbox_margin`` to override the padding of editboxes and a table ``fonts`` with two :ref:`font definition tables<theme_fonts>` ``button_font`` (for buttons) and ``details_font`` (for other texts).

progress bars
~~~~~~~~~~~~~

The ``progressbar`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and loading screen progress bars respectively. Each subtable contains a :ref:`font definition table<theme_fonts>` ``font`` and a table ``background_colors`` with three RGB colors ``low``, ``medium``, and ``high`` for the three states of the progress bar.

tables
~~~~~~

The ``tables`` table contains two subtables ``wui`` and ``fsmenu`` for in-game/in-editor and main menu tables respectively. Each subtable contains three :ref:`font definition tables<theme_fonts>` ``enabled`` (for normal table entries), ``disabled`` (for greyed-out table entries), and ``hotkey`` (to display hotkeys).

ware statistics
~~~~~~~~~~~~~~~

The ``wareinfo`` table contains two subtables ``highlight`` and ``normal``  for highlighted/selected and non-highlighted items respectively. Each subtable contains a string ``icon_background_image`` with an image path; a ``colors`` table with three RGB colors ``icon_frame`` (for frames), ``icon_background`` (for the backgrounds of icons), and ``info_background`` (for the background of the info string associated with an item); and a ``fonts`` table with two :ref:`font definition tables<theme_fonts>` ``header`` (for captions) and ``info`` (for the info strings).

.. _theme_fonts:

fonts
~~~~~

A font definition table contains the following entries:

* **color** (RGB): The font's color
* **face** (string): The font's face's name
* **size** (int): The default font size

Optionally the following boolean values may be set: **bold**, **italic**, **underline**, and **shadow**.

Some of the above style tables define custom fonts. Additionally, there is a ``fonts`` table which defines fonts used in many places in Widelands:

* **chat_message**: Basic chat messages
* **chat_timestamp**: Timestamps for chat messages
* **chat_whisper**: Whispered chat messages
* **chat_playername**: The name of the player who sent a chat message
* **chat_server**: Chat messages sent by the server
* **fsmenu_intro**: Previously used in the splashscreen. Currently unused.
* **italic**: Italic texts (main menu only)
* **fsmenu_gametip**: Tips shown on the loading screens
* **fsmenu_info_panel_heading**: Headings for info texts (main menu only)
* **fsmenu_info_panel_paragraph**: Info texts (main menu only)
* **fsmenu_game_setup_headings**: Headings in the main menu's Launch Game screens
* **fsmenu_game_setup_mapname**: The name of the selected map in the Launch Game screens
* **fsmenu_game_setup_superuser**: Administrators in the internet lobby's list of users
* **fsmenu_game_setup_irc_client**: IRC clients in the internet lobby's list of users
* **fsmenu_translation_info**: Translation statistics in the main menu's options window
* **fs_window_title**: Window titles (main menu only)
* **wui_window_title**: Window titles (game and editor)
* **label_fs**: Normal labels (main menu only)
* **label_wui**: Normal labels (game and editor)
* **tooltip_fs**: Tooltips (main menu only)
* **tooltip_hotkey_fs**: Hotkey tooltips (main menu only)
* **tooltip_header_fs**: Tooltip headings (main menu only)
* **tooltip_wui**: Tooltips (game and editor)
* **tooltip_hotkey_wui**: Hotkey tooltips (game and editor)
* **tooltip_header_wui**: Tooltip headings (game and editor)
* **game_summary_title**: Heading in the end-of-game-summary screen
* **warning**: Warnings and errors
* **disabled**: Greyed-out items
* **wui_attack_box_slider_label**: The label over the in-game attack dialog's slider
* **wui_info_panel_heading**: Headings for info texts (game and editor)
* **wui_info_panel_paragraph**: Info texts (game and editor)
* **wui_message_heading**: Headings for in-game inbox messages
* **wui_message_paragraph**: Text of in-game inbox messages
* **wui_game_speed_and_coordinates**: Time, speed and coordinates strings in the game/editor info panel

