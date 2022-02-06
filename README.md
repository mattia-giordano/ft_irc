# ft_irc
My own IRC server in C++, fully compatible with [KVIrc](http://www.kvirc.net/) client

## What's IRC
**Internet Relay Chat (IRC)** is a text-based chat (instant messaging) system. IRC is designed for group communication in discussion forums, called channels, but also allows one-on-one communication via private messages as well as chat and data transfer, including file sharing.

## How to run it
In the project directory type `make` and then `./irc_server (port) (password)`  
The standard port for IRC protocol il **6667**  
The password you choose will be needed by the client to connect

## Connecting to it from KVIrc
From the server list select 127.0.0.1 from *Standalone Servers*  
Go to *Advanced>Server Details an Configuration>Advanced* and disable **SASL Authentication** (our server does not support it)  
Go to *Advanced>Identity* and set a username of your choice and **set the correct password**  
Click *Connect* and then you can use the client interface to play with it!

## Server features
* Handling multiple clients at the same time
* Authenticate, set a nickname, a username, join a channel, send and receive private messages from the client
* Basically almost every standard command from [IRC RFC](https://datatracker.ietf.org/doc/html/rfc1459)
* File transfer

## Bot
We also added a bot that insults you or someone for you whit typical roman expression such as *MORTACCI TUA*  
To run it:
* in the project directory type `make bot`
* to start it type `./insultaBOT 127.0.0.1 (port) (password)` with `port` and `password` being the one you chose at the beginning  
if everything goes as expected it will output *Bot correctly connected!* on the prompt
* join *insultaBOT* channel from the client, type `COMANDI` for available commands and have fun!  
***
  
This project was devoloped by me and @ale3000ale