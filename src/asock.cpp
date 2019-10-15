#ifndef __communicate_cpp__
#define __communicate_cpp__

#include "asock.hpp"
#include "space.hpp"
#include <iostream>

namespace alioth {

mutex Socket::repo_lock;
map<Uri,Socket::$InputStream> Socket::istreams;
map<Uri,Socket::$OutputStream> Socket::ostreams;
mutex Socket::seq_lock;
long Socket::global_seq = 0;

Socket::Socket() {}
Socket::Socket( Socket&& an ):
in(an.in),out(an.out) {

}

bool Socket::ActivateInputStream( const Uri& uri ) {
    using namespace protocol;
    if( in ) return false;
    in = GetInputStream( uri );
    if( !in ) return false;
    return true;

}

bool Socket::ActivateOutputStream( const Uri& uri ) {
    using namespace protocol;
    if( out ) return false;
    out = GetOutputStream( uri );
    if( !out ) return false;
    return true;
}

long Socket::requestContent( const Uri& uri ) {
    using namespace protocol;
    json pack = json::object;
    pack["uri"] = uri;
    return sendRequest(CONTENT, pack);
}

long Socket::requestContents( const Uri& uri ) {
    using namespace protocol;
    json pack = json::object;
    pack["uri"] = uri;
    return sendRequest(CONTENTS, pack);
}

void Socket::respondDiagnostics( long seq, const json& diagnostics ) {
    using namespace protocol;
    json pack = json::object;
    pack["diagnostics"] = diagnostics;
    pack["status"] = (long)SUCCESS;
    return sendRespond(seq, DIAGNOSTICS, pack);
}

void Socket::respondSuccess( long seq, protocol::Title title ) {
    using namespace protocol;
    json pack = json::object;
    pack["status"] = (long)SUCCESS;
    return sendRespond(seq, title, pack);
}

void Socket::respondFailure( long seq, protocol::Title title ) {
    using namespace protocol;
    json pack = json::object;
    pack["status"] = (long)FAILED;
    return sendRespond(seq, title, pack);
}

void Socket::reportException( const string& msg ) {
    using namespace protocol;
    json pack = json::object;
    pack["status"] = (long)FAILED;
    pack["msg"] = msg;
    return sendRespond(0, EXCEPTION, pack);
}

protocol::$Package Socket::receiveRespond( long seq ) {
    using namespace protocol;
    unique_lock<mutex> guard(*in);

    auto i = in->responds.find(seq);
    if( i == in->responds.end() ) {
        auto it = in->transactions.find(seq);
        if( it == in->transactions.end() ) {
            in->transactions[seq] = new Transaction;
            it = in->transactions.find(seq);
        }
        it->second->cv.wait(guard);
        i = in->responds.find(seq);
    }

    if( i == in->responds.end() ) throw runtime_error("Socket::receiveRespond( long seq ): Internal error occured");
    auto ret = i->second;
    return in->responds.erase(i), ret;
}

protocol::$Package Socket::receiveRequest() {
    using namespace protocol;
    unique_lock<mutex> guard(*in);
    while( in->requests.size() == 0 and in->is ) in->cvreq.wait(guard);
    if( !in->is ) {
        auto pack = new Package;
        pack->action = REQUEST;
        pack->title = EXIT;
        return pack;
    }
    if( !in->requests.size() ) return nullptr;
    auto pack = in->requests[0];
    return in->requests.remove(0), pack;
}

Socket::$InputStream Socket::GetInputStream( const Uri& uri ) {
    std::lock_guard guard(repo_lock);

    if( !uri ) return nullptr;
    auto& s = istreams[uri];
    if( s ) return s;    

    auto is = SpaceEngine::OpenStreamForRead(uri);
    if( !is ) return nullptr;

    s = new InputStream(std::move(is));
    s->guard = new thread(GuardInputStream, s);

    return s;
}

Socket::$OutputStream Socket::GetOutputStream( const Uri& uri ) {
    std::lock_guard guard(repo_lock);

    if( !uri ) return nullptr;
    auto& s = ostreams[uri];
    if( s ) return s;

    auto os = SpaceEngine::OpenStreamForWrite(uri);
    if( !os ) return nullptr;

    s = new OutputStream(std::move(os));

    return s;
}

void Socket::GuardInputStream( $InputStream s ) {
    using namespace protocol;
    
    while( true ) try {
        if( s->is->peek() == EOF ){
            s->is = nullptr;
            s->cvreq.notify_all();
            break;
        } else {
            auto recv = json::FromJsonStream(*s->is);
            auto pack = ExtractPackage(recv);
            if( pack ) {
                lock_guard guard(*s);
                if( pack->action == REQUEST ) {
                    s->requests << pack;
                    s->cvreq.notify_all();
                } else if( pack->action == RESPOND ) {
                    s->responds[pack->seq] = pack;
                    if( s->transactions.count(pack->seq) ) {
                        auto tr = s->transactions.find(pack->seq);
                        tr->second->cv.notify_all();
                        s->transactions.erase(tr);
                    }
                }
            }
        }
    } 

    catch( exception& e ) {/*nothing to be done*/}
}

protocol::$Package Socket::ExtractPackage( const json& data ) {
    using namespace protocol;
    if( !data.is(json::object) ) return nullptr;
    if( !data.count("seq", json::integer) ) return nullptr;
    if( !data.count("timestamp", json::integer) ) return nullptr;
    if( !data.count("action",json::string) ) return nullptr;
    if( !data.count("title", json::string) ) return nullptr;

    $Package package = new Package;
    package->seq = (long)data["seq"];
    package->timestamp = (long)data["timestamp"];

    if( auto title = (string)data["title"]; title == TitleStr(CONTENT) ) {
        package->title = CONTENT;
    } else if( title == TitleStr(CONTENTS) ) {
        package->title = CONTENTS;
    } else if( title == TitleStr(DIAGNOSTICS) ) {
        package->title = DIAGNOSTICS;
    } else if( title == TitleStr(WORKSPACE) ) {
        package->title = WORKSPACE;
    } else if( title == TitleStr(EXIT) ) {
        package->title = EXIT;
    } else {
        return nullptr;
    }

    if( auto action = (string)data["action"]; action == "request" ) {
        package->action = REQUEST;
        return ($Package)ExtractRequestPackage(data, package);
    } else if( action == "respond" ) {
        package->action = RESPOND;
        return ($Package)ExtractRespondPackage(data, package);
    } else {
        return nullptr;
    }
}

protocol::$Package Socket::ExtractRequestPackage( const json& data, protocol::$Package package ) {
    using namespace protocol;

    package->extension = new Extension::Request;
    auto ex = package->request();
    
    switch( package->title ) {
        case DIAGNOSTICS: {
            ex->params = new Parameter::Request::Diagnostics;
            auto params = ex->diagnostics();

            if( !data.count("targets", json::array) ) return nullptr;

            for( const auto& target : data["targets"] ) {
                if( !target.is(json::string) ) continue;
                params->targets << (string)target;
            }
        } break;
        case WORKSPACE: {
            ex->params = new Parameter::Request::Workspace;
            auto params = ex->workspace();

            if( !data.count("uri", json::string) ) return nullptr;

            params->uri = data["uri"];
        } break;
        case EXIT: {

        } break;
        default: return nullptr;
    }

    return package;
}

protocol::$Package Socket::ExtractRespondPackage( const json& data, protocol::$Package package ) {
    using namespace protocol;
    if( !data.count("status", json::integer) ) return nullptr;
    
    package->extension = new Extension::Respond;
    auto ex = package->respond();
    ex->status = (Status)(long)data["status"];

    switch( package->title ) {
        case CONTENT: {
            ex->params = new Parameter::Respond::Content;
            auto params = ex->content();
            if( data.count("data", json::string) ) 
                params->data = (string)data["data"];
        } break;
        case CONTENTS: {
            ex->params = new Parameter::Respond::Contents;
            auto params = ex->contents();
            if( data.count("data", json::object) ) {
                data["data"].for_each([&]( const string& key, const json& item ){
                    if( !item.is(json::object) ) return true;
                    if( !item.count("size", json::integer) ) return true;
                    if( !item.count("mtime", json::integer) ) return true;
                    if( !item.count("dir", json::boolean) ) return true;
                    params->data[key] = {
                        size:(size_t)(long)item["size"],
                        mtime:(time_t)(long)item["mtime"],
                        dir:(bool)item["dir"]
                    };
                    return true;
                });
            }
        } break;
        default: return nullptr;
    }

    return package;
}

long Socket::sendRequest( protocol::Title title, json& pack ) {
    using namespace protocol;
    seq_lock.lock();
    long seq = (long)++global_seq;
    seq_lock.unlock();

    pack["seq"] = seq;
    pack["action"] = string("request");
    pack["title"] = TitleStr(title);
    sendPackage(pack);
    return seq;
}

void Socket::sendRespond( long seq, protocol::Title title, json& pack ) {
    pack["seq"] = seq;
    pack["action"] = string("respond");
    pack["title"] = TitleStr(title);
    sendPackage(pack);
    return;
}

void Socket::sendPackage( json& pack ) {
    if( !out ) throw logic_error("Socket::requestContent(...): please initial output stream first");
    pack["timestamp"] = time(nullptr);
    out->lock();
    auto data = pack.toJsonString();
    *out->os << data << endl;
    // this_thread::sleep_for(std::chrono::microseconds{4});
    out->unlock();
    return;
}

string protocol::TitleStr( Title title ) {
    switch( title ) {
        case CONTENT: return "content";
        case CONTENTS: return "contents";
        case WORKSPACE: return "workspace";
        case DIAGNOSTICS: return "diagnostics";
        case EXIT: return "exit";
        case EXCEPTION: return "exception";
        default: return "";
    }
}

}

#endif