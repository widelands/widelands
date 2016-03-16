set_textdomain("texts")

include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

return {
   title = _"General Help",
   text =
      title(_"General Help") ..
      rt(
         help_introduction() ..

         h2(_"Window Control") ..
         p(
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Right-click:", _"Close window") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Middle-click or Ctrl + Left-click:", _"Minimize/Maximize window") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Ctrl + Left-click on Button:", _"Skip confirmation dialog")) ..

         h2(_"Road Control") ..
         p(
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Ctrl + Left-click:", _"While connecting two flags: Place flags automatically") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Ctrl + Left-click:", _"While removing a flag: Remove all flags up the the first junction")) ..

         h2(_"Keyboard Shortcuts") ..
            p(
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Page Up:", _"Increase game speed") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Page Down:", _"Decrease game speed") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Pause:", _"Pause the game") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Space:", _"Toggle building spaces") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"M:", _"Toggle minimap") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"N:", _"Toggle messages (‘news’)") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"C:", _"Toggle census") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"S:", _"Toggle statistics") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"I:", _"Toggle stock inventory") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"O:", _"Toggle objectives") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"B:", _"Toggle building statistics") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"F:", _"Toggle fullscreen (if supported by the OS)") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Home:", _"Center main mapview on starting location") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"(Ctrl +) 0-9:", _"Remember and go to previously remembered locations") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_",:", _"Go to the previous location") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_".::", _"Go to the next location") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"F6:", _"Show the debug console (only in debug-builds)") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Ctrl + F10::", _"Quit the game immediately") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Ctrl + F11:", _"Take a screenshot")) ..

         h3(_"In the message window, the following additional shortcuts are available:") ..
         p(
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"0:", _"Show all messages") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"1:", _"Show geologists’ messages only") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"2:", _"Show economy messages only") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"3:", _"Show seafaring messages only") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"4:", _"Show warfare messages only") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"5:", _"Show scenario messages only") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"G:", _"Jump to the location corresponding to the current message") ..
               -- TRANSLATORS: This is an access key combination. Do not change the key.
               dl(_"Delete:", _"Archive/Restore the current message")) ..

         h2(_"Tribal Encyclopedia") ..
         -- TRANSLATORS: %s is a representation of the ? button.
         p(_"For a detailed description of the tribes’ economies, use the %s button on the bottom right."):bformat(b("?")) ..
         help_online_help()
      )
}
