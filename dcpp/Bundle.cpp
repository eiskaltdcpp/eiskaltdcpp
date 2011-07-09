/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "Bundle.h"

#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace dcpp {

using boost::find_if;
using boost::for_each;

TTHValue Bundle::getHash() const {
        TigerHash ret;

        //for_each(entries, [&](const Entry &e) { ret.update(e.tth.data, TTHValue::BYTES); });
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            ret.update((*i).tth.data, TTHValue::BYTES);
        }

        return TTHValue(ret.finalize());
}

bool Bundle::contains(const TTHValue &tth) const {
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            if ((*i).tth == tth)
                return true;
        }
        return false;

        //return find_if(entries, [&](const Entry &e) { return e.tth == tth; }) != entries.end();
}

}
