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
#include "ptree.hpp"

namespace alioth {
using namespace std;

const string PackageLocator::THIS_ARCH = "x86_64";
#ifdef __WINDOWS__
const char SpaceEngine::dirdvc = '\\';
const string SpaceEngine::dirdvs = "\\";
const string SpaceEngine::default_work_path = ".\\";
const string SpaceEngine::default_root_path = "C:\\Alioth\\";
const string PackageLocator::THIS_PLATFORM = "windows";
#else
const char SpaceEngine::dirdvc = '/';
const string SpaceEngine::dirdvs = "/";
const string SpaceEngine::default_work_path = "./";
const string SpaceEngine::default_root_path = "/usr/lib/alioth/";
const string PackageLocator::THIS_PLATFORM = "linux";
#endif

const srcdesc srcdesc::error = {flags:0};
const Uri Uri::Bad = {port:-1};

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

Uri::operator bool()const {
    return port >= 0;
}

bool Uri::operator == ( const Uri& an ) const {
    if( port < 0 and an.port < 0 ) return false;
    return 
        scheme == an.scheme and
        user == an.user and
        password == an.password and
        host == an.host and
        port == an.port and
        path == an.path and
        query == an.query and
        fragment == an.fragment;

}

bool Uri::operator < ( const Uri& an ) const {
    return
        scheme < an.scheme and
        user < an.user and
        password < an.password and
        host < an.host and
        port < an.port and
        path < an.path and
        query < an.query and
        fragment < an.fragment;
}

PackageLocator::operator bool()const {
    return !name.empty() and !publisher.empty();
}

PackageLocator PackageLocator::Parse( const string& str ) {

    if( str.empty() ) return {};

    PackageLocator pl;
    string ps; // 以点分割的字符串，不包含点
    string sec; // 用于分析段名称的
    string aps; // 用于分析架构和平台的字符串
    bool ma = false; // 架构名称是否启用通配符
    bool mp = false; // 平台名称是否启用通配符
    int state = 1;
    bool stay = false;
    auto it = str.begin();
    int vof = 0; // 版本号偏移量，0表示正在分析主版本号，1表示次版本号，2表示补丁号

    while( state > 0 ) {
        switch( state ) {
            case 1: // 分析发布者或包名
                switch( *it ) {
                    case 'a' ... 'z': case 'A' ... 'Z': case '0' ... '9': case '_': 
                        ps += *it;
                        break;
                    case '.':
                        if( ps.empty() ) state = -__LINE__; //[ERROR]: 发布者字段中出现空片段
                        if( pl.publisher.empty() )
                            pl.publisher = ps;
                        else
                            pl.publisher += "." + ps;
                        ps.clear();
                        break;
                    case ':': case '-': case ' ': case '\0':
                        if( ps.empty() ) state = -__LINE__; //[ERROR]: 包名称为空
                        if( pl.publisher.empty() ) state = -__LINE__; //[ERROR]: 发布者为空
                        if( state < 0 ) break;
                        pl.name = ps;
                        ps.clear();
                        if( *it == ':' ) state = 2;
                        else if( *it == '-' ) state = 5;
                        else if( *it == ' ' ) state = 4;
                        else if( *it == '\0' ) (pl.major = pl.minor = pl.patch = -1),(state = 0);
                        else state = -__LINE__; //[ERROR]: 内部错误
                        break;
                    default:
                        state = -__LINE__; //[ERROR]: 包名称内无效的输入
                        break;
                } break;
            case 2: // 分析版本号
                switch( *it ) {
                    case '0' ... '9': {
                        auto& version = (
                            (vof==0)?
                                pl.major:
                                ((vof==1)?
                                    pl.minor:
                                    pl.patch ));
                        version = version*10 + *it-'0';
                    } break;
                    case '^': {
                        if( *(it-1) != '.' and *(it-1) != ':' ) {state = -__LINE__; break;} //[ERROR]: 匹配标记不在数字开端
                        pl.patch = -1;
                        if( vof < 2 ) pl.minor = -1;
                        if( vof < 1 ) pl.major = -1;
                        state = 3;
                    } break;
                    case '.':
                        vof += 1;
                        if( vof > 2 ) state = -__LINE__; //[ERROR]: 版本号超长
                        break;
                    case ' ': case '\0':
                        if( vof < 2 ) state = -__LINE__; //[ERROR]: 版本号不完整
                        else if( *it == ' ' ) state = 4;
                        else if( *it == '\0' ) state = 0;
                        else state = -__LINE__; //[ERROR]: 内部错误
                        break;
                    default:
                        state = -__LINE__; //[ERROR]: 版本号中无效的输入
                } break;
            case 3:
                switch( *it ) {
                    case ' ':
                        state = 4;
                        break;
                    case '\0':
                        state = 0;
                        break;
                    default:
                        state = -__LINE__; //[ERROR]: 最高匹配的版本号之后无效的输入
                        break;
                } break;
            case 4:
                switch( *it ) {
                    case 'a':case 'c':case 'd':case 'e':case 'i':case 'm':case 'n':case 'o':case 'v':
                        sec += *it;
                        break;
                    case ' ':case '\0': {
                        if( sec.empty() ) state = -__LINE__; //[ERROR]: 段名称为空
                        auto sof = (
                            (sec == "main")?
                                PackageLocator::MAIN:
                                (sec == "doc")?
                                    PackageLocator::DOC:
                                    (sec == "dev")?
                                        PackageLocator::DEV:
                                        0);
                        if( sof == 0 ) state = -__LINE__; //[ERROR]: 非法的段名称
                        if( (pl.sections bitand sof) != 0 ) state = -__LINE__; //[ERROR]: 重复的段
                        if( state < 0 ) break;
                        pl.sections bitor sof;
                        sec.clear();
                        if( *it == '\0' ) state = 0;
                    } break;
                    default:
                        state = -__LINE__; //[ERROR]: 段名称中非法的输入
                        break;
                } break;
            case 5:
                switch( *it ) {
                    case 'a' ... 'z':
                    case 'A' ... 'Z':
                    case '0' ... '9':
                    case '_': 
                        aps += *it;
                        if( ma ) state = -__LINE__; //[ERROR]: 通配符已启用
                        break;
                    case '%':
                        if( aps.empty() ) (ma = true),(aps += *it);
                        else state = -__LINE__; //[ERROR]: 通配符应当单独存在
                        break;
                    case '-': case ':':case ' ':case '\0':
                        if( aps.empty() ) state = -__LINE__; //[ERROR]: 架构名为空
                        else if( *it == '-' ) state = 6;
                        else if( *it == ':' ) state = 2;
                        else if( *it == ' ' ) state = 4;
                        else if( *it == '\0' )(pl.major = pl.minor = pl.patch = -1), (state = 0);
                        else state = -__LINE__; //[ERROR]: 内部错误
                        pl.arch = aps;
                        aps.clear();
                        break;
                    default:
                        state = -__LINE__; //[ERROR]: 非法输入
                        break;
                } break;
            case 6:
                switch( *it ) {
                    case 'a' ... 'z':
                    case 'A' ... 'Z':
                    case '0' ... '9':
                    case '_':
                        aps += *it;
                        if( mp ) state = -__LINE__; //[ERROR]: 通配符已启用
                        break;
                    case '%':
                        if( aps.empty() ) (mp = true),(aps += *it);
                        else state = -__LINE__; //[ERROR]: 通配符应当单独存在
                        break;
                    case ':': case ' ': case '\0':
                        if( aps.empty() ) state = -__LINE__; //[ERROR]: 平台名为空
                        else if( *it == ':' ) state = 2;
                        else if( *it == ' ' ) state = 4;
                        else if( *it == '\0' )(pl.major = pl.minor = pl.patch = -1), state = 0;
                        else state = -__LINE__; //[ERROR]: 内部错误
                        pl.platform = aps;
                        aps.clear();
                        break;
                    default:
                        state = -__LINE__; //[ERROR]: 非法输入
                        break;
                } break;
            default:
                state = -__LINE__; //[ERROR]: 内部错误
                break;
        }
        if( stay ) stay = false;
        else it ++;
    }

    if( state >= 0 ) return pl;
    return {};
}

string PackageLocator::toString( bool uriPath, const string& sarch, const string& splatform )const {
    string ret = publisher + (uriPath?"/":".") + name;
    
    if( arch.empty() or arch == "%" ) ret += "-" + sarch;
    else ret += "-" + arch;

    if( platform.empty() or platform == "%" ) ret += "-" + splatform;
    else ret += "-" + platform;

    if( uriPath ) return ret;

    if( major < 0 ) ret += ":^";
    else {
        ret += ":" + to_string(major) + ".";
        if( minor < 0 ) ret += "^";
        else {
            ret += to_string(minor) + ".";
            if( patch < 0 ) ret += "^";
            else ret += to_string(patch);
        }
    }

    return ret;
}

bool srcdesc::isSpace() const {
    return SpaceEngine::IsSpace(flags);
}

bool srcdesc::isDocument() const {
    return SpaceEngine::IsDocument(flags);
}

bool srcdesc::isMainSpace() const {
    return SpaceEngine::IsMainSpace(flags);
}

bool srcdesc::isSubSpace() const {
    return SpaceEngine::IsSubSpace(flags);
}

json srcdesc::toJson() const {
    json ret = json::object;
    ret["flags"] = (long)flags;
    ret["name"] = name;
    ret["package"] = package;
    return ret;
}

srcdesc srcdesc::fromJson( const json& object ) {
    srcdesc d;
    if( !object.is(json::object) ) return error;
    if( !object.count("flags") or !object["flags"].is(json::integer) ) return error;
    d.flags = (long)object["flags"];
    
    if( object.count("name") and object["name"].is(json::string) )
        d.name = object["name"];
    if( object.count("package") and object["package"].is(json::string) ) 
        d.package = object["package"];
    return d;
}

json fulldesc::toJson() const {
    auto ret = srcdesc::toJson();
    ret["mtime"] = (long)mtime;
    ret["size"] = (long)size;
    return ret;
}

fulldesc fulldesc::fromJson( const json& object ) {
    fulldesc d;
    if( !object.is(json::object) ) return error;
    if( !object.count("flags") or !object["flags"].is(json::integer) ) return error;
    d.flags = (long)object["flags"];

    if( object.count("name") and object["name"].is(json::string) )
        d.name = object["name"];
    if( object.count("package") and object["package"].is(json::string) ) 
        d.package = object["package"];
    if( object.count("mtime") and object["mtime"].is(json::integer) )
        d.mtime = (time_t)(long)object["mtime"];
    if( object.count("size") and object["size"].is(json::integer) )
        d.size = (size_t)(long)object["size"];
    return d;
}

srcdesc::operator bool()const {
    return flags != 0;
}

SpaceEngine::SpaceEngine() {
    mwork.desc.flags = WORK;
    mwork.mapping = Uri::FromString(default_work_path);
    mroot.desc.flags = ROOT;
    mroot.mapping = Uri::FromString(default_root_path);
    arch = PackageLocator::THIS_ARCH;
    platform = PackageLocator::THIS_PLATFORM;
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
            mapkg[package].desc = (srcdesc){
                flags: APKG,
                package:package
            };
        default:
            throw invalid_argument("SpaceEngine::setMainSpaceMapping( int space, const string& mapping, const string& package ): bad space specifier.");
    }

    return true;
}

void SpaceEngine::setArch( const string& ar ) {
    arch = ar;
}

void SpaceEngine::setPlatform( const string& pl ) {
    platform = pl;
}

chainz<fulldesc> SpaceEngine::enumerateContents( const srcdesc& desc ) {
    using namespace protocol;

    chainz<fulldesc> descs;
    if( !desc.isSpace() ) 
        throw runtime_error("SpaceEngine::enumerateContents( const srcdesc& desc): descriptor doesn't describe a space.");
    
    auto modifier = [&desc](fulldesc& ret, bool dir) {
        if( dir ) {
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
    };
    
    do if( interactive ) {
        auto seq = msock->requestContents(getUri(desc));
        auto pack = msock->receiveRespond(seq);
        if( !pack ) break;
        auto respond = pack->respond();
        if( respond->status == Status::SUCCESS ) {
            auto params = respond->contents();
            for( auto& [fname,item] : params->data ) {
                if( fname == "." or fname == ".." ) continue;
                fulldesc& ret = descs.construct(-1);
                ret.mtime = item.mtime;
                ret.size = item.size;
                ret.flags = desc.flags;
                ret.package = desc.package;
                ret.name = fname;
                modifier( ret, item.dir );
            }
            return descs;
        }
    } while( false );
    auto path = getPath(desc);
    DIR* dir = opendir(path.data());
    if( !dir )
        throw runtime_error("SpaceEngine::enumerateContents( const srcdesc& desc ): bad mapping for space, cannot open it.");
    
    dirent* p = nullptr;
    while( (p = readdir(dir)) != nullptr ) {
        string name(p->d_name);
        if( name == "." or name == ".." ) continue;
        
        name = path + name;
        struct stat stat;
        if( ::stat(name.data(), &stat ) ) continue;
        
        fulldesc& ret = descs.construct(-1);
        ret.mtime = stat.st_mtime;
        ret.size = stat.st_size;
        ret.flags = desc.flags;
        ret.package = desc.package;
        ret.name = p->d_name;
        
        modifier( ret, S_ISDIR(stat.st_mode) );
    }

    return descs;
}

chainz<string> SpaceEngine::enumeratePublishers() {
    chainz<string> res;
    for( auto& desc : enumerateContents({flags:ROOT,name:"pkg"}) )
        if( !desc.isDocument() ) res << desc.name;
    return res;
}

chainz<string> SpaceEngine::enumeratePackages( const string& publisher ) {
    chainz<string> res;
    if( publisher.empty() ) return {};
    for( auto& desc : enumerateContents({flags:ROOT,name:"pkg"+dirdvs+publisher}) )
        if( !desc.isDocument() ) res << desc.name;
    return res;
}

chainz<string> SpaceEngine::enumerateVersions( const string& publisher, const string& package ) {
    chainz<string> res;
    if( publisher.empty() or package.empty() ) return {};
    for( auto& desc : enumerateContents({flags:ROOT,name:"pkg"+dirdvs+publisher+dirdvc+package}))
        if( !desc.isDocument() ) res << desc.name;
    return res;
}

bool SpaceEngine::createDocument( const srcdesc& desc, int mod ) {
    if( !desc.isDocument() )
        throw runtime_error("SpaceEngine::createDocument( const srcdesc& desc ): descriptor doesn't describe a document.");
    createSubSpace({flags:desc.flags&~DOCUMENT,name:desc.name,package:desc.package});
    if( auto fd = open(getPath(desc).data(),O_CREAT|O_RDONLY, mod ); fd < 0 ) {
        return false;
    } else {
        close(fd);
    }
    return true;
}

bool SpaceEngine::createSubSpace( const srcdesc& desc ) {
    if( !desc.isSubSpace() )
        throw runtime_error("SpaceEngine::createSubSpace( const srcdesc& desc )： descriptor doesn't describe a sub space.");
    auto path = getPath(desc);
    return mkdir(path.data(), 0755 ) == 0;
}

uistream SpaceEngine::openDocumentForRead( const srcdesc& desc ) {
    using namespace protocol;
    if( !desc.isDocument() )
        throw runtime_error("SpaceEngine::openDocumentForRead( const srcdesc& desc ): descriptor doesn't describe a document.");
    auto uri = getUri(desc);
    do if( interactive ) {
        auto seq = msock->requestContent(uri);
        auto package = msock->receiveRespond(seq);
        if( !package ) break;
        if( auto ext = package->respond(); ext->status == Status::SUCCESS ) {
            auto stream = std::make_unique<stringstream>();
            auto param = ext->content();
            stream->str(param->data);
            return stream;
        }
    } while( false );
    return OpenStreamForRead( uri );
}

uostream SpaceEngine::openDocumentForWrite( const srcdesc& desc ) {
    return OpenStreamForWrite(getUri(desc));
}

bool SpaceEngine::reachDataSource( const srcdesc& desc ) {
    return ReachDataSource( getUri(desc) );
}

fulldesc SpaceEngine::statDataSource( const srcdesc& desc ) {
    if( !desc ) return srcdesc::error;
    auto uri = getUri(desc);
    if( uri.scheme != "file" )
        throw runtime_error("SpaceEngine::statDataSource( const srcdesc& desc ): data source doesn't located in filesystem.");
    auto path = getPath(desc);
    struct stat stat;
    if( ::stat(path.data(), &stat) ) return srcdesc::error;
    auto res = fulldesc(desc);
    res.size = stat.st_size;
    res.mtime = stat.st_mtime;
    return res;
}

fulldesc SpaceEngine::statDataSource( Uri uri ) {
    if( !uri ) return srcdesc::error;
    if( uri.scheme != "file" )
        throw runtime_error("SpaceEngine::statDataSource( Uri uri ): uri doesn't point to filesystem.");
    auto is = (string)uri;
    fulldesc desc;
    
    /** 准备主空间前缀树 */
    ptree<srcdesc> tree;
    tree[((string)getUri({flags:WORK})).data()].store({flags:WORK});
    tree[((string)getUri({flags:ROOT})).data()].store({flags:ROOT});
    for( auto& pub : enumeratePublishers() )
        for( auto& pkg : enumeratePackages(pub) )
            for(auto ver : enumerateVersions(pub,pkg) ) {
                auto pos = ver.find("latest");
                if( pos != string::npos ) ver.replace(pos,6, "^");
                auto package = pub+"."+pkg+":"+ver;
                tree[(pub+dirdvs+pkg+dirdvs+ver).data()].store({flags:APKG,package:package});
            }

    /** 检测主空间 */
    if( auto main = tree.far(is.data()); !main or !main->getp() ) {
        return srcdesc::error;
    } else {
        /** 准备子空间前缀树 */
        for( const auto& sub : enumerateContents(main->get()) )
            (*main)[((string)getUri(sub)).data()].store(sub);
        /** 检测子空间 */
        if( auto sub = tree.far(is.data()); !sub or !sub->getp() ) {
            return srcdesc::error;
        } else {
            auto& d = sub->get();
            auto ds = (string)getUri(d);
            return statDataSource({
                flags: d.flags,
                name: string(is.begin()+ds.size(),is.end()),
                package: d.package
            });
        }
    }
}

string SpaceEngine::getPath( const srcdesc& desc ) {
    auto uri = getUri( desc );

    if( uri.scheme == "file" ) {
        return dirdvs + uri.path;
    } else {
        throw runtime_error("SpaceEngine::getPath( const srcdesc& desc ): resource is not located on filesystem.");
    }
}

Uri SpaceEngine::getUri( const srcdesc& desc ) {
    auto main = desc.flags&0x0F00;
    auto sub = desc.flags&0x00FF;

    Uri uri;
    if( main == WORK ) {
        uri = mwork.mapping;
    } else if( main == ROOT ) {
        uri = mroot.mapping;
    } else if( main == APKG ) {
        if( desc.package.empty() )
            throw runtime_error("SpaceEngine::getUri( const srcdesc& desc ): no package name specified.");
        if( mapkg.count(desc.package) ) {
            uri = mapkg[desc.package].mapping;
        } else {
            auto loc = PackageLocator::Parse(desc.package);
            if( !loc ) throw runtime_error("SpaceEngine::getUri( const srdesc& desc ): invalid package locator.");
            uri = mroot.mapping;
            uri.path += string("pkg") + dirdvs + loc.toString(true,arch,platform) + dirdvs;
            if( loc.major < 0 ) uri.path += "latest";
            else {
                uri.path += to_string(loc.major) + ".";
                if( loc.minor < 0 ) uri.path += "latest";
                else {
                    uri.path += to_string(loc.minor) + ".";
                    if( loc.patch < 0 ) uri.path += "latest";
                    else uri.path += to_string(loc.patch);
                }
            }
        }
    } else {
        throw runtime_error("SpaceEngine::getUri( const srcdesc& desc ): no main space specified.");
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

bool SpaceEngine::enableInteractiveMode( int input, int output ) {
    if( interactive ) return true;
    if( !msock ) msock = new Socket;
    auto a = msock->ActivateInputStream({scheme:"fd",host:to_string(input),port:0});
    auto b = msock->ActivateOutputStream({scheme:"fd",host:to_string(output),port:0});
    if( a and b ) interactive = true;
    return a and b;
}

uistream SpaceEngine::OpenStreamForRead( int i ) {
    return make_unique<fdistream>(i);
}

uistream SpaceEngine::OpenStreamForRead( Uri uri ) {
    if( uri.scheme == "file" ) {
        string path = dirdvs + uri.path;
        unique_ptr<ifstream> is = std::make_unique<ifstream>(path);
        if( !is->good() ) return nullptr;
        return is;
    } else if( uri.scheme == "fd" ) {
        return OpenStreamForRead(stol(uri.host));
    } else {
        throw runtime_error("SpaceEngine::OpenStreamForRead( Uri uri ): unsolvable scheme: " + uri.scheme );
    }
}

uostream SpaceEngine::OpenStreamForWrite( int i ) {
    return make_unique<fdostream>(i);
}

uostream SpaceEngine::OpenStreamForWrite( Uri uri ) {
    if( uri.scheme == "file" ){
        string path = dirdvs + uri.path;
        unique_ptr<ofstream> os = make_unique<ofstream>(path);
        if( !os->good() ) return nullptr;
        return os;
    } else if( uri.scheme == "fd" ) {
        return OpenStreamForWrite(stol(uri.host));
    } else {
        throw runtime_error("SpaceEngine::OpenStreamForWrite( Uri uri ): unsolvable scheme: " + uri.scheme );
    }
}

bool SpaceEngine::ReachDataSource( Uri uri ) {
    if( uri.scheme != "file" ) {
        throw runtime_error("SpaceEngine::ReachDataSource( Uri uri ): resource is not located on filesystem.");
    }
    string path = dirdvs + uri.path;
    struct stat stat;
    if( ::stat( path.data(), &stat) ) return false;
    return true;
}

bool SpaceEngine::IsSpace( int flags ) {
    return flags != 0 and (flags & DOCUMENT) == 0;
}

bool SpaceEngine::IsDocument(int flags ) {
    return (flags & DOCUMENT) != 0;
}

bool SpaceEngine::IsMainSpace( int flags ) {
    return (flags & (DOCUMENT | SUB) ) == 0 and (flags & MAIN) != 0;
}

bool SpaceEngine::IsSubSpace( int flags ) {
    return (flags & DOCUMENT) == 0 and (flags & SUB) != 0;
}
int SpaceEngine::PeekMain( int flags ) {
    return flags & MAIN;
}
int SpaceEngine::PeekSub( int flags ) {
    return flags & SUB;
}

int SpaceEngine::HideMain( int flags ) {
    return flags & ~MAIN;
}
int SpaceEngine::HideSub( int flags ) {
    return flags & ~SUB;
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