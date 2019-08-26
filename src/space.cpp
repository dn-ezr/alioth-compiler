#ifndef __space_cpp__
#define __space_cpp__

#include "space.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace alioth {
using namespace std;

#ifdef __WINDOWS__
const char SpaceEngine::dirdvc = '\\';
const string SpaceEngine::dirdvs = "\\";
const string SpaceEngine::default_work_path = ".\\";
const string SpaceEngine::default_root_path = "C:\\Alioth\\";
#else
const char SpaceEngine::dirdvc = '/';
const string SpaceEngine::dirdvs = "/";
const string SpaceEngine::default_work_path = "./";
const string SpaceEngine::default_root_path = "/usr/lib/alioth/";
#endif

Uri Uri::FromString( const string& str ) {

    if( IsPath(str) ) return FromPath(str);

    Uri uri;
    auto it = str.begin();

    /** 扫描scheme */
    while( isalpha(*it) ) it++;
    if( it == str.begin() ) throw invalid_argument("Uri::FromString( const string& str): no scheme found");
    if( *it++ != ':' or *it++ != '/' or *it++ != '/' ) throw invalid_argument("Uri::FromString( const string& str ): format error 01");
    uri.scheme = string(str.begin(),it-3);

    /** 扫描后续格式定界 */
    decltype(it) boundary[4] = { str.begin() };
    int c = 0;
    int base = 0;
    for( auto i = it; i != str.end(); i++ ) {
        if( *i == ':' or *i == '@' or *i == '/' or *i == '?' or *i == '#' )
            boundary[c++] = i;
        if( *i == '/' or *i == '?' or *i == '#' ) break;
    }

    /** 利用定界分析用户信息 */
    if( *boundary[0] == ':' and *boundary[1] == '@' ) {
        uri.user = string(it,boundary[0]);
        uri.password = string(boundary[0]+1,boundary[1]);
        it = boundary[1] + 1;
        base = 2;
    } else if( *boundary[0] == '@' ) {
        uri.user = string(it,boundary[0]);
        it = boundary[0] + 1;
        base = 1;
    }

    /** 利用定界分析主机 */
    if( c > base ) {
        uri.host = string(it,boundary[base]);
        it = boundary[base++];
    } else {
        uri.host = string(it,str.end());
        it = str.end();
    }

    /** 扫描端口 */
    if( *it == ':' ) {
        for( it += 1; isdigit(*it); it++ ) {
            uri.port = uri.port*10 + *it-'0';
        }
    }

    /** 扫描路径 */
    if( *it == '/' ) {
        decltype(it) i = it++;
        for( ; *i != '?' and *i != '#' and i != str.end(); i++ );
        uri.path = string(it,i);
        it = i;
    }

    /** 扫描请求 */
    if( *it == '?' ) {
        decltype(it) i = it++;
        for( ; *i != '#' and i != str.end(); i++ );
        uri.query = string(it,i);
        it = i;
    }

    /** 扫描请求 */
    if( *it == '#' ) {
        uri.fragment = string(it+1,str.end());
    }

    return uri;
}

Uri Uri::FromPath( const string& str ) {
    Uri uri;
    uri.scheme = "file";
    if( IsRelativePath(str) ) {
        auto path = getcwd(nullptr,0);
        uri.path = string(path+1);
        free(path);
        if( uri.path.back() != SpaceEngine::dirdvc ) uri.path += SpaceEngine::dirdvs;
        uri.path += string(str.begin()+2,str.end());
    } else if( IsAbsolutePath(str) ) {
        uri.path = string(str.begin()+1,str.end());
    } else {
        throw invalid_argument("Uri::FromPath( const string& str ): inacceptable path");
    }
    return uri;
}

bool Uri::IsPath( const string& str ) {
    return IsRelativePath(str) or IsAbsolutePath(str);
}

bool Uri::IsRelativePath( const string& str ) {
    return str[0] == '.' and str[1] == SpaceEngine::dirdvc;
}

bool Uri::IsAbsolutePath( const string& str ) {
    return str[0] == SpaceEngine::dirdvc;
}

Uri::operator string() const {
    return scheme + "://" 
    + user + (password.size()?":":"") + (user.size()?"@":"") + 
    host + 
    ((port>=0)?(":" + to_string(port)):"") +
    (path.size()?"/":"") + path +
    (query.size()?"?":"") + query +
    (fragment.size()?"#":"") + fragment;
}

bool SpaceEngine::Desc::isSpace() const {
    return SpaceEngine::IsSpace(flags);
}

bool SpaceEngine::Desc::isDocument() const {
    return SpaceEngine::IsDocument(flags);
}

bool SpaceEngine::Desc::isMainSpace() const {
    return SpaceEngine::IsMainSpace(flags);
}

bool SpaceEngine::Desc::isSubSpace() const {
    return SpaceEngine::IsSubSpace(flags);
}

json SpaceEngine::Desc::toJson() const {
    json ret = json::object;
    ret["flags"] = (double)flags;
    ret["name"] = name;
    ret["package"] = package;
    return ret;
}

json SpaceEngine::FullDesc::toJson() const {
    auto ret = Desc::toJson();
    ret["mtime"] = (double)mtime;
    ret["size"] = (double)size;
    return ret;
}

SpaceEngine::SpaceEngine() {
    mwork.desc.flags = WORK;
    mwork.mapping = Uri::FromString(default_work_path);
    mroot.desc.flags = ROOT;
    mroot.mapping = Uri::FromString(default_root_path);
}

bool SpaceEngine::setMainSpaceMapping( int space, const string& mapping, const string& package ) {
    if( !IsMainSpace(space) )
        throw invalid_argument("SpaceEngine::setMainSpaceMapping( int space, const string& mapping, const string& package ): argument 'space' doesn't specify a main space");
    Uri uri;
    try { uri = Uri::FromString(mapping); } catch( exception& e ) { return false; }
    switch( space ) {
        case WORK: mwork.mapping = uri; break;
        case ROOT: mroot.mapping = uri; break;
        case APKG: 
            if( package.empty() )
                throw invalid_argument("SpaceEngine::setMainSpaceMapping( int space, const string& mapping, const string& package ): argument 'package' used, but empty.");
            mapkg[package].mapping = uri;
            mapkg[package].desc = (Desc){
                flags: APKG,
                package:package
            };
        default:
            throw invalid_argument("SpaceEngine::setMainSpaceMapping( int space, const string& mapping, const string& package ): bad space specifier.");
    }

    return true;
}

chainz<SpaceEngine::FullDesc> SpaceEngine::enumerateContents( const SpaceEngine::Desc& desc ) {
    if( !desc.isSpace() ) 
        throw runtime_error("SpaceEngine::enumerateContents( const Desc& desc): descriptor doesn't describe a space.");
    auto uri = getUri(desc);
    DIR* dir = opendir(uri.path.data());
    if( !dir )
        throw runtime_error("SpaceEngine::enumerateContents( const Desc& desc ): bad mapping for space, cannot open it.");
    chainz<SpaceEngine::FullDesc> descs;
    
    dirent* p = nullptr;
    while( (p = readdir(dir)) != nullptr ) {
        string name(p->d_name);
        if( name == "." or name == ".." ) continue;
        
        name = uri.path + name;
        struct stat stat;
        if( ::stat(name.data(), &stat ) ) continue;
        
        FullDesc& ret = descs.construct(-1);
        ret.mtime = stat.st_mtime;
        ret.mtime = stat.st_size;
        ret.flags = desc.flags;
        ret.package = desc.package;
        ret.name = p->d_name;
        
        if( S_ISDIR(stat.st_mode) ) {
            if( desc.isMainSpace() ) {
                if( "arc" == ret.name ) ret.flags |= ARC;
                else if( "bin" == ret.name ) ret.flags |= BIN;
                else if( "doc" == ret.name ) ret.flags |= DOC;
                else if( "inc" == ret.name ) ret.flags |= INC;
                else if( "lib" == ret.name ) ret.flags |= LIB;
                else if( "obj" == ret.name ) ret.flags |= OBJ;
                else if( "src" == ret.name ) ret.flags |= SRC;
                else ret.flags |= EXT;
            } else {
                ret.flags |= EXT;
            }
        } else {
            ret.flags |= DOCUMENT;
        }
    }

    return descs;
}

bool SpaceEngine::createDocument( const SpaceEngine::Desc& desc, int mod ) {
    if( !desc.isDocument() )
        throw runtime_error("SpaceEngine::createDocument( const SpaceEngine::Desc& desc ): descriptor doesn't describe a document.");
    createSubSpace({flags:desc.flags&~DOCUMENT,name:desc.name,package:desc.package});
    if( auto fd = open(getPath(desc).data(),O_CREAT|O_RDONLY, mod ); fd < 0 ) {
        return false;
    } else {
        close(fd);
    }
    return true;
}

bool SpaceEngine::createSubSpace( const SpaceEngine::Desc& desc ) {
    if( !desc.isSubSpace() )
        throw runtime_error("SpaceEngine::createSubSpace( const SpaceEngine::Desc& desc )： descriptor doesn't describe a sub space.");
    auto path = getPath(desc);
    return mkdir(path.data(), 0755 ) == 0;
}

uistream SpaceEngine::openDocumentForRead( const SpaceEngine::Desc& desc ) {
    if( !desc.isDocument() )
        throw runtime_error("SpaceEngine::openDocumentForRead( const SpaceEngine::Desc& desc ): descriptor doesn't describe a document.");
    auto uri = getUri(desc);
    if( interactive ) {
        auto os = OpenStreamForWrite(interactive_o);
        auto is = OpenStreamForRead(interactive_i);
        json ask = json::object;
        ask["cmd"] = string("requestContent");
        ask["uri"] = uri;
        *os << ask.toJsonString();
        auto answer = json::FromJsonStream(*is);
        if( answer.is(json::object) and answer["ret"].is(json::boolean) and (bool)answer["ret"] ) {
            auto stream = std::make_unique<stringstream>();
            stream->str((string)answer["content"]);
            return stream;
        }
    }
    return OpenStreamForRead( uri );
}

uostream SpaceEngine::openDocumentForWrite( const SpaceEngine::Desc& desc ) {
    return OpenStreamForWrite(getUri(desc));
}

string SpaceEngine::getPath( const SpaceEngine::Desc& desc ) {
    auto uri = getUri( desc );

    if( uri.scheme == "file" ) {
        return dirdvs + uri.path;
    } else {
        throw runtime_error("SpaceEngine::getPath( const SpaceEngine::Desc& desc ): resource is not located on filesystem.");
    }
}

Uri SpaceEngine::getUri( const SpaceEngine::Desc& desc ) {
    auto main = desc.flags&0x0F00;
    auto sub = desc.flags&0x00FF;

    Uri uri;
    if( main == WORK ) {
        uri = mwork.mapping;
    } else if( main == ROOT ) {
        uri = mroot.mapping;
    } else if( main == APKG ) {
        if( desc.package.empty() )
            throw runtime_error("SpaceEngine::getUri( const SpaceEngine::Desc& desc ): no package name specified.");
        if( mapkg.count(desc.package) ) {
            uri = mapkg[desc.package].mapping;
        } else {
            uri = mroot.mapping;
            uri.path += string("pkg") + dirdvs + desc.package + dirdvs;
        }
    } else {
        throw runtime_error("SpaceEngine::getPath( const SpaceEngine::Desc& desc ): no main space specified.");
    }

    if( sub and uri.path.back() != dirdvc ) uri.path += dirdvs;
    switch( sub ) {
        case ARC: uri.path += "arc" + dirdvs; break;
        case BIN: uri.path += "bin" + dirdvs; break;
        case DOC: uri.path += "doc" + dirdvs; break;
        case INC: uri.path += "inc" + dirdvs; break;
        case LIB: uri.path += "lib" + dirdvs; break;
        case OBJ: uri.path += "obj" + dirdvs; break;
        case SRC: uri.path += "src" + dirdvs; break;
        case EXT: uri.path += desc.name + dirdvs; break;
        default: break;
    }

    if( desc.isDocument() ) {
        if( uri.path.back() != dirdvc ) uri.path += dirdvs;
        uri.path += desc.name;
    }

    return uri;
}

void SpaceEngine::enableInteractiveMode( int input, int output ) {
    interactive = true;
    interactive_i = input;
    interactive_o = output;
}

uistream SpaceEngine::OpenStreamForRead( int i ) {
    return make_unique<fdistream>(i);
}

uistream SpaceEngine::OpenStreamForRead( Uri uri ) {
    if( uri.scheme != "file" ) 
        throw runtime_error("SpaceEngine::OpenStreamForRead( Uri uri ): resource is not located on filesystem.");
    string path = dirdvs + uri.path;
    unique_ptr<ifstream> is = std::make_unique<ifstream>(path);
    if( !is->good() ) return nullptr;
    return is;
}

uostream SpaceEngine::OpenStreamForWrite( int i ) {
    return make_unique<fdostream>(i);
}

uostream SpaceEngine::OpenStreamForWrite( Uri uri ) {
    if( uri.scheme != "file" )
        throw runtime_error("SpaceEngine::OpenStreamForWrite( Uri uri ): resource is not located on filesystem.");
    string path = dirdvs + uri.path;
    unique_ptr<ofstream> os = make_unique<ofstream>(path);
    if( !os->good() ) return nullptr;
    return os;
}

bool SpaceEngine::IsSpace( int flags ) {
    return flags != 0 and (flags & DOCUMENT) == 0;
}

bool SpaceEngine::IsDocument(int flags ) {
    return flags & DOCUMENT != 0;
}

bool SpaceEngine::IsMainSpace( int flags ) {
    return (flags & 0x80ff) == 0 and (flags & 0x0f00) != 0;
}

bool SpaceEngine::IsSubSpace( int flags ) {
    return (flags & DOCUMENT) == 0 and (flags & 0x00ff) != 0;
}

fdistream::fdistream( int fd ):
istream(new __gnu_cxx::stdio_filebuf<char>(fd, ios::in)) {
}
fdistream::~fdistream() {
    delete _M_streambuf;
}

fdostream::fdostream( int fd ):
ostream(new __gnu_cxx::stdio_filebuf<char>(fd, ios::out)) {
}
fdostream::~fdostream() {
    delete _M_streambuf;
}

}

#endif