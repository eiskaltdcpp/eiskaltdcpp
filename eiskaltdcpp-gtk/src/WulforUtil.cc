/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "WulforUtil.hh"
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <dcpp/ClientManager.h>
#include <dcpp/Util.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include "settingsmanager.hh"

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#include <net/if.h>
#endif

using namespace std;
using namespace dcpp;

const string WulforUtil::ENCODING_LOCALE = _("System default");
vector<string> WulforUtil::charsets;
const string WulforUtil::magnetSignature = "magnet:?xt=urn:tree:tiger:";
GtkIconFactory* WulforUtil::iconFactory = NULL;

vector<int> WulforUtil::splitString(const string &str, const string &delimiter)
{
    string::size_type loc, len, pos = 0;
    vector<int> array;

    if (!str.empty() && !delimiter.empty())
    {
        while ((loc = str.find(delimiter, pos)) != string::npos)
        {
            len = loc - pos;
            array.push_back(Util::toInt(str.substr(pos, len)));
            pos = loc + delimiter.size();
        }
        len = str.size() - pos;
        array.push_back(Util::toInt(str.substr(pos, len)));
    }
    return array;
}

string WulforUtil::linuxSeparator(const string &ps)
{
    string str = ps;
    for (string::iterator it = str.begin(); it != str.end(); ++it)
        if ((*it) == '\\')
            (*it) = '/';
    return str;
}

string WulforUtil::windowsSeparator(const string &ps)
{
    string str = ps;
    for (string::iterator it = str.begin(); it != str.end(); ++it)
        if ((*it) == '/')
            (*it) = '\\';
    return str;
}

vector<string> WulforUtil::getLocalIPs()
{
    vector<string> addresses;

#ifdef HAVE_IFADDRS_H
    struct ifaddrs *ifap;

    if (getifaddrs(&ifap) == 0)
    {
        for (struct ifaddrs *i = ifap; i != NULL; i = i->ifa_next)
        {
            struct sockaddr *sa = i->ifa_addr;

            // If the interface is up, is not a loopback and it has an address
            if ((i->ifa_flags & IFF_UP) && !(i->ifa_flags & IFF_LOOPBACK) && sa != NULL)
            {
                void* src = NULL;
                socklen_t len;

                // IPv4 address
                if (sa->sa_family == AF_INET)
                {
                    struct sockaddr_in* sai = (struct sockaddr_in*)sa;
                    src = (void*) &(sai->sin_addr);
                    len = INET_ADDRSTRLEN;
                }
                // IPv6 address
                else if (sa->sa_family == AF_INET6)
                {
                    struct sockaddr_in6* sai6 = (struct sockaddr_in6*)sa;
                    src = (void*) &(sai6->sin6_addr);
                    len = INET6_ADDRSTRLEN;
                }

                // Convert the binary address to a string and add it to the output list
                if (src != NULL)
                {
                    char address[len];
                    inet_ntop(sa->sa_family, src, address, len);
                    addresses.push_back(address);
                }
            }
        }
        freeifaddrs(ifap);
    }
#endif

    return addresses;
}

string WulforUtil::getNicks(const string &cid)
{
    return getNicks(CID(cid));
}

string WulforUtil::getNicks(const CID& cid)
{
    return Util::toString(ClientManager::getInstance()->getNicks(cid));
}

string WulforUtil::getNicks(const UserPtr& user)
{
    return getNicks(user->getCID());
}

string WulforUtil::getHubNames(const string &cid)
{
    return getHubNames(CID(cid));
}

string WulforUtil::getHubNames(const CID& cid)
{
    StringList hubs = ClientManager::getInstance()->getHubNames(cid);
    if (hubs.empty())
        return _("Offline");
    else
        return Util::toString(hubs);
}

string WulforUtil::getHubNames(const UserPtr& user)
{
    return getHubNames(user->getCID());
}

StringList WulforUtil::getHubAddress(const CID& cid)
{
    return ClientManager::getInstance()->getHubs(cid);
}

StringList WulforUtil::getHubAddress(const UserPtr& user)
{
    return getHubAddress(user->getCID());
}

string WulforUtil::getTextFromMenu(GtkMenuItem *item)
{
    string text;
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(item));

    if (child && GTK_IS_LABEL(child))
        text = gtk_label_get_text(GTK_LABEL(child));

    return text;
}

vector<string>& WulforUtil::getCharsets()
{
    if (charsets.size() == 0)
    {
        charsets.push_back(ENCODING_LOCALE);
        //charsets.push_back(_("UTF-8 (Unicode)"));
        //charsets.push_back(_("CP1252 (Western Europe)"));
        //charsets.push_back(_("CP1250 (Central Europe)"));
        //charsets.push_back(_("ISO-8859-2 (Central Europe)"));
        //charsets.push_back(_("ISO-8859-7 (Greek)"));
        //charsets.push_back(_("ISO-8859-8 (Hebrew)"));
        //charsets.push_back(_("ISO-8859-9 (Turkish)"));
        //charsets.push_back(_("ISO-2022-JP (Japanese)"));
        //charsets.push_back(_("SJIS (Japanese)"));
        //charsets.push_back(_("CP949 (Korean)"));
        //charsets.push_back(_("KOI8-R (Cyrillic)"));
        //charsets.push_back(_("CP1251 (Cyrillic)"));
        //charsets.push_back(_("CP1256 (Arabic)"));
        //charsets.push_back(_("CP1257 (Baltic)"));
        //charsets.push_back(_("GB18030 (Chinese)"));
        //charsets.push_back(_("TIS-620 (Thai)"));
        charsets.push_back(_("UTF-8"));
        charsets.push_back(_("CP1252"));
        charsets.push_back(_("CP1250"));
        charsets.push_back(_("ISO-8859-2"));
        charsets.push_back(_("ISO-8859-7"));
        charsets.push_back(_("ISO-8859-8"));
        charsets.push_back(_("ISO-8859-9"));
        charsets.push_back(_("ISO-2022-JP"));
        charsets.push_back(_("SJIS"));
        charsets.push_back(_("CP949"));
        charsets.push_back(_("KOI8-R"));
        charsets.push_back(_("CP1251"));
        charsets.push_back(_("CP1256"));
        charsets.push_back(_("CP1257"));
        charsets.push_back(_("GB18030"));
        charsets.push_back(_("TIS-620"));
    }
    return charsets;
}

void WulforUtil::openURI(const string &uri)
{
    GError* error = NULL;
    gchar *argv[3];

#if defined(__APPLE__)
    argv[0] = (gchar *)"open";
#elif defined(_WIN32)
    argv[0] = (gchar *)"start";
#else
    argv[0] = (gchar *)"xdg-open";
#endif
    argv[1] = (gchar *)Text::fromUtf8(uri).c_str();
    argv[2] = NULL;

    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);

    if (error != NULL)
    {
        cerr << "Failed to open URI: " << error->message << endl;
        g_error_free(error);
    }
}

void WulforUtil::openURItoApp(const string &cmd)
{
    GError* error = NULL;

    g_spawn_command_line_async((gchar *)Text::fromUtf8(cmd).c_str(), &error);

    if (error != NULL)
    {
        cerr << "Failed to open application: " << error->message << endl;
        g_error_free(error);
    }
}

string WulforUtil::colorToString(const GdkColor *color)
{
    gchar strcolor[14];

    g_snprintf(strcolor, sizeof(strcolor), "#%04X%04X%04X",
        color->red, color->green, color->blue);

    return strcolor;
}

GdkPixbuf* WulforUtil::scalePixbuf(const GdkPixbuf *pixbuf, const int width, const int height, GdkInterpType type)
{
    g_return_val_if_fail(pixbuf != NULL, NULL);
    g_return_val_if_fail(width > 0, NULL);
    g_return_val_if_fail(height > 0, NULL);

    GdkPixbuf *scale = NULL;

    int Width = gdk_pixbuf_get_width(pixbuf);
    int Height = gdk_pixbuf_get_height(pixbuf);

    double w, h, k;

    w = (double) width / Width;
    h = (double) height / Height;
    k = MIN (w, h);

    if (Width > width || Height > height)

        scale = gdk_pixbuf_scale_simple(pixbuf, (int)(Width * k), (int)(Height * k), type);
    else
        scale = gdk_pixbuf_scale_simple(pixbuf, (int)(Width * k * 0.85), (int)(Height * k * 0.85), type);
    return
        scale;
}

string WulforUtil::makeMagnet(const string &name, const int64_t size, const string &tth)
{
    if (name.empty() || tth.empty())
        return string();

    // other clients can return paths with different separators, so we should catch both cases
    string::size_type i = name.find_last_of("/\\");
    string path = (i != string::npos) ? name.substr(i + 1) : name;

    return magnetSignature + tth + "&xl=" + Util::toString(size) + "&dn=" + Util::encodeURI(path);
}

bool WulforUtil::splitMagnet(const string &magnet, string &name, int64_t &size, string &tth)
{
    name = _("Unknown");
    size = 0;
    tth = _("Unknown");

    if (!isMagnet(magnet.c_str()) || magnet.size() <= magnetSignature.length())
        return FALSE;

    string::size_type nextpos = 0;

    for (string::size_type pos = magnetSignature.length(); pos < magnet.size(); pos = nextpos + 1)
    {
        nextpos = magnet.find('&', pos);
        if (nextpos == string::npos)
            nextpos = magnet.size();

        if (pos == magnetSignature.length())
            tth = magnet.substr(magnetSignature.length(), nextpos - magnetSignature.length());
        else if (magnet.compare(pos, 3, "xl=") == 0)
            size = Util::toInt64(magnet.substr(pos + 3, nextpos - pos - 3));
        else if (magnet.compare(pos, 3, "dn=") == 0)
            name = Util::encodeURI(magnet.substr(pos + 3, nextpos - pos - 3), TRUE);
    }

    return TRUE;
}

bool WulforUtil::splitMagnet(const string &magnet, string &line)
{
    string name;
    string tth;
    int64_t size;

    if (splitMagnet(magnet, name, size, tth))
        line = name + " (" + Util::formatBytes(size) + ")";
    else
        return FALSE;

    return TRUE;
}

bool WulforUtil::isMagnet(const string &text)
{
    return g_ascii_strncasecmp(text.c_str(), magnetSignature.c_str(), magnetSignature.length()) == 0;
}

bool WulforUtil::isLink(const string &text)
{
    return g_ascii_strncasecmp(text.c_str(), "http://", 7) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "https://", 8) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "www.", 4) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "ftp://", 6) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "sftp://", 7) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "irc://", 6) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "ircs://", 7) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "im:", 3) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "mailto:", 7) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "news:", 5) == 0;
}

bool WulforUtil::isHubURL(const string &text)
{
    return g_ascii_strncasecmp(text.c_str(), "dchub://", 8) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "adc://", 6) == 0 ||
        g_ascii_strncasecmp(text.c_str(), "adcs://", 7) == 0;
}

bool WulforUtil::profileIsLocked()
{
    static bool profileIsLocked = false;

    if (profileIsLocked)
        return TRUE;

    // We can't use Util::getConfigPath() since the core has not been started yet.
    // Also, Util::getConfigPath() is utf8 and we need system encoding for g_open().
    char *home = getenv("HOME");
    string configPath = home ? string(home) + "/.dc++/" : "/tmp/";
    string profileLockingFile = configPath + "profile.lck";
    int flags = O_WRONLY | O_CREAT;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    int fd = g_open(profileLockingFile.c_str(), flags, mode);
    if (fd != -1) // read error
    {
        struct flock lock;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        struct flock testlock = lock;

        if (fcntl(fd, F_GETLK, &testlock) != -1) // Locking not supported
        {
            if (fcntl(fd, F_SETLK, &lock) == -1)
                profileIsLocked = true;
        }
    }

    return profileIsLocked;
}


gboolean WulforUtil::getNextIter_gui(GtkTreeModel *model, GtkTreeIter *iter, bool children /* = TRUE */, bool parent /* = TRUE */)
{
    gboolean valid = FALSE;
    GtkTreeIter old = *iter;

    if (children && gtk_tree_model_iter_has_child(model, iter))
    {
        valid = gtk_tree_model_iter_children(model, iter, &old);
    }
    else
    {
        valid = gtk_tree_model_iter_next(model, iter);
    }

    // Try to go up one level if next failed
    if (!valid && parent)
    {
        valid = gtk_tree_model_iter_parent(model, iter, &old);
        if (valid)
            valid = gtk_tree_model_iter_next(model, iter);
    }

    return valid;
}

GtkTreeIter WulforUtil::copyRow_gui(GtkListStore *store, GtkTreeIter *fromIter, int position /* = -1 */)
{
    GtkTreeIter toIter;
    gtk_list_store_insert(store, &toIter, position);
    GtkTreeModel *m = GTK_TREE_MODEL(store);
    int count = gtk_tree_model_get_n_columns(m);

    for (int col = 0; col < count; col++)
    {
        copyValue_gui(store, fromIter, &toIter, col);
    }

    return toIter;
}

void WulforUtil::copyValue_gui(GtkListStore *store, GtkTreeIter *fromIter, GtkTreeIter *toIter, int position)
{
    GValue value = {0, };
    gtk_tree_model_get_value(GTK_TREE_MODEL(store), fromIter, position, &value);
    gtk_list_store_set_value(store, toIter, position, &value);
    g_value_unset(&value);
}

GtkTreeIter WulforUtil::copyRow_gui(GtkTreeStore *store, GtkTreeIter *fromIter, GtkTreeIter *parent /* = NULL */, int position /* = -1 */)
{
    GtkTreeIter toIter;
    gtk_tree_store_insert(store, &toIter, parent, position);
    GtkTreeModel *m = GTK_TREE_MODEL(store);
    int count = gtk_tree_model_get_n_columns(m);

    for (int col = 0; col < count; col++)
    {
        copyValue_gui(store, fromIter, &toIter, col);
    }

    return toIter;
}

void WulforUtil::copyValue_gui(GtkTreeStore *store, GtkTreeIter *fromIter, GtkTreeIter *toIter, int position)
{
    GValue value = {0, };
    gtk_tree_model_get_value(GTK_TREE_MODEL(store), fromIter, position, &value);
    gtk_tree_store_set_value(store, toIter, position, &value);
    g_value_unset(&value);
}

/*
 * Registers either the custom icons or the GTK+ icons as stock icons in
 * GtkIconFactory according to the user's preference. If the icons have
 * previously been loaded, they are removed and re-added.
 */
void WulforUtil::registerIcons()
{
    // Holds a mapping of custom icon names -> stock icon names.
    // Not all icons have stock representations.
    WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
    map<string, string> icons;
    icons["eiskaltdcpp"] = "eiskaltdcpp";
    icons["icon_msg"] = wsm->getString("icon_msg");
    icons["eiskaltdcpp-dc++"] = wsm->getString("icon-dc++");
    icons["eiskaltdcpp-dc++-fw"] = wsm->getString("icon-dc++-fw");
    icons["eiskaltdcpp-dc++-fw-op"] = wsm->getString("icon-dc++-fw-op");
    icons["eiskaltdcpp-dc++-op"] = wsm->getString("icon-dc++-op");
    icons["eiskaltdcpp-normal"] = wsm->getString("icon-normal");
    icons["eiskaltdcpp-normal-fw"] = wsm->getString("icon-normal-fw");
    icons["eiskaltdcpp-normal-fw-op"] = wsm->getString("icon-normal-fw-op");
    icons["eiskaltdcpp-normal-op"] = wsm->getString("icon-normal-op");
    icons["icon-smile"] = wsm->getString("icon-smile");
    icons["icon-download"] = wsm->getString("icon-download");
    icons["icon-favorite-hubs"] = wsm->getString("icon-favorite-hubs");
    icons["icon-favorite-users"] = wsm->getString("icon-favorite-users");
    icons["icon-finished-downloads"] = wsm->getString("icon-finished-downloads");
    icons["icon-finished-uploads"] = wsm->getString("icon-finished-uploads");
    icons["icon-hash"] = wsm->getString("icon-hash");
    icons["icon-preferences"] = wsm->getString("icon-preferences");
    icons["icon-public-hubs"] = wsm->getString("icon-public-hubs");
    icons["icon-queue"] = wsm->getString("icon-queue");
    icons["icon-search"] = wsm->getString("icon-search");
    icons["icon-search-spy"] = wsm->getString("icon-search-spy");
    icons["icon-upload"] = wsm->getString("icon-upload");
    icons["icon-quit"] = wsm->getString("icon-quit");
    icons["icon-connect"] = wsm->getString("icon-connect");
    icons["icon-file"] = wsm->getString("icon-file");
    icons["icon-directory"] = wsm->getString("icon-directory");
    icons["icon-refresh"] = wsm->getString("icon-refresh");
    icons["icon-reconnect"] = wsm->getString("icon-reconnect");
    icons["icon-openlist"] = wsm->getString("icon-openlist");
    icons["icon-own-filelist"] = wsm->getString("icon-own-filelist");
    icons["icon-magnet"] = wsm->getString("icon-magnet");
    icons["icon-search-adl"] = wsm->getString("icon-search-adl");
    icons["icon-pm-online"] = wsm->getString("icon-pm-online");
    icons["icon-pm-offline"] = wsm->getString("icon-pm-offline");
    icons["icon-hub-online"] = wsm->getString("icon-hub-online");
    icons["icon-hub-offline"] = wsm->getString("icon-hub-offline");

    if (iconFactory)
    {
        gtk_icon_factory_remove_default(iconFactory);
        iconFactory = NULL;
    }

    iconFactory = gtk_icon_factory_new();

    for (map<string, string>::const_iterator i = icons.begin(); i != icons.end(); ++i)
    {
        GtkIconSource *iconSource = gtk_icon_source_new();
        GtkIconSet *iconSet = gtk_icon_set_new();
        gtk_icon_source_set_icon_name(iconSource, i->second.c_str());
        gtk_icon_set_add_source(iconSet, iconSource);
        gtk_icon_factory_add(iconFactory, i->first.c_str(), iconSet);
        gtk_icon_source_free(iconSource);
        gtk_icon_set_unref(iconSet);
    }

    gtk_icon_factory_add_default(iconFactory);
    g_object_unref(iconFactory);
}
