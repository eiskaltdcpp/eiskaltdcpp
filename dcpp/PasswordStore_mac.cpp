/*
 * Copyright (C) 2026 EiskaltDC++ developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "stdinc.h"
#include "PasswordStore.h"

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

namespace dcpp {
namespace {

const char* KEYCHAIN_SERVICE = "org.eiskaltdcpp.hub-password";

CFStringRef makeString(const std::string& value) {
    return CFStringCreateWithBytes(kCFAllocatorDefault,
                                   reinterpret_cast<const UInt8*>(value.data()),
                                   static_cast<CFIndex>(value.size()),
                                   kCFStringEncodingUTF8,
                                   false);
}

std::string accountForHub(const std::string& server, const std::string& nick) {
    return server + "|" + nick;
}

class CFReleaseGuard {
public:
    explicit CFReleaseGuard(CFTypeRef value = nullptr) : value(value) { }
    ~CFReleaseGuard() { if(value) CFRelease(value); }
    CFTypeRef get() const { return value; }
    CFMutableDictionaryRef mutableDictionary() const { return const_cast<CFMutableDictionaryRef>(static_cast<CFDictionaryRef>(value)); }
    CFTypeRef* out() { return &value; }
private:
    CFTypeRef value;
};

CFMutableDictionaryRef createQuery(const std::string& server, const std::string& nick) {
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);
    if(!query)
        return nullptr;

    CFReleaseGuard service(makeString(KEYCHAIN_SERVICE));
    CFReleaseGuard account(makeString(accountForHub(server, nick)));
    if(!service.get() || !account.get()) {
        CFRelease(query);
        return nullptr;
    }

    CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(query, kSecAttrService, service.get());
    CFDictionarySetValue(query, kSecAttrAccount, account.get());
    return query;
}

} // namespace

bool PasswordStore::isAvailable() {
    return true;
}

bool PasswordStore::getHubPassword(const std::string& server, const std::string& nick, std::string& password) {
    CFReleaseGuard query(createQuery(server, nick));
    if(!query.get())
        return false;

    CFDictionarySetValue(query.mutableDictionary(), kSecReturnData, kCFBooleanTrue);
    CFDictionarySetValue(query.mutableDictionary(), kSecMatchLimit, kSecMatchLimitOne);

    CFReleaseGuard result;
    OSStatus status = SecItemCopyMatching(static_cast<CFDictionaryRef>(query.get()), result.out());
    if(status != errSecSuccess || !result.get())
        return false;

    CFDataRef data = static_cast<CFDataRef>(result.get());
    password.assign(reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
                    static_cast<size_t>(CFDataGetLength(data)));
    return true;
}

bool PasswordStore::setHubPassword(const std::string& server, const std::string& nick, const std::string& password) {
    if(password.empty())
        return deleteHubPassword(server, nick);

    CFReleaseGuard query(createQuery(server, nick));
    if(!query.get())
        return false;

    CFReleaseGuard data(CFDataCreate(kCFAllocatorDefault,
                                     reinterpret_cast<const UInt8*>(password.data()),
                                     static_cast<CFIndex>(password.size())));
    if(!data.get())
        return false;

    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);
    if(!attrs)
        return false;
    CFReleaseGuard attrsGuard(attrs);
    CFDictionarySetValue(attrs, kSecValueData, data.get());

    OSStatus status = SecItemUpdate(static_cast<CFDictionaryRef>(query.get()), attrs);
    if(status == errSecSuccess)
        return true;
    if(status != errSecItemNotFound)
        return false;

    CFDictionarySetValue(query.mutableDictionary(), kSecValueData, data.get());
    return SecItemAdd(static_cast<CFDictionaryRef>(query.get()), nullptr) == errSecSuccess;
}

bool PasswordStore::deleteHubPassword(const std::string& server, const std::string& nick) {
    CFReleaseGuard query(createQuery(server, nick));
    if(!query.get())
        return false;

    OSStatus status = SecItemDelete(static_cast<CFDictionaryRef>(query.get()));
    return status == errSecSuccess || status == errSecItemNotFound;
}

} // namespace dcpp
