/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(SINGLETON_H)
#define SINGLETON_H

namespace dcpp {

class ISingleton{
public:
    ISingleton(){}

    virtual void release() = 0;
};

template<typename T>
class Singleton : public ISingleton {
public:
    Singleton() { }
    virtual ~Singleton() { }

    static T* getInstance() {
        dcassert(instance);
        return instance;
    }

    static void newInstance() {
        if(instance)
            delete instance;

        instance = new T();
    }

    static void deleteInstance() {
        if(instance)
            delete instance;
        instance = NULL;
    }

    virtual void release(){
        deleteInstance();
    }

protected:
        static T* instance;
private:
        Singleton(const Singleton&);
        Singleton& operator=(const Singleton&);

};

template<class T> T* Singleton<T>::instance = NULL;

} // namespace dcpp

#endif // !defined(SINGLETON_H)
