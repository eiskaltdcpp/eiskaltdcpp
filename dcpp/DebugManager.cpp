/*
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"
#include "DebugManager.h"

std::string dcpp::DebugManager::typeDirToString(int typeDir) {
    switch (typeDir) {
    case HUB_IN: return "HUB_IN";
    case HUB_OUT: return "HUB_OUT";
    case CLIENT_IN: return "CLIENT_IN";
    case CLIENT_OUT: return "CLIENT_OUT";
    case DHT_IN: return "DHT_IN";
    case DHT_OUT: return "DHT_OUT";
    default: return "Unknown";
    }
}
