#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <boost/asio.hpp>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>

using namespace std;

string panel();
string console_temp(string);
void console(boost::asio::ip::tcp::socket&);

string do_replace(string s, int length) {
    string s1;
    for (int i = 0; i < (int)s.length(); i++){
        if (s[i] == '\n')
            s1 += "&NewLine;";
        else if (s[i] == '\r') {
            if (i+1 < (int)s.length() && s[i+1] == '\n') {
                s1 += "&NewLine;";
                i++;
            }
        }
        else if (s[i] == ' ') {
            s1 += "&nbsp;";
        }
        else if (s[i] == '\t') {
            s1 += "&Tab;";
        }
        else if (s[i] == '\"') {
            s1 += "&quot;";
        }
        else if (s[i] == '\'') {
            s1 += "&apos;";
        }
        else if (s[i] == '&') {
            s1 += "&amp;";
        }
        else if (s[i] == '>') {
            s1 += "&gt;";
        }
        else if (s[i] == '<') {
            s1 += "&lt;";
        }
        else {
            s1 += s[i];
        }
    }
    s1 += "\0";

    return s1;
}

class session : public std::enable_shared_from_this<session>{
public:
  session(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)){}

  void start() {
    env_vars.push_back(pair<string, string>("REQUEST_METHOD", ""));
    env_vars.push_back(pair<string, string>("REQUEST_URI", ""));
    env_vars.push_back(pair<string, string>("QUERY_STRING", ""));
    env_vars.push_back(pair<string, string>("SERVER_PROTOCOL", ""));
    env_vars.push_back(pair<string, string>("HTTP_HOST", ""));
    env_vars.push_back(pair<string, string>("SERVER_ADDR", ""));
    env_vars.push_back(pair<string, string>("SERVER_PORT", ""));
    env_vars.push_back(pair<string, string>("REMOTE_ADDR", ""));
    env_vars.push_back(pair<string, string>("REMOTE_PORT", ""));
    do_read();
  }

private:
  void do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec) {
                Parser(string(data_));
                env_vars[5].second = socket_.local_endpoint().address().to_string();
                env_vars[6].second = to_string(socket_.local_endpoint().port());
                env_vars[7].second = socket_.remote_endpoint().address().to_string();
                env_vars[8].second = to_string(socket_.remote_endpoint().port());
                
                if (service_name == "panel.cgi") {
                    string out = env_vars[3].second + " 200 OK\r\n";
                    do_write(out, out.length());
                    out = panel();
                    do_write(out, out.length());
                }
                else if (service_name == "console.cgi") {
                    string out = env_vars[3].second + " 200 OK\r\n";
                    do_write(out, out.length());
                    out = console_temp(env_vars[2].second);
                    do_write(out, out.length());
                    console(socket_);
                }
                else {
                    memset(data_, '\0', max_length);
                    do_read();
                }

            }
            else {
                socket_.close();
            }
            //do_write(length);
        });
    }

    void do_write(string output, std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(output, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                //do_read();
            }
            });
    }
    
    void Parser(string http_req) {

        vector<string> parsed_req;

        boost::split(parsed_req, http_req, boost::is_any_of(" \r\n"), boost::token_compress_on);
        for (unsigned int i = 0; i < parsed_req.size(); i++) {
        }
        env_vars[0].second = parsed_req[0];
        env_vars[1].second = parsed_req[1];
        std::size_t indx = parsed_req[1].find_first_of('?');
        string query = "";
        if (indx != string::npos) {
            service_name = parsed_req[1].substr(1, indx-1);
            query = parsed_req[1].substr(indx+1);
        }
        else {
            service_name = parsed_req[1].substr(1);
        }
        env_vars[2].second = query;
        env_vars[3].second = parsed_req[2];
        env_vars[4].second = parsed_req[4];
        
        return;
    }

    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    string service_name;
    vector<pair<string, string>> env_vars;
};

class mysession {
public:
    string host = "";
    string port = "";
    string file = "";
} session_list[5];

class client {
public:
    client(int sid, string host, string port, string file, boost::asio::io_context& io_context, boost::asio::ip::tcp::socket& session_socket_)
    : sid(sid), host(host), port(port), file(file), resolver_(io_context), socket_(io_context), session_socket_(session_socket_){}

    void start() {
        boost::asio::ip::tcp::resolver::query query_(host, port);
        resolver_.async_resolve(query_, 
        [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type result) {
            endpoint_ = result;
            if (!ec) {
                do_connect();                
            }
            else {
                socket_.close();
            }
        });
        
    }
    
private:
    void do_connect() {
        socket_.async_connect(*(endpoint_.begin()), 
        [this](boost::system::error_code ec) {
            if (!ec) {
                if (file != "") {
                    file = ".\\test_case\\" + file;
                    fin = ifstream(file.data());
                }
                memset(data_, '\0', 20480);
                do_read();
            }
            else {
                fin.close();
                socket_.close();
            }
        });
    }
    void do_read() {
        socket_.async_read_some(boost::asio::buffer(data_, max_length), 
        [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                string out = "<script>document.getElementById(\"s" + to_string(sid) + "\").innerHTML += \'<b>" + do_replace(data_, length) + "</b>\';</script>\n";
                do_session_write(out);
                if (strstr(data_, "% ") != NULL) {
                    memset(data_, '\0', 20480);
                    do_write();
                }
                else {
                    memset(data_, '\0', 20480);
                    do_read();
                }
            }
        });
    }
    void do_write() {
        char cmd[20480] = "";
        if (fin.getline(cmd, 20480)) {
            cmd[strlen(cmd)] = '\n';
            string out = "<script>document.getElementById(\"s" + to_string(sid) + "\").innerHTML += \'" + do_replace(cmd, 0) + "\';</script>\n";
            do_session_write(out);
            boost::asio::async_write(socket_, boost::asio::buffer(cmd, strlen(cmd)), 
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    do_read();
                }
            });
        }
        else {
            fin.close();
            socket_.close();
        }
    }
    void do_session_write(string out) {
        boost::asio::async_write(session_socket_, boost::asio::buffer(out.data(), out.length()),
        [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {

            }
        });
    }

    int sid;
    string host = "";
    string port = "";
    string file = "";
    ifstream fin;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::results_type endpoint_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::socket& session_socket_;
    enum { max_length = 20480 };
	char data_[max_length];
};


class server
{
public:
  server(boost::asio::io_context& io_context, short port)
   : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        do_accept();
    }

private:
  void do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept();
        });
  }

  boost::asio::ip::tcp::acceptor acceptor_;
};


string panel() {
    string response = "";
    int N_SERVERS = 5;

    string FORM_METHOD = "GET";
    string FORM_ACTION = "console.cgi";

    string testcase_menu = "";
    for (int i = 1; i < 6; i++) {
        testcase_menu += "<option value=\"t" + to_string(i) + ".txt" + "\">" + "t" + to_string(i) + ".txt" + "</option>";
    }
    string domain_name = ".cs.nycu.edu.tw";
    string host_menu = "";
    for (int i = 1; i < 13; i++) {
        host_menu += "<option value=\"nplinux" + to_string(i) + domain_name + "\">" + "nplinux" + to_string(i) + "</option>";
    }

    response += "Content-type: text/html\r\n\r\n";
    response += "<!DOCTYPE html>";
    response += "<html lang=\"en\">";
    response += "<head>";
    response += "<title>NP Project 3 Panel</title>";
    response += "<link";
    response += " rel=\"stylesheet\"";
    response += " href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"";
    response += " integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"";
    response += " crossorigin=\"anonymous\"";
    response += "/>";
    response += "<link";
    response += " href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"";
    response += " rel=\"stylesheet\"";
    response += "/>";
    response += "<link";
    response += " rel=\"icon\"";
    response += " type=\"image/png\"";
    response += " href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"";
    response += "/>";
    response += "<style>";
    response += "* {";
    response += "    font-family: \'Source Code Pro\', monospace;";
    response += "}";
    response += "</style>";
    response += "</head>";
    response += "<body class=\"bg-secondary pt-5\">";

    response += "<form action=\"" + FORM_ACTION + "\" method=\"" + FORM_METHOD + "\">";
    response += "<table class=\"table mx-auto bg-light\" style=\"width: inherit\">";
    response += "<thead class=\"thead-dark\">";
    response += "<tr>";
    response += "<th scope=\"col\">#</th>";
    response += "<th scope=\"col\">Host</th>";
    response += "<th scope=\"col\">Port</th>";
    response += "<th scope=\"col\">Input File</th>";
    response += "</tr>";
    response += "</thead>";
    response += "<tbody>";

    for (int i = 0; i < N_SERVERS; i++) {
        response += "<tr>";
            response += "<th scope=\"row\" class=\"align-middle\">Session " + to_string(i+1) + "</th>";
            response += "<td>";
            response += "<div class=\"input-group\">";
                response += "<select name=\"h" + to_string(i) + "\" class=\"custom-select\">";
                response += "<option></option>" + host_menu;
                response += "</select>";
                response += "<div class=\"input-group-append\">";
                response += "<span class=\"input-group-text\">.cs.nycu.edu.tw</span>";
                response += "</div>";
            response += "</div>";
            response += "</td>";
            response += "<td>";
            response += "<input name=\"p" + to_string(i) + "\" type=\"text\" class=\"form-control\" size=\"5\" />";
            response += "</td>";
            response += "<td>";
            response += "<select name=\"f" + to_string(i) + "\" class=\"custom-select\">";
                response += "<option></option>";
                response += "" + testcase_menu + "";
            response += "</select>";
            response += "</td>";
        response += "</tr>";
    }
        response += "<tr>";
                response += "<td colspan=\"3\"></td>";
                response += "<td>";
                response += "<button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>";
                response += "</td>";
            response += "</tr>";
            response += "</tbody>";
        response += "</table>";
        response += "</form>";
    response += "</body>";
    response += "</html>";

    return response;
}

string console_temp(string requests) {
    std::size_t indx1 = 0;
    std::size_t indx2 = 0;

    for (int i = 0; i < 5; i++) {
        indx1 = requests.find_first_of("=", indx1+1);
        indx2 = requests.find_first_of("&", indx2+1);
        session_list[i].host = requests.substr(indx1+1, indx2-indx1-1);    
        indx1 = requests.find_first_of("=", indx1+1);
        indx2 = requests.find_first_of("&", indx2+1);
        session_list[i].port = requests.substr(indx1+1, indx2-indx1-1);
        indx1 = requests.find_first_of("=", indx1+1);
        indx2 = requests.find_first_of("&", indx2+1);
        if (indx2 == string::npos) {
            session_list[i].file = requests.substr(indx1+1);
            break;
        }
        else {
            session_list[i].file = requests.substr(indx1+1, indx2-indx1-1);
        }
    }

    //h0=nplinux7.cs.nycu.edu.tw&p0=12345&f0=t1.txt&h1=nplinux3.cs.nycu.edu.tw&p1=12344&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=
    string response = "";
    response += "Content-type: text/html\r\n\r\n";
    response += "<!DOCTYPE html>";
    response += "<html lang=\"en\">";
    response += "<head>";
    response += "<meta charset=\"UTF-8\" />";
    response += "<title>NP Project 3 Sample Console</title>";
    response += "<link";
    response += "  rel=\"stylesheet\"";
    response += "  href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"";
    response += "  integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"";
    response += "  crossorigin=\"anonymous\"";
    response += "/>";
    response += "<link";
    response += "  href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"";
    response += "  rel=\"stylesheet\"";
    response += "/>";
    response += "<link";
    response += "  rel=\"icon\"";
    response += "  type=\"image/png\"";
    response += "  href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"";
    response += "/>";
    response += "<style>";
    response += "  * {";
    response += "font-family: \'Source Code Pro\', monospace;";
    response += "font-size: 1rem !important;";
    response += "  }";
    response += "  body {";
    response += "background-color: #212529;";
    response += "  }";
    response += "  pre {";
    response += "color: #cccccc;";
    response += "  }";
    response += "  b {";
    response += "color: #01b468;";
    response += "  }";
    response += "</style>";
    response += "  </head>";
    response += "  <body>";
    response += "<table class=\"table table-dark table-bordered\">";
    response += "  <thead>";
    response += "<tr>";
    for (int i = 0; i < 5; i++) {
        if (session_list[i].host == "") {
            continue;
        }
        response += " <th scope=\"col\">" + session_list[i].host + ":" + session_list[i].port + "</th>";
    }
    response += "</tr>";
    response += "  </thead>";
    response += "  <tbody>";
    response += "<tr>";
    for (int i = 0; i < 5; i++) {
        if (session_list[i].host == "") {
            continue;
        }
        response += "  <td><pre id=\"s" + to_string(i) + "\" class=\"mb-0\"></pre></td>";
    }
    response += "</tr>";
    response += "  </tbody>";
    response += "</table>";
    response += "  </body>";
    response += "</html>";
    
    return response;
}

void console(boost::asio::ip::tcp::socket& socket_) {
    boost::asio::io_context io_context;
    for (int i = 0; i < 5; i++) {
        if (session_list[i].host == "") {
            continue;
        }
        client* c = new client(i, session_list[i].host, session_list[i].port, session_list[i].file, io_context, socket_);
        c->start();
    }
    io_context.run();
    return;
}



int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_cgi_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}