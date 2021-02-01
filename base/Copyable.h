/**
 * @brief copyable base class ,value type class
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_COPYABLE_H
#define WEB_SERVER_BASE_COPYABLE_H

namespace web_server {

class Copyable {
protected:
    Copyable() = default;
    ~Copyable() = default;
};

} // namespace web_server


#endif // WEB_SERVER_BASE_COPYABLE_H