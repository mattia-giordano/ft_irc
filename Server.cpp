
#include "Server.hpp"
#include "Channel.hpp"

Server::Server(std::string port, std::string password) : _port(port), _password(password), _handler(*this)
{
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai;

	std::time_t result = std::time(nullptr);
    this->_dateTimeCreated = std::asctime(std::localtime(&result));
    // Get us a socket and bind it
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(nullptr, this->_port.c_str(), &hints, &ai)))
		throw std::runtime_error(gai_strerror(rv));
    
	if ((this->_socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
		throw std::runtime_error(strerror(errno));

	// Lose the pesky "address already in use" error message
    setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(this->_socket_fd, ai->ai_addr, ai->ai_addrlen) < 0)
	{
		close(this->_socket_fd); // da capire se devo fare la delete nel catch o meno
		freeaddrinfo(ai);
		throw std::runtime_error(strerror(errno));
	}

	freeaddrinfo(ai); // All done with this

	_load_motd(MOTD_PATH);
}

Server::~Server()
{
	close(this->_socket_fd);
	for (u_int i = 0; i < this->_users.size(); i++)
		this->_deleteUser(i);
}

void Server::run()
{
	if ((listen(this->_socket_fd, BACKLOG)) < 0)
		throw std::runtime_error(strerror(errno));
	
	// Add server fd to set
	_addFd(this->_socket_fd);

	while (1)
	{
		if (poll(this->_pfds.data(), this->_pfds.size(), -1) < 0)
			throw std::runtime_error(strerror(errno));
		for(u_int i = 0; i < this->_pfds.size(); i++)
		{
			// Check if someone's ready to read
			if (this->_pfds[i].revents & POLLIN)
			{
				if (this->_pfds[i].fd == this->_socket_fd)
				{
					// If listener is ready to read, handle new connection
					_addUser();
				}
				else
				{
					// If not the listener, we're just a regular client
					char buf[512];
					memset(buf, 0, sizeof(buf));
					int nbytes = recv(this->_pfds[i].fd, buf, sizeof(buf), 0);
					if (nbytes <= 0) // Got error or connection closed by client
					{
						if (nbytes) // if nbytes != 0 an error occured
							 perror("recv");
						else
							_deleteUser(i);
					}
					else
					{
						User &curr = *this->_users[i - 1];
						curr.buffer() += buf;
						if (curr.buffer().find(CRLF) != std::string::npos)
							_exec_cmd(curr);
					}
				}
			}
		}
	}
}

void	Server::_load_motd(const char *file)
{
	std::fstream my_file;
	my_file.open(file, std::ios::in);
	if (!my_file)
		return ;
	
	my_file >> std::noskipws;

	std::string line;
	char ch;

	do {
		my_file >> ch;
		if (ch == '\n')
		{
			this->_motd.push_back(line);
			line.clear();
		}
		else
			line += ch;
	} while (!my_file.eof());
	my_file.close();
}

void Server::_addUser()
{
	struct sockaddr_storage clientaddr; // Client address
	socklen_t addrlen = sizeof(clientaddr);
	int new_fd;
	if ((new_fd = accept(this->_socket_fd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
		return (perror("accept"));
	_addFd(new_fd);
	char remoteIP[INET6_ADDRSTRLEN];
	struct sockaddr *casted_addr = (struct sockaddr*)&clientaddr;
	if (casted_addr->sa_family == AF_INET)
    	inet_ntop(AF_INET, &(((struct sockaddr_in*)casted_addr)->sin_addr), remoteIP, INET_ADDRSTRLEN);
    else
		inet_ntop(AF_INET6, &(((struct sockaddr_in6*)casted_addr)->sin6_addr), remoteIP, INET6_ADDRSTRLEN);
	this->_users.push_back(new User(new_fd, remoteIP));
}

void Server::_deleteUser(int index)
{
	close(this->_pfds[index].fd); // closing client's fd
	this->_pfds.erase(this->_pfds.begin() + index);
	delete(this->_users[index - 1]);
	this->_users.erase(this->_users.begin() + index - 1);
}

void Server::deleteUser(std::string nick)
{
	for (u_int i = 0; i < this->_users.size(); i++)
	{
		if (*this->_users[i] == nick)
			return (_deleteUser(i + 1));
	}
}

void Server::_addFd(int new_fd)
{
	struct pollfd tmp;

	fcntl(new_fd, F_SETFL, O_NONBLOCK);
	tmp.fd = new_fd;
	tmp.events = POLLIN; // Report ready to read on incoming connection
	this->_pfds.push_back(tmp);
}

bool Server::checkPass(std::string& pass)
{
	return (pass == this->_password);
}

std::vector<User*> const & Server::getUserList() const
{
	return (this->_users);
}

void Server::_exec_cmd(User& executor)
{
	std::string& buffer = executor.buffer();
	int pos = buffer.find(CRLF);
	do
	{
		this->_handler.handle(buffer.substr(0, pos), executor);
		buffer.erase(0, pos + 2);
		pos = buffer.find(CRLF);
	} while (pos != -1);
}

bool	Server::exist_channel(std::string name) const
{
	if(_channels.find(name) != _channels.end())
		return true;
	else
		return false;
}

Channel			&Server::get_channel(std::string name)
{
	return _channels[name];
}

bool			Server::add_channel(Channel ch)
{
	if(_channels.find(ch.getName()) == _channels.end())
	{
		_channels.insert ( std::pair<std::string ,Channel>(ch.getName(), ch) );
		return true;
	}
	return false;
}

void	Server::send_msg(std::string& msg, User const & target) const
{
	if (send(target.getSocket(), msg.c_str(), msg.length(), 0) < 0)
        perror("send");
}

int		Server::send_msg(std::string& msg, std::string target, User const & owner) 
{
	if (exist_channel(target))
	{
		Channel& tmp_chan = get_channel(target);
		if (tmp_chan.canSendMsg(owner))
			tmp_chan.sendAll(msg, owner.getNick());
		else
			return (0);
	}
	else
		return (ERR_NOSUCHNICK);
	return (0);
}

int		Server::send_msg(std::string& msg, std::string target) const
{
	u_int i = 0;
	
	while (i < _users.size())
	{
		if (*_users[i] == target)
		{
			if (_users[i]->isAway())
				return (RPL_AWAY);
			send_msg(msg, *_users[i]);
			break ;
		}
		i++;
	}
	if (i == _users.size())
		return (ERR_NOSUCHNICK);
	return 0;
}

CommandHandler	Server::getHandler() const
{
	return (_handler);
}

void			Server::sendAllChans(std::string msg, User& sender)
{
	chan_it it = this->_channels.cbegin();
	chan_it tmp;
	while (it != this->_channels.cend())
	{
		std::string ch_name = (*it).first;
		send_msg(msg, ch_name, sender);
		_channels[ch_name].removeUser(sender);
		++it;
	}
	it = this->_channels.cbegin();
	while (it != this->_channels.cend())
	{
		tmp = it;
		tmp++;
		std::string ch_name = (*it).first;
		if(_channels[ch_name].empty())
			this->removeChannel(ch_name);
		it = tmp;
	}
}

bool			Server::exist_user(std::string name) const
{
	size_t i = 0;
	for (;i< _users.size(); i++)
		if (*_users[i] == name)
			return true;
	return false;

}

User const 		&Server::getUser(std::string user) const
{
	size_t i  = 0;
	for (; i < _users.size(); i++)
	{
		if (*_users[i] == user)
			return *_users[i];
	}
	return *_users[i];
}

std::string		Server::getDateTimeCreated() const
{ return (this->_dateTimeCreated); }

void			Server::removeChannel(std::string name)
{
	_channels.erase(name);
}

std::map<std::string, Channel> const &Server::getchannelList() const
{
	return this->_channels;
}

std::vector<std::string> const &Server::getMotd() const
{
	return this->_motd;
}