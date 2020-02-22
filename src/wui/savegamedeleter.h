#ifndef WL_WUI_SAVEGAMEDELETER_H
#define WL_WUI_SAVEGAMEDELETER_H

#include "gamedetails.h"
#include <stdint.h>
#include <string>
#include <vector>
class SavegameDeleter {
public:
	SavegameDeleter();

	uint32_t try_to_delete(const std::vector<SavegameData>& to_be_deleted) const;
};

#endif  // WL_WUI_SAVEGAMEDELETER_H
