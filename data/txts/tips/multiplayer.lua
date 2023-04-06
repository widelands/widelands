push_textdomain("texts")
tips = {
   {
      text = _("You can use the ’Page up’ and ‘Page down’ keys to adjust your desired speed – the real speed is then democratically set."),
      seconds = 5
   },
   {
      text = _("You can use ‘/me’ at the beginning of a chatmessage: ‘/me is tired’ will be printed as ‘-> Nickname is tired’."),
      seconds = 5
   },
   {
      text = _("Use ‘@name’ at the beginning of a chat message to send it to only one player."),
      seconds = 5
   },
   {
      text = _("If the game should crash, don’t worry! The game will be saved automatically so that you can reload it afterwards."),
      seconds = 7
   },
}
pop_textdomain()
return tips
