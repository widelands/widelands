#include "editor/editorinteractive.h"
#include "editor/tools/tool_conf.h"

ToolConf::ToolConf()
  : toolId(static_cast<ToolID>(0)),
    sel_radius(0),
    change_by(0),
    set_to(0),
    interval(0, 0) {
}

ToolConf::ToolConf(EditorInteractive& base, ToolID toolId)
  : toolId(toolId),
    sel_radius(base.get_sel_radius()),
    change_by(0),
    set_to(0),
    interval(0, 0) {
}
