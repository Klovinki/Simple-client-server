# Simple-client-server

How to run
=====
Server:
`g++ server.cpp -o server`

`./server 1234`

Client:
`python client.py localhost 1234`

You can change `1234` with any port

Concurrent Server
=====
To use concurrent server (server that can handle multiple client), do:

Server:
`g++ pollserver.cpp -o pollserver`

`./pollserver 1234`
