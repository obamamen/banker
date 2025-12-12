/* ================================== *\
 @file     http_server.hpp
 @project  banker
 @author   moosm
 @date     12/12/2025
*\ ================================== */

#ifndef BANKER_HTTP_SERVER_HPP
#define BANKER_HTTP_SERVER_HPP

#include <algorithm>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <list>
#include <string>
#include <vector>

#include "banker/core/networker/core/stream_socket/stream_socket.hpp"

namespace fs = std::filesystem;

struct directory_entry
{
    std::string name;
    bool is_directory;
    uint64_t file_size;
};

std::vector<directory_entry> list_directory(const std::string& path)
{
    std::vector<directory_entry> entries;
    try
    {
        for (const auto& p : std::filesystem::directory_iterator(path))
        {
            directory_entry entry;
            entry.name = p.path().filename().string();
            entry.is_directory = p.is_directory();
            entry.file_size = p.is_regular_file() ? p.file_size() : 0;
            entries.push_back(entry);
        }
        std::sort(entries.begin(), entries.end(), [](const directory_entry &a, const directory_entry &b)
        {
            if (a.is_directory != b.is_directory)
                return a.is_directory > b.is_directory;
            return a.name < b.name;
        });
    }
    catch (const std::filesystem::filesystem_error& e)
    {
    }
    return entries;
}

inline std::string url_encode(const std::string& str)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (const char c : str)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/')
        {
            escaped << c;
        }
        else
        {
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return escaped.str();
}

inline std::string html_escape(const std::string& str)
{
    std::string escaped;
    for (char c : str)
    {
        switch(c)
        {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

inline std::string generate_breadcrumb(const std::string& path)
{
    std::string html = "<div class=\"breadcrumb\"><a href=\"/\">./</a>";
    if (path.empty() || path == ".")
    {
        html += "</div>";
        return html;
    }
    std::string accumulated = "/";
    std::istringstream ss(path);
    std::string segment;
    while (std::getline(ss, segment, '/'))
    {
        if (segment.empty() || segment == ".") continue;
        accumulated += segment + "/";
        html += " -&gt; <a href=\"" + accumulated + "\">" + html_escape(segment) + "/</a>";
    }
    html += "</div>";
    return html;
}

inline std::string format_file_size(const uint64_t size)
{
    const char* units[] = {" B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_d = static_cast<double>(size);
    while (size_d >= 1024.0 && unit_index < 4)
    {
        size_d /= 1024.0;
        unit_index++;
    }
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f %s", size_d, units[unit_index]);
    return std::string(buffer);
}

inline size_t get_max_name_length(const std::vector<directory_entry>& entries)
{
    size_t max_len = 0;
    for (const auto& entry : entries)
    {
        size_t len = entry.name.length();
        if (entry.is_directory) len += 1;
        if (len > max_len) max_len = len;
    }
    return max_len;
}

inline std::string get_css()
{
    return R"css(
body {
    font-family: 'Courier New', Courier, monospace;
    margin: 20px;
    background: #f5f5f5;
    color: #000000;
    line-height: 1.6;
}
.breadcrumb {
    padding: 10px;
    background: #e0e0e0;
    border: 1px solid #808080;
    margin-bottom: 20px;
    font-weight: bold;
    position: sticky;
    top: 0;
    z-index: 100;
}
.breadcrumb a {
    color: #0000cc;
    text-decoration: underline;
}
/*.breadcrumb a:visited {
    color: #551a8b;
}*/
.listing {
    background: #ffffff;
    border: 1px solid #c0c0c0;
    padding: 15px;
}
.entry {
    display: flex;
    align-items: center;
    padding: 3px 0;
    font-family: 'Courier New', Courier, monospace;
}
.entry-name {
    flex: 1 1 auto;
    /*margin-right: 20px;*/
}
.entry-name a {
    color: #0000cc;
    text-decoration: none;
}
/*.entry-name a:visited {
    color: #551a8b;
}*/
.entry-name a:hover {
    text-decoration: underline;
}
.entry-size {
    flex: 0 0 180px;
    text-align: right;
    align-items: center;
    color: #666666;
    margin-right: 50%;
    white-space: pre;
}
.entry-download {
    flex: 0 0 100px;
    text-align: center;
}
.entry-download a {
    color: #0000cc;
    text-decoration: none;
    padding: 2px 6px;
    border: 1px solid #0000cc;
    background: #e8f4ff;
}
.entry-download a:hover {
    background: #d0e8ff;
    text-decoration: none;
}
)css";
}

inline std::string generate_directory_listing(const std::string& path)
{
    std::vector<directory_entry> entries = list_directory(path);
    size_t max_name = get_max_name_length(entries);

    std::string html = "<div class=\"listing\">";
    for (auto& entry : entries)
    {
        std::string href = "/" + path;
        if (path == ".") href = "/";
        if (!href.empty() && href.back() != '/') href += "/";
        href += url_encode(entry.name);

        std::string display_name = html_escape(entry.name);
        if (entry.is_directory) display_name += "/";


        size_t padding = max_name - display_name.length() + (entry.is_directory ? 0 : 1);
        std::string padded_name = display_name + std::string(padding, ' ');

        html += "<div class=\"entry\">";
        html += "<div class=\"entry-name\">";
        if (entry.is_directory)
        {
            html += "<a href=\"" + href + "/\">" + padded_name + "</a>";
        }
        else
        {
            html += "<a href=\"" + href + "\">" + padded_name + "</a>";
        }
        html += "</div>";

        if (!entry.is_directory)
        {
            html += "<div class=\"entry-size\">" + format_file_size(entry.file_size) + "</div>";
            html += "<div class=\"entry-download\"><a href=\"" + href + "?download=1\">[download]</a></div>";
        }

        html += "</div>";
    }
    html += "</div>";
    return html;
}

inline std::string url_decode(const std::string& str)
{
    std::string decoded;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '%' && i + 2 < str.length())
        {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value)
            {
                decoded += static_cast<char>(value);
                i += 2;
            }
            else
            {
                decoded += str[i];
            }
        }
        else if (str[i] == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += str[i];
        }
    }
    return decoded;
}

inline std::string http_process(const std::string& input)
{
    std::istringstream stream(input);
    std::string method, request_path, version;
    if (!(stream >> method >> request_path >> version) || method != "GET")
    {
        std::string body = "<h1>400 Bad Request</h1>";
        std::ostringstream response;
        response << "HTTP/1.1 400 Bad Request\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << body.size() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << body;
        return response.str();
    }
    bool is_download = false;
    size_t query_pos = request_path.find('?');
    if (query_pos != std::string::npos)
    {
        std::string query = request_path.substr(query_pos + 1);
        if (query.find("download=1") != std::string::npos)
        {
            is_download = true;
        }
        request_path = request_path.substr(0, query_pos);
    }
    if (!request_path.empty() && request_path[0] == '/')
    {
        request_path.erase(0, 1);
    }
    request_path = url_decode(request_path);
    std::string path = request_path.empty() ? "." : request_path;
    std::string body;
    std::string content_type = "text/html";
    if (fs::exists(path))
    {
        if (fs::is_directory(path))
        {
            body = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><style>" + get_css() + "</style></head><body>";
            body += generate_breadcrumb(path);
            body += generate_directory_listing(path);
            body += "</body></html>";
        }
        else
        {
            std::ifstream file(path, std::ios::binary);
            std::ostringstream ss;
            ss << file.rdbuf();
            body = ss.str();
            std::string ext = fs::path(path).extension().string();
            if (ext == ".html" || ext == ".htm") content_type = "text/html";
            else if (ext == ".txt") content_type = "text/plain";
            else if (ext == ".css") content_type = "text/css";
            else if (ext == ".js") content_type = "application/javascript";
            else if (ext == ".json") content_type = "application/json";
            else if (ext == ".jpg" || ext == ".jpeg") content_type = "image/jpeg";
            else if (ext == ".png") content_type = "image/png";
            else if (ext == ".gif") content_type = "image/gif";
            else if (ext == ".pdf") content_type = "application/pdf";
            else content_type = "application/octet-stream";
        }
    }
    else
    {
        body = "<!DOCTYPE html><html><head><style>" + get_css() + "</style></head><body><h1>404 Not Found</h1></body></html>";
    }
    std::ostringstream response;
    if (fs::exists(path))
    {
        response << "HTTP/1.1 200 OK\r\n";
    }
    else
    {
        response << "HTTP/1.1 404 Not Found\r\n";
    }
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    if (is_download && fs::exists(path) && !fs::is_directory(path))
    {
        response << "Content-Disposition: attachment; filename=\"" << fs::path(path).filename().string() << "\"\r\n";
    }
    response << "Connection: close\r\n\r\n";
    response << body;
    return response.str();
}

[[noreturn]] inline void http_server(const bool log)
{
    banker::networker::stream_socket::acceptor server("0.0.0.0", 0);
    uint16_t port = server.raw_socket().get_local_info().port;
    std::cout << "open on: http://127.0.0.1" << ":" << port << std::endl;

    std::list<banker::networker::stream_socket> clients;
    while (true)
    {
        while (server.touch())
        {
            banker::networker::stream_socket new_client = server.accept();
            if (new_client.is_valid())
            {
                if (log) std::cout << "[SERVER] new client connected. client("<<new_client.raw_socket().to_fd()<<")" << std::endl;
                clients.push_back(std::move(new_client));
            }
        }

        for (auto it = clients.begin(); it != clients.end(); )
        {
            auto& client = *it;
            banker::networker::stream_socket_core::request_result result;
            auto r = client.tick(true, true, &result);
            auto& buf = client.receive();
            auto pos = std::search(buf.begin(), buf.end(), "\r\n\r\n", "\r\n\r\n"+4);
            if (pos != buf.end())
            {
                std::string request(buf.begin(), pos+4);
                buf.erase(buf.begin(), pos+4);
                if (log) std::cout << "[SERVER] client("<<client.raw_socket().to_fd()<<") :" << request << std::endl;
                std::string response = http_process(request);
                client.enqueue({response.begin(), response.end()});
            }
            if (result != banker::networker::stream_socket_core::request_result::ok)
            {
                if (log) std::cout << "[SERVER] client("<<client.raw_socket().to_fd()<<") disconnected." << std::endl;
                it = clients.erase(it);
                continue;
            }
            ++it;
        }
    }
}

#endif //BANKER_HTTP_SERVER_HPP