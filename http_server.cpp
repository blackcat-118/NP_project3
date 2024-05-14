#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include <cstddef>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <boost/asio.hpp>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>

using boost::asio::ip::tcp;
using namespace std;

vector<pair<string, string>> env_vars;
string service_name;
void Parser(string http_req) {
    vector<string> parsed_req;

    boost::split(parsed_req, http_req, boost::is_any_of(" \r\n"), boost::token_compress_on);

    env_vars[0].second = parsed_req[0];
    env_vars[1].second = parsed_req[1];
    size_t indx = parsed_req[1].find_first_of('?');
    string query = "";
    if (indx != string::npos) {
      service_name = parsed_req[1].substr(1, indx-1);
      query = parsed_req[1].substr(indx+1);
    }
    else {
      service_name = parsed_req[1].substr(1);
      //cout << service_name << endl;
    }
    env_vars[2].second = query;
    env_vars[3].second = parsed_req[2];
    env_vars[4].second = parsed_req[4];
    
    return;
}

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            Parser(string(data_));
            env_vars[5].second = socket_.local_endpoint().address().to_string();
            env_vars[6].second = to_string(socket_.local_endpoint().port());
            env_vars[7].second = socket_.remote_endpoint().address().to_string();
            env_vars[8].second = to_string(socket_.remote_endpoint().port());
            
            int childpid = fork();
            if (childpid == 0) {
              for (int i = 0; i < 9; i++) {
                setenv(env_vars[i].first.data(), env_vars[i].second.data(), 1);
              }
              dup2(socket_.native_handle(), STDIN_FILENO);
              dup2(socket_.native_handle(), STDOUT_FILENO);
              dup2(socket_.native_handle(), STDERR_FILENO);

              cout << env_vars[3].second << " 200 OK\r\n" << flush;

              char sn[20] = {};
              strcpy(sn, service_name.data());
              char* argv[5] = {sn, NULL};

              if (execv(service_name.data(), argv) < 0) {
                cerr << "error: " << strerror(errno) << endl;
                exit(1);
              }
              exit(0);

            }
            else {
              socket_.close();
              int wstatus;
              waitpid(childpid, &wstatus, 0);
            }
            //do_write(length);
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            do_read();
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
          std::cerr << "Usage: async_tcp_echo_server <port>\n";
          return 1;
        }

        // set env
        env_vars.push_back(pair<string, string>("REQUEST_METHOD", ""));
        env_vars.push_back(pair<string, string>("REQUEST_URI", ""));
        env_vars.push_back(pair<string, string>("QUERY_STRING", ""));
        env_vars.push_back(pair<string, string>("SERVER_PROTOCOL", ""));
        env_vars.push_back(pair<string, string>("HTTP_HOST", ""));
        env_vars.push_back(pair<string, string>("SERVER_ADDR", ""));
        env_vars.push_back(pair<string, string>("SERVER_PORT", ""));
        env_vars.push_back(pair<string, string>("REMOTE_ADDR", ""));
        env_vars.push_back(pair<string, string>("REMOTE_PORT", ""));

        boost::asio::io_context io_context;

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    }
    catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}