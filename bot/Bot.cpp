
#include "Bot.hpp"

Bot::Bot(std::string server_ip, std::string port, std::string password) : _server_ip(server_ip), _port(port), _password(password)
{
	struct addrinfo hints, *ai;
	int rv, yes=1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server_ip.c_str(), port.c_str(), &hints, &ai)) != 0)
		throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(rv)));
	if ((this->_socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
		throw std::runtime_error("socket: " + std::string(strerror(errno)));
	setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (connect(this->_socket_fd, ai->ai_addr, ai->ai_addrlen) == -1)
	{
		close(this->_socket_fd);
		throw std::runtime_error("connect: " + std::string(strerror(errno)));
	}
	freeaddrinfo(ai);

	_load_insults(INSULTS_PATH);
}

Bot::~Bot() {}

void	Bot::_load_insults(const char *file)
{
	std::fstream my_file;
	my_file.open(file, std::ios::in);
	if (!my_file)
		throw std::runtime_error("open: No input file for insults");
	
	my_file >> std::noskipws;
	
	std::string line;
	char ch;

	do {
		my_file >> ch;
		if (ch == '\n')
		{
			this->_insults.push_back(line);
			line.clear();
		}
		else
			line += ch;
	} while (!my_file.eof());
	my_file.close();
}

void	Bot::run()
{
	int rv = _register();
	if (rv != 1)
	{
		close(this->_socket_fd);
		if (!rv)
			throw std::runtime_error("Connection to server lost");
		if (rv < 0)
			throw std::runtime_error("recv: " + std::string(strerror(errno)));
		if (rv == 464)
			throw std::runtime_error("Incorrect pasword");
	}
	else
		std::cout<<"Bot correctly connected!"<<std::endl;
	while (1)
	{
		memset(this->_buff, 0, sizeof(this->_buff));
		int nbytes = recv(this->_socket_fd, this->_buff, sizeof(this->_buff), 0);
		if (!nbytes) // lost connection to server
			throw std::runtime_error("Connection to server lost");
		if (nbytes < 0)
			throw std::runtime_error("recv: " + std::string(strerror(errno)));
		_handle_cmd(_get_cmd(this->_buff));
	}
}

int	Bot::_register()
{
	std::string msg;
	int nbytes;
	sleep(1);
	msg = "PASS " + this->_password + CRLF;
	_send_msg(msg);
	msg = "NICK "+ BOT_NAME +CRLF;
	_send_msg(msg);
	msg = "USER bot_user 0 * :insulta BOT" + CRLF;
	_send_msg(msg);
	msg = "JOIN #" + BOT_NAME + CRLF;
	_send_msg(msg);
	memset(this->_buff, 0, sizeof(this->_buff));
	nbytes = recv(this->_socket_fd, this->_buff, sizeof(this->_buff), 0);
	
	if (nbytes <= 0)
		return (nbytes);
	int numeric = _get_numeric(this->_buff);
	return (numeric);
}

void Bot::_send_msg(std::string msg) const
{
	send(this->_socket_fd, msg.c_str(), msg.length(), 0);
}

void	Bot::_handleJOIN() const
{
	std::string sender = _get_sender(this->_buff);
	if (sender == BOT_NAME)
		return ;
	std::string header = "PRIVMSG #" + BOT_NAME + " :";
	std::string msg = header + sender + " é appena arrivato, SPERIAMO NON SIA DA ‘A LAZZIO" + CRLF;
	_send_msg(msg);
	msg = header + "** comandi disponibili ** INSULTAMI / INSULTA <NAME> / COMANDI" + CRLF;
	_send_msg(msg);
}

void	Bot::_handlePRIVMSG() const
{
	std::string text = _get_text(this->_buff);
	srand(time(nullptr));
	std::string msg = "PRIVMSG #" + BOT_NAME + " :";
	if (text == ":COMANDI" + CRLF || text.substr(0, text.find(" ")) == ":COMANDI")
		msg += "** comandi disponibili ** INSULTAMI / INSULTA <NAME> / COMANDI" + CRLF;
	else
	{
		std::string insult = this->_insults[rand() % this->_insults.size()];
		std::string sender = _get_sender(this->_buff);
		if (text == ":INSULTAMI" + CRLF || text.substr(0, text.find(" ")) == ":INSULTAMI")
			msg += sender + ", " + insult + CRLF;
		else if (text == ":INSULTA" + CRLF || text.substr(0, text.find(" ")) == ":INSULTA")
		{
			int pos = std::string(text).find(" ");
			if (pos != -1)
			{
				sender = std::string(text).substr(pos);
				sender.pop_back();
				sender.pop_back(); // deleting \r\n
			}
			msg += sender + ", " + insult + CRLF;
		}
	}
	if (msg != ("PRIVMSG #" + BOT_NAME + " :"))
		_send_msg(msg);
}

void	Bot::_handlePART() const
{
	std::string msg = "PRIVMSG #" + BOT_NAME + " :";
	std::string sender = _get_sender(this->_buff);
	msg += sender + " s'é tirato, E 'STI CAZZI" + CRLF;
	_send_msg(msg);
}

void	Bot::_handle_cmd(std::string cmd) const
{
	if (cmd == "JOIN")
		_handleJOIN();
	if (cmd == "PRIVMSG")
		_handlePRIVMSG();
	if (cmd == "PART" || cmd == "QUIT")
		_handlePART();
}

int Bot::_get_numeric(std::string buff) const
{
	buff.erase(0, buff.find(" ") + 1);
	std::string numeric = buff.substr(0, buff.find(" "));
	return (std::atoi(numeric.c_str()));
}

std::string	Bot::_get_cmd(std::string buff) const
{
	buff.erase(0, buff.find(" ") + 1);
	return (buff.substr(0, buff.find(" ")));
}

std::string	Bot::_get_sender(std::string buff) const
{
	buff.erase(0, 1); // deleting first ":"
	return (buff.substr(0, buff.find("!")));
}

std::string	Bot::_get_text(std::string buff) const
{
	int pos = buff.find(" ") + 1; // source
	pos = buff.find(" ", pos) + 1; // PRIVMSG
	pos = buff.find(" ", pos) + 1; // #insultaBOT
	return (buff.substr(pos));
}