include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

push_textdomain("texts")
tips = {
   {
      -- TRANSLATORS: %s = URL to the Widelands website
      text = (_("You can find general help for the editor as well as a short tutorial on our wiki at %s.")):bformat(u("widelands.org")),
      seconds = 6
   },
   {
      text = _("You can select multiple animal types if you hold down Ctrl during selection. If you then click on the map, an instance of one of the selected animal types will be placed."),
      seconds = 7
   },
   {
      text = _("Just like with animals, you can also select multiple immovables or ground textures with Ctrl + mouse click."),
      seconds = 6
   },
   {
      text = _("Holding down Shift switches the first alternative tool on. This tool is in most cases the complete opposite of the normal tool (for example deletion of an object instead of placing one)."),
      seconds = 7
   },
   {
      text = _("Holding down Ctrl switches the second alternative tool on (if it exists)."),
      seconds = 6
   },
   {
      text = _("The keys %1% allow you to directly set the size of the selected area around your mouse cursor."):bformat(help_editor_toolsize_tips()),
      seconds = 6
   },
   {
      text = _("%1% behaves like in normal games: It switches if building spaces are shown on or off."):bformat(wl.ui.get_shortcut("buildhelp")),
      seconds = 4
   },
   {
      text = _("When placing trees, use the help button to see which trees are more likely to survive on which terrain."),
      seconds = 6
   },
   {
      text = _("If you do not set water and fish resources, a default amount will be used."),
      seconds = 4
   },
   {
      text = _("You can place a medium building on any node where there is a slope no greater than 1 in the south-east direction and its height difference to all nodes in a radius of 2 is no greater than 2."),
      seconds = 7
   },
   {
      text = _("You can place a big building on any node where you could place a medium building if there is no obstacle or non-arable terrain towards the west, northwest and northeast."),
      seconds = 7
   },
}
pop_textdomain()
return tips
