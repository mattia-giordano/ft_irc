
#include "User.hpp"

User::User(int fd, std::string host) : _socket_fd(fd), _host(host), _pass_set(false), _registered(false), _away(false), _modes("+") {}

User::~User() {};

std::string& 		User::buffer() { return (this->_buffer); }

bool				User::is_passed() const { return(this->_pass_set); }
bool				User::is_registered() const { return(this->_registered); };

void				User::set_passed() { this->_pass_set= true; }
void				User::set_registered() { this->_registered = true; }

void				User::addChannel(std::string name)
{
	if (std::find(_channels.begin(),_channels.end(), name) == _channels.end())
		_channels.push_back(name);
}

void				User::removeChannel(std::string name)
{
	_channels.erase(std::find(_channels.begin(),_channels.end(), name));
}

bool				User::commonChannel(const std::vector<std::string> &channels) const
{
	std::vector<std::string>::const_iterator i = channels.begin();
	for (;i != channels.end() ;i++)
		if (std::find(_channels.begin(),_channels.end(), *i) != _channels.end())
			return (true);
	return (false);
}

std::vector<std::string> const	&User::getChannels() const
{
	return (_channels);
}

std::string const &	User::getHost() const
{ return (this->_host); }

int	User::getSocket() const
{ return (this->_socket_fd); };

std::string const &	User::getNick() const
{ return (this->_nickname); }

void				User::setNick(std::string nick)
{ this->_nickname = nick; }

std::string const &	User::getUsername() const
{ return (this->_username); }

void				User::setUsername(std::string username)
{ this->_username = username; }

void				User::setRealname(std::string realname)
{ this->_realname = realname; }
std::string const &	User::getRealname() const
{ return _realname;}

bool				User::isAway() const
{ return (this->_away); }

void				User::setAway(bool away, std::string msg)
{
	this->_away = away;
	if (away)
		this->_away_msg = msg;
	else
		this->_away_msg.clear();
}

std::string const & User::getAwayMsg() const
{ return (this->_away_msg); }

std::string	toUpper(std::string const & str)
{
	std::string result;
	for (unsigned int i=0; i<str.length(); i++)
		result += std::toupper(str[i]);
	return (result);
}

bool				User::operator==(std::string const & nick) const
{ return (toUpper(this->_nickname) == toUpper(nick)); }

bool				User::operator==(User const & other) const
{ return (*this == other._nickname); }

std::string const & User::getModes() const
{ return (this->_modes); }

bool				User::hasMode(char mode) const
{
	if (this->_modes.find(mode) == std::string::npos)
		return false;
	return true;
}

void				User::addMode(char mode)
{
	if (!hasMode(mode))
		this->_modes += mode;
}

void				User::delMode(char mode)
{
	int pos;
	if ((pos = this->_modes.find(mode)) == -1)
		return ;
	this->_modes = this->_modes.substr(0, pos) + this->_modes.substr(pos+1);
}