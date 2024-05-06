#include <iostream>
#include <string>
#include <cstdlib>
#include <boost/asio.hpp>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>

using boost::asio::ip::tcp;
using namespace std;

class mysession {
public:
    string host = "";
    string port = "";
    string file = "";
} session_list[5];

class session 
    : public std::enable_shared_from_this<session>
{    
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)){}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    do_write(length);
                }
            });
    }
    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    do_read();
                }
            });
    }
    tcp::socket socket_;
    enum {max_length = 1024};
    char data_[max_length];
    
};

class server {
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
            do_accept();
        }
private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket))->start();
                }
                do_accept();
            });
    }
    tcp::acceptor acceptor_;
};

int main() {

string requests = getenv("QUERY_STRING");

size_t indx1 = 0;
size_t indx2 = 0;
for (int i = 0; i < 5; i++) {
    indx1 = requests.find_first_of('=', indx1+1);
    indx2 = requests.find_first_of('&', indx2+1);
    session_list[i].host = requests.substr(indx1+1, indx2-indx1-1);    
    indx1 = requests.find_first_of('=', indx1+1);
    indx2 = requests.find_first_of('&', indx2+1);
    session_list[i].port = requests.substr(indx1+1, indx2-indx1-1);
    indx1 = requests.find_first_of('=', indx1+1);
    indx2 = requests.find_first_of('&', indx2+1);
    if (indx2 != string::npos) {
        session_list[i].file = requests.substr(indx1+1);
        break;
    }
    else {
        session_list[i].file = requests.substr(indx1+1, indx2-indx1-1);
    }
}

cout << "Content-type: text/html\r\n\r\n" << flush;
cout << "<!DOCTYPE html>" << endl;
cout << "<html lang=\"en\">" << endl;
cout << "<head>" << endl;
cout << "<meta charset=\"UTF-8\" />" << endl;
cout << "<title>NP Project 3 Sample Console</title>" << endl;
cout << "<link" << endl;
cout << "  rel=\"stylesheet\"" << endl;
cout << "  href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"" << endl;
cout << "  integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"" << endl;
cout << "  crossorigin=\"anonymous\"" << endl;
cout << "/>" << endl;
cout << "<link" << endl;
cout << "  href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"" << endl;
cout << "  rel=\"stylesheet\"" << endl;
cout << "/>" << endl;
cout << "<link" << endl;
cout << "  rel=\"icon\"" << endl;
cout << "  type=\"image/png\"" << endl;
cout << "  href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"" << endl;
cout << "/>" << endl;
cout << "<style>" << endl;
cout << "  * {" << endl;
cout << "font-family: \'Source Code Pro\', monospace;" << endl;
cout << "font-size: 1rem !important;" << endl;
cout << "  }" << endl;
cout << "  body {" << endl;
cout << "background-color: #212529;" << endl;
cout << "  }" << endl;
cout << "  pre {" << endl;
cout << "color: #cccccc;" << endl;
cout << "  }" << endl;
cout << "  b {" << endl;
cout << "color: #01b468;" << endl;
cout << "  }" << endl;
cout << "</style>" << endl;
cout << "  </head>" << endl;
cout << "  <body>" << endl;
cout << "<table class=\"table table-dark table-bordered\">" << endl;
cout << "  <thead>" << endl;
cout << "<tr>" << endl;
for (int i = 0; i < 5; i++) {
    if (session_list[i].host == "") {
        continue;
    }
    cout << " <th scope=\"col\">" << session_list[i].host << ":" << session_list[i].port << "</th>" << endl;
}
// cout << "  <th scope=\"col\">nplinux1.cs.nycu.edu.tw:1234</th>" << endl;
// cout << "  <th scope=\"col\">nplinux2.cs.nycu.edu.tw:5678</th>" << endl;
cout << "</tr>" << endl;
cout << "  </thead>" << endl;
cout << "  <tbody>" << endl;
cout << "<tr>" << endl;
for (int i = 0; i < 5; i++) {
    if (session_list[i].host == "") {
        continue;
    }
    cout << "  <td><pre id=\"s" << i << "\" class=\"mb-0\"></pre></td>" << endl;
}
// cout << "  <td><pre id=\"s0\" class=\"mb-0\"></pre></td>" << endl;
// cout << "  <td><pre id=\"s1\" class=\"mb-0\"></pre></td>" << endl;
cout << "</tr>" << endl;
cout << "  </tbody>" << endl;
cout << "</table>" << endl;
cout << "  </body>" << endl;
cout << "</html>" << endl;

return 0;
}

