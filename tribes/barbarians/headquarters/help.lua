use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text = rt(p("font-weight=bold font-size=14 font-color=ff7700",_"Please note that the in-game help is incomplete and under active development.")) ..
		rt("image=tribes/barbarians/constructionsite/idle_with_worker_00.png",p(_"The help text of this building is under construction."))
}