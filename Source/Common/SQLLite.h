/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// SQLLite functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef HAVE_SQLITE

//---------------------------------------------------------------------------
#ifndef SQLLiteH
#define SQLLiteH
//---------------------------------------------------------------------------

#include <sqlite3.h>
#include <vector>
#include <map>
#include <string>
//---------------------------------------------------------------------------

namespace MediaConch {

//***************************************************************************
// Class SQLLite
//***************************************************************************

class SQLLite
{
public:
    //Constructor/Destructor
    SQLLite();
    virtual ~SQLLite();

protected:
    virtual int execute();
    virtual int init(const std::string& db_dirname, const std::string& db_filename);

    std::string                                      query;
    std::vector<std::map<std::string, std::string> > reports;
    std::string                                      error;
    sqlite3                                         *db;
    sqlite3_stmt                                    *stmt; // Statement handler

    // Helper
    int std_string_to_int(const std::string& str);
    int get_db_version(int& version);
    int set_db_version(int version);
    const std::string& get_error() const;

    SQLLite (const SQLLite&);
    SQLLite& operator=(const SQLLite&);
};

}

#endif /* !SQLLiteH */

#endif /* !HAVE_SQLITE */
