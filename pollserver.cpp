#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <poll.h>
#include <memory>
using namespace std;

#define MAX_CLIENTS  1
#define MAX_BUFF_SIZE 2048

class Server
{
private:
    int m_port;
    int m_clientNum;
    int m_bytesRead;
    int m_bytesWritten;
    char m_msg[MAX_BUFF_SIZE];
    sockaddr_in m_servAddr;
    int m_serverSocket;
    struct timeval m_start1;
    struct timeval m_end1;
    struct pollfd m_pollfds[MAX_CLIENTS + 1];
public:
    Server(const char* port)
        : m_port{atoi(port)}, m_clientNum{0}
    {
        bzero((char*)&m_servAddr, sizeof(m_servAddr));
        m_servAddr.sin_family = AF_INET;
        m_servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_servAddr.sin_port = htons(m_port);

        // initialize all file descriptor to zero
        for(int i=0; i<=MAX_CLIENTS; ++i)
        {
            m_pollfds[i].fd = 0;
            m_pollfds[i].events = 0;
            m_pollfds[i].revents = 0;
        }
    }

    void serverListen()
    {
        m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_serverSocket < 0)
        {
            cerr << "Error establishing the server socket" << endl;
            exit(0);
        }

        const int enable = 1;
        if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
        }

        int bindStatus = bind(
            m_serverSocket,
            (struct sockaddr*) &m_servAddr,
            sizeof(m_servAddr)
        );

        // index 0 for server file descriptor
        m_pollfds[0].fd = m_serverSocket;
        // listen input event
        m_pollfds[0].events = POLLIN;

        //listen for up to MAX_CLIENTS requests at a time
        listen(m_serverSocket, MAX_CLIENTS);
    }
    
    void mainLoop()
    {
        //lets keep track of the session time
        gettimeofday(&m_start1, NULL);
        //also keep track of the amount of data sent as well

        bool maxclientReached = false;
        while(1)
        {
            //receive a request from client using accept
            //we need a new address to connect with the client
            sockaddr_in newSockAddr;
            socklen_t newSockAddrSize = sizeof(newSockAddr);

            // let poll return immediately
            int numEvent = poll(m_pollfds, m_clientNum + 1, 0);
            if(numEvent == 0)
            {
                continue;
            }
            else if(numEvent < 0)
            {
                perror("Poll error");
                exit(1);
            }

            
            if(m_pollfds[0].revents & POLLIN)
            {

                int newSd = accept(m_serverSocket, (sockaddr *)&newSockAddr, &newSockAddrSize);
                if(newSd < 0)
                {
                    cerr << "Error accepting request from client!" << endl;
                    exit(1);
                }
                if(m_clientNum == MAX_CLIENTS)
                {
                    cerr << "Error: Maximum number of client reached, no new client being connected\n";
                    close(newSd);
                }

                // find for empty/unused file descriptor
                for (int i = 1; i <= MAX_CLIENTS; i++)
                {
                    if (m_pollfds[i].fd == 0)
                    {
                        m_pollfds[i].fd = newSd;
                        m_pollfds[i].events = POLLIN;
                        m_clientNum++;
                        break;
                    }
                }

                cout << "A new client connected!\n";
            }

            // loop through all client file descriptors
            // i = 1, since client fd start from index 1
            for(int i=1; i <= MAX_CLIENTS; ++i)
            {
                if (m_pollfds[i].fd > 0 && m_pollfds[i].revents & POLLIN)
                {
                    char buf[MAX_BUFF_SIZE];
                    memset(&buf, 0, sizeof(buf));

                    // read until MAX_BUFF_SIZE - 1, because we want to append the null ourself
                    int bufSize = read(m_pollfds[i].fd, buf, MAX_BUFF_SIZE - 1);

                    if (bufSize == -1)
                    {
                        // error while reading (probably client disconnected)
                        m_pollfds[i].fd = 0;
                        m_pollfds[i].events = 0;
                        m_pollfds[i].revents = 0;
                        m_clientNum--;
                        cout << "Error while reading from client\n";
                        continue;
                    }
                    else if (bufSize == 0)
                    {
                        cout << "FIN received";
                        // client send FIN
                        m_pollfds[i].fd = 0;
                        m_pollfds[i].events = 0;
                        m_pollfds[i].revents = 0;
                        m_clientNum--;
                        cout << "Client end\n";
                        continue;
                    }

                    m_bytesRead += bufSize;

                    if(!strcmp(buf, "exit"))
                    {
                        cout << "Client has quit the session" << endl;
                        m_pollfds[i].fd = 0;
                        m_pollfds[i].events = 0;
                        m_pollfds[i].revents = 0;
                        m_clientNum--;
                        cout << "Client end\n";
                        continue;
                    }
                    else if(!strcmp(buf, "Klovinki Purnama"))
                    {
                        strncpy(buf, "Zaky Hermawan", MAX_BUFF_SIZE);
                    }
                    else if(!strcmp(buf, "13222119"))
                    {
                        strncpy(buf, "13220022", MAX_BUFF_SIZE);
                    }
                    else
                    {
                        strncpy(buf, "Pesan tidak dikenali", MAX_BUFF_SIZE);
                    }

                    m_bytesWritten += send(m_pollfds[i].fd, (char*)&buf, strlen(buf), 0);
                    if(!strcmp(buf, "exit"))
                    {
                        //send to the client that server has closed the connection
                        break;
                    }
                }
            }
        }
    }

    ~Server()
    {
        //we need to close the socket descriptors after we're all done
        gettimeofday(&m_end1, NULL);
        for(int i=0; i<=MAX_CLIENTS; ++i)
        {
            if(m_pollfds[i].fd != 0)
            {
                close(m_pollfds[i].fd);
                m_pollfds[i].fd = 0;
                m_pollfds[i].events = 0;
                m_pollfds[i].revents = 0;
            }
        }
        m_clientNum = 0;
        close(m_serverSocket);
        cout << "********Session********" << endl;
        cout << "Bytes written: " << m_bytesWritten << " Bytes read: " << m_bytesRead << endl;
        cout << "Elapsed time: " << (m_end1.tv_sec - m_start1.tv_sec) << " secs" << endl;
        cout << "Connection closed..." << endl;
    }
};

unique_ptr<Server> server;

// Our universal signal handler
static void signal_handler(int signum)
{
    // manually delete smart pointer to call destructor
    server = nullptr;
    exit(signum);
}

int main(int argc, char *argv[])
{
    //for the server, we only need to specify a port number
    if(argc != 2)
    {
        cerr << "Usage: port" << endl;
        exit(0);
    }
    signal(SIGINT, signal_handler);
    server = std::make_unique<Server>(argv[1]);

    server->serverListen();
    server->mainLoop();

    return 0;   
}
