#ifndef __diagnostic_cpp__
#define __diagnostic_cpp__

#include "diagnostic.hpp"
#include <ctime>
#include <regex>

namespace alioth {

DiagnosticEngine::DiagnosticEngine():language(nullptr),format("%p:%l:%c:(%s<%E>) %i") {
    auto& deflang = languages["default"];
    language = &deflang;

    deflang.severities[0] = "\033[1;31m错误\033[0m";
    deflang.severities[1] = "\033[1;35m警告\033[0m";
    deflang.severities[2] = "\033[1;34m信息\033[0m";
    deflang.severities[3] = "\033[1;36m提示\033[0m";

    deflang.templates["0"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "诊断引擎配置文件缺失" };

    deflang.templates["1"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "诊断引擎配置文件损坏" };

    deflang.templates["2"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "命令行选项'%B0'的参数缺失" };

    deflang.templates["3"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "工作空间重映射失败" };

    deflang.templates["4"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "根空间重映射失败" };
    
    deflang.templates["5"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "选项'%B0'无效，未找到诊断语种'%R1'" };
    
    deflang.templates["6"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "交互模式启动失败，参数'%R0'格式不正确" };

    deflang.templates["7"] = {
        severity: 1,
        beg: {0,0},
        end: {0,0},
        msg: "未找到执行目标" };
}

bool DiagnosticEngine::configure( const json& config ) {
    bool correct = true;
    if( !config.is(json::object) ) return false;
    if( config.count("format") and config["format"].is(json::string) ) configureFormat(config["format"]);
    if( config.count("languages") and config["languages"].is(json::object) ) {
        config["languages"].for_each( [&]( const string& key, const json& language ) -> bool {
            if( !configureLanguage( key, language ) ) correct = false;
            return true;
        });
    }
    if( config.count("default-language") and config["default-language"].is(json::string) ) 
        correct = selectLanguage( config["default-language"] ) and correct;
    return correct;
}

void DiagnosticEngine::configureFormat( const string& fmt ) {
    format = fmt;
}

bool DiagnosticEngine::configureLanguage( const string& language, const json& config ) {
    if( language.empty() or !config.is(json::object) ) return false;
    auto& lang = languages[language];
    bool correct = true;
    if( config.count("severities") ) {
        correct = correct and [&](){
            const auto& severities = config["severities"];
            if( !severities.is(json::array) ) return false;
            if( severities.count() != 4 ) return false;
            for( const auto& s : severities ) if( !s.is(json::string) ) return false;
            lang.severities[0] = severities[0];
            lang.severities[1] = severities[1];
            lang.severities[2] = severities[2];
            lang.severities[3] = severities[3];
            return true;
        }();
    }

    if( config.count("templates") ) {
        const auto& conf = config["templates"];
        if( !conf.is(json::object) ) return false;
        auto& templates = lang.templates;
        auto rpos = regex(R"(n|(b|e)\d+)");
        auto pos_detector = [&](const string& pos ) -> tuple<int,int> {
            if( pos == "n" ) return {0,0};
            if( pos[0] == 'b' ) return {1,strtol(pos.data()+1,nullptr,10)};
            if( pos[0] == 'e' ) return {-1,strtol(pos.data()+1,nullptr,10)};
            throw runtime_error("DiagnosticEngine::configureLanguage( const string& language, const json& config ): bad boundary indicator for diagnostic template.");
        };
        conf.for_each([&]( const string& key, const json& value ) -> bool {
            auto& temp = templates[key];
            if( !value.is(json::object) ) {correct = false;return true;}
            if( value.count("sev") == 0 or !value["sev"].is(json::integer) ) {correct = false;return true;}
            if( value.count("beg") == 0 or !value["beg"].is(json::string) and regex_match((string)value["beg"],rpos) ) {correct = false;return true;}
            if( value.count("end") == 0 or !value["end"].is(json::string) and regex_match((string)value["end"],rpos) ) {correct = false;return true;}
            if( value.count("msg") == 0 or !value["msg"].is(json::string) ) {correct = false;return true;}

            templates[key] = (DiagnosticTemplate){
                severity: (int)(long)value["sev"],
                beg: pos_detector(value["beg"]),
                end: pos_detector(value["end"]),
                msg: value["msg"]
            };
            return true;
        });

    }

    return correct;
}

chainz<string> DiagnosticEngine::enumerateLanguages() const {
    chainz<string> language_names;
    for( const auto& [key,value]: languages ) language_names.construct(-1,key);
    return language_names;
}

bool DiagnosticEngine::selectLanguage( const string& lang ) {
    if( languages.count(lang) == 0 ) return false;
    language = &languages[lang];
    return true;
}

string DiagnosticEngine::printToString( const Diagnostic& d )const {
    string res;
    int off = 0;

    if( !language ) throw runtime_error("DiagnosticEngine::printToString( const Diagnostic& d, const DiagnosticLanguage* lang )const: language options unavailable");
    if( !language->templates.count(d.code) ) throw runtime_error("DiagnosticEngine::printToString( const Diagnostic& d, const DiagnosticLanguage* lang )const: no corresponding diagnostic template found for error code "+d.code);
    auto tmpl = language->templates.at(d.code);
    
    while( format[off] != '\0' ) {
        if( format[off] == '%' ) switch( format[++off] ) {
            case 'c':
                if( auto [lo,nu] = tmpl.beg; lo == 0 ) res += "0";
                else if( lo > 0 ) res += to_string(d.args[nu].bc);
                else res += to_string(d.args[nu].ec);
                break;
            case 'C':
                if( auto [lo,nu] = tmpl.end; lo == 0 ) res += "0";
                else if( lo > 0 ) res += to_string(d.args[nu].bc);
                else res += to_string(d.args[nu].ec);
                break;
            case 'd': {
                auto t = time(nullptr);
                auto tm = localtime_r( &t, new struct tm );
                char buf[16];
                sprintf( buf,"%04d/%02d/%02d", tm->tm_year, tm->tm_mon, tm->tm_mday );
                res += buf;
                delete tm;
            } break;
            case 'E':
                res += d.code;
                break;
            case 'i':
                res += organizeDiagnosticInformation( d );
                break;
            case 'l':
                if( auto [lo,nu] = tmpl.beg; lo == 0 ) res += "0";
                else if( lo > 0 ) res += to_string(d.args[nu].bl);
                else res += to_string(d.args[nu].el);
                break;
            case 'L':
                if( auto [lo,nu] = tmpl.end; lo == 0 ) res += "0";
                else if( lo > 0 ) res += to_string(d.args[nu].bl);
                else res += to_string(d.args[nu].el);
                break;
            case 'p': 
                res += d.prefix;
                break;
            case 's': 
                res += language->severities[tmpl.severity-1];
                break;
            case 'S': 
                res += to_string(tmpl.severity);
                break;
            case 't': {
                auto t = time(nullptr);
                auto tm = localtime_r( &t, new struct tm );
                char buf[16];
                sprintf( buf,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec );
                res += buf;
                delete tm;
            } break;
            case 'T': 
                res += to_string(time(nullptr));
                break;
            default:
                throw runtime_error("DiagnosticEngine::printToString( const Diagnostic& d )const: bad format variable indicator.");
        } else {
            res += format[off];
        }
        off += 1;
    }

    for( auto i : printToString(d.info) ) {
        i.insert(0,"\n");
        for( int o = 0; o < i.size(); o++ ) {
            if( i[o] == '\n' ) i.insert(++o,"\t");
        }
        res += i;
    }

    return res;
}

chainz<string> DiagnosticEngine::printToString( const Diagnostics& s )const {
    chainz<string> diagnostics;
    for( const auto& d : s ) diagnostics.construct(-1, printToString(d) );
    return diagnostics;
}

json DiagnosticEngine::printToJson( const Diagnostic& d )const {
    json diagnostic = json::object;

    if( !language ) throw runtime_error("DiagnosticEngine::printToString( const Diagnostic& d, const DiagnosticLanguage* lang )const: language options unavailable");
    if( !language->templates.count(d.code) ) throw runtime_error("DiagnosticEngine::printToString( const Diagnostic& d, const DiagnosticLanguage* lang )const: no corresponding diagnostic template found");
    auto tmpl = language->templates.at(d.code);

    diagnostic["severity"] = (long)tmpl.severity;
    diagnostic["prefix"] = d.prefix;
    diagnostic["error_code"] = d.code;
    diagnostic["message"] = organizeDiagnosticInformation(d, false);

    if( auto [lo,nu] = tmpl.beg; lo == 0 ) diagnostic["begin_line"] = (long)0;
    else if( lo > 0 ) diagnostic["begin_line"] = (long)d.args[nu].bl;
    else diagnostic["begin_line"] = (long)d.args[nu].el;

    if( auto [lo,nu] = tmpl.beg; lo == 0 ) diagnostic["begin_column"] = (long)0;
    else if( lo > 0 ) diagnostic["begin_column"] = (long)d.args[nu].bc;
    else diagnostic["begin_column"] = (long)d.args[nu].ec;

    if( auto [lo,nu] = tmpl.end; lo == 0 ) diagnostic["end_line"] = (long)0;
    else if( lo > 0 ) diagnostic["end_line"] = (long)d.args[nu].bl;
    else diagnostic["end_line"] = (long)d.args[nu].el;

    if( auto [lo,nu] = tmpl.end; lo == 0 ) diagnostic["end_column"] = (long)0;
    else if( lo > 0 ) diagnostic["end_column"] = (long)d.args[nu].bc;
    else diagnostic["end_column"] = (long)d.args[nu].ec;

    auto& informations = diagnostic["informations"];
    informations = json(json::array);
    for( const auto& info : d.info ) 
        informations[informations.count()] = printToJson(info);

    return diagnostic;
}

json DiagnosticEngine::printToJson( const Diagnostics& s )const {
    json diagnostics = json::array;
    for( const auto& d : s ) diagnostics[diagnostics.count()] = printToJson(d);
    return diagnostics;
}

string DiagnosticEngine::organizeDiagnosticInformation( const Diagnostic& d, bool colored ) const {
    string res;
    int off = 0;
    int state = 1;
    bool stay = false;

    const auto& tmpl = language->templates.at(d.code);
    while( state > 0 ) {
        switch( auto c = tmpl.msg[off]; state ) {
            case 1:
                if( c == '%' ) state = 2;
                else if( c == '\0' ) state = 0;
                else res += c;
                break;
            case 2:
                if( isdigit(c) ) {stay = true;state=3;break;}
                else if( !colored ) {state=3;break;}
                else switch( c ) {
                    case 'r':res += "\033[31m";break;
                    case 'R':res += "\033[1;31m";break;
                    case 'g':res += "\033[32m";break;
                    case 'G':res += "\033[1;32m";break;
                    case 'b':res += "\033[34m";break;
                    case 'B':res += "\033[1;34m";break;
                    case 'y':res += "\033[33m";break;
                    case 'Y':res += "\033[1;33m";break;
                    case 'p':res += "\033[35m";break;
                    case 'P':res += "\033[1;35m";break;
                    case 'c':res += "\033[36m";break;
                    case 'C':res += "\033[1;36m";break;
                    default: throw invalid_argument("DiagnosticEngine::organizeDiagnosticInformation( const Diagnostic& d, bool colored ): bad diagnostic variable color indicator.");
                }
                state = 3;
                break;
            case 3: {
                if( !isdigit(c) ) throw invalid_argument("DiagnosticEngine::organizeDiagnosticInformation( const Diagnostic& d, bool colored ): bad diagnostic variable indicator.");
                res += d.args[c-'0'];
                if( colored ) res += "\033[0m";
                state = 1;
                break;
            }
        }
        if( stay ) stay = false;
        else off += 1;
    }

    return res;
}

}

#endif