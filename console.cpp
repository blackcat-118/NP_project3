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

int main() {

string requests = getenv("QUERY_STRING");

size_t indx1 = 0;
size_t indx2 = 0;
for (int i = 0; i < 5; i++) {
    indx1 = requests.find_first_of("=", indx1+1);
    indx2 = requests.find_first_of("&", indx2+1);
    cout << indx1 << " " << indx2 << endl;
    session_list[i].host = requests.substr(indx1+1, indx2-indx1-1);    
    indx1 = requests.find_first_of("=", indx1+1);
    indx2 = requests.find_first_of("&", indx2+1);
    cout << indx1 << " " << indx2 << endl;
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
    cout << " <th scope=\"col\">" << session_list[i].host << ":" << session_list[i].port << ":" << session_list[i].file << "</th>" << endl;
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

