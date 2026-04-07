#include "stdinc.h"
#include "Util.h"
#include "Text.h"
#include "HubEntry.h"
#include "DCContext.h"
#include "SettingsManager.h"

namespace dcpp {

const string& FavoriteHubEntry::getNick(bool useDefault) const {
    if (!nick.empty() || !useDefault)
        return nick;
    return ctx_.getSettingsManager()->get(SettingsManager::NICK, true);
}

} // namespace dcpp
