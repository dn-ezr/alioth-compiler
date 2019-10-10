#ifndef __communicate_cpp__
#define __communicate_cpp__

#include "asock.hpp"
#include "space.hpp"

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
    if( in ) return false;
    in = GetInputStream( uri );
    if( !in ) return false;
    return true;

}

bool Socket::ActivateOutputStream( const Uri& uri ) {
    if( out ) return false;
    out = GetOutputStream( uri );
    if( !out ) return false;
    return true;
}

long Socket::requestContent( const Uri& uri ) {
    json pack = json::object;
    pack["uri"] = uri;
    return sendRequest(CONTENT, pack);
}

$BasicPackage Socket::receiveRespond( long seq ) {
    unique_lock<mutex> guard(*in);
    auto i = in->responds.find(seq);

    if( i == in->responds.end() ) {
        auto it = in->transactions.find(seq);
        if( it == in->transactions.end() ) {
            in->transactions[seq] = new Transaction;
            it = in->transactions.find(seq);
        }
        it->second->cv.wait(guard);
        auto i = in->responds.find(seq);
    }

    if( i == in->responds.end() ) throw runtime_error("Socket::receiveRespond( long seq ): Internal error occured");
    auto ret = i->second;
    return in->responds.erase(i), ret;
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
    
    while( true ) try {
        auto recv = json::FromJsonStream(*s->is);
        auto pack = ExtractPackage(recv);
        if( pack ) {
            lock_guard guard(*s);
            if( pack->action == REQUEST ) {
                s->requests << pack;
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

    catch( exception& e ) {/*nothing to be done*/}
}

$BasicPackage Socket::ExtractPackage( const json& data ) {
    if( !data.is(json::object) ) return nullptr;
    if( !data.count("seq", json::integer) ) return nullptr;
    if( !data.count("timestamp", json::integer) ) return nullptr;
    if( !data.count("action",json::string) ) return nullptr;
    if( !data.count("title", json::string) ) return nullptr;

    $BasicPackage package = new BasicPackage;
    package->seq = (long)data["seq"];
    package->timestamp = (long)data["timestamp"];

    if( auto title = (string)data["title"]; title == "content" ) {
        package->title = CONTENT;
    } else {
        return nullptr;
    }

    if( auto action = (string)data["action"]; action == "request" ) {
        package->action = REQUEST;
        return ($BasicPackage)ExtractRequestPackage(data, package);
    } else if( action == "respond" ) {
        package->action = RESPOND;
        return ($BasicPackage)ExtractRespondPackage(data, package);
    } else {
        return nullptr;
    }
}

$BasicPackage Socket::ExtractRequestPackage( const json& data, $BasicPackage package ) {
    return nullptr;
}

$BasicPackage Socket::ExtractRespondPackage( const json& data, $BasicPackage package ) {
    if( !data.count("status", json::integer) ) return nullptr;
    auto ex = (agent<RespondPackageExtension>)package->extension = new RespondPackageExtension;
    ex->status = (long)data["status"];

    switch( package->title ) {
        case CONTENT: {
            auto params = (agent<RespondContentPackageExtension>)ex->params = new RespondContentPackageExtension;
            if( data.count("data", json::string) ) 
                params->data = (string)data["data"];
        } break;
        default: return nullptr;
    }

    return package;
}

long Socket::sendRequest( Title title, json& pack ) {
    seq_lock.lock();
    long seq = (long)++global_seq;
    seq_lock.unlock();

    pack["seq"] = seq;
    pack["action"] = "request";
    switch( title ) {
        case CONTENT: pack["title"] = "content"; break;
        case VALIDATION: pack["title"] = "validation"; break;
        case ENUMERATE: pack["title"] = "enumerate"; break;
        default: throw logic_error("Socket::createRequestPackage(...): function not ready yet");
    }
    sendPackage(pack);
    return seq;
}

void Socket::sendRespond( long seq, json& pack ) {
    pack["seq"] = seq;
    sendPackage(pack);
    return;
}

void Socket::sendPackage( json& pack ) {
    if( !out ) throw logic_error("Socket::requestContent(...): please initial output stream first");
    pack["timestamp"] = time(nullptr);
    out->lock();
    *out->os << pack.toJsonString();
    out->unlock();
    return;
}

}

#endif