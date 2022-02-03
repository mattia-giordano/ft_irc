
#include "Channel.hpp"
#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Channel::Channel()
{
	_creationTime = std::time(nullptr);
}

Channel::Channel(std::string name, Server &server) : 
	_name(name), _key(""), _topic(""), _server(&server)
{
	_creationTime = std::time(nullptr);
	_topicTime = std::time(nullptr);
	_limit = INT32_MAX;
}

Channel::Channel(std::string name, std::string key, Server &server): 
	_name(name), _key(key), _topic(""), _server(&server)/*, _founder(us)*/
{
	if (!key.empty())
	{
		/*std::cout<<"aaaa\n" + key;*/
 		_modes = "k";
	}
	_creationTime = std::time(nullptr);
	_topicTime = std::time(nullptr);
	_limit = INT32_MAX;
}

Channel::Channel(Channel const & ch): 
	_name(ch._name), _key(ch._key), _topic(ch._topic), _server(ch._server), _users(ch._users)
{
	_creationTime = ch._creationTime;
	_topicTime = ch._topicTime;
	_limit = INT32_MAX;
	_modes = ch._modes;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Channel::~Channel()
{
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void				Channel::ban(User &owner, std::string nick)
{
	if (!this->isOperator(owner))
	{
		return (_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name));
	}
	if ((_banList.find(nick) != _banList.end()))
		return;
	_banList.insert(nick);
	std::string msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " +b :" + nick + "!*@*" + CRLF;
	if (this->_modes.find('b') == std::string::npos)
		_modes += "b";
	this->sendAll(msg);
}

void				Channel::unBan(User &owner, std::string nick)
{
	if (!this->isOperator(owner))
		return (_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name));
	if (_banList.find(nick) == _banList.end())
		return;
	_banList.erase(nick);
	std::string msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " -b :" + nick + "!*@*" + CRLF;
	this->sendAll(msg);
	if (_banList.empty())
		this->delMode('b');
}
void				Channel::exception(User &owner, std::string nick, char type)
{
	std::string msg;
	if (!this->isOperator(owner))
		return (_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name));
	switch (type)
	{
	case 'I':
			if (_excInviteList.find(nick) != _excInviteList.end())
				return;
			_excInviteList.insert(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " +I :" + nick + "!*@*" + CRLF;
			if (this->_modes.find('I') == std::string::npos)
				_modes += "I";
			this->sendAll(msg);
		break;
	
	case 'e':
			if (_excBanList.find(nick) != _excBanList.end())
				return;
			_excBanList.insert(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " +e :" + nick + "!*@*" + CRLF;
			this->sendAll(msg);
			if (this->_modes.find('e') == std::string::npos)
				_modes += "e";
		break;
	}
}

void				Channel::unException(User &owner, std::string nick, char type)
{
	std::string msg;
	if (!this->isOperator(owner))
		return (_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name));
	switch (type)
	{
	case 'I':
			if (_excInviteList.find(nick) == _excInviteList.end())
				return;
			_excInviteList.erase(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " -I :" + nick + "!*@*" + CRLF;
			this->sendAll(msg);
			if (_excInviteList.empty())
				this->delMode('I');
		break;
	
	case 'e':
			if (_excBanList.find(nick) == _excBanList.end())
				return;
			_excBanList.erase(nick);
			msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
								" MODE " + _name + " -e :" + nick + "!*@*" + CRLF;
			this->sendAll(msg);
			if (_excBanList.empty())
				this->delMode('e');
		break;
	}
}

bool				Channel::empty()
{
	return _users.empty();
}


void				Channel::delMode(char mode)
{
	int pos;
	if ((pos = this->_modes.find(mode)) == -1)
		return ;
	this->_modes = this->_modes.substr(0, pos) + this->_modes.substr(pos+1);
}

void				Channel::addMode(char mode)
{
	if (this->_modes.find(mode) == std::string::npos)
		_modes += mode;
}

bool				Channel::addMode(User &owner, char m, char mode, std::string param)
{
	if (!mode)
		mode = '+';
	switch (m)
	{
		case 'b':
			return modeBAN(owner, mode, param);
		case 'e':
			return modeEXCBAN(owner, mode, param);
		case 'I':
			return modeEXCINVITE(owner, mode, param);
		case 'i':
			return modeINVITE(owner, mode);
		case 'k':
			return modeKEY(owner, mode, param);
		case 'l':
			return modeLIMIT(owner, mode, param);
		case 'o':
			return modeOPERATOR(owner, mode, param);
		case 'm':
			return modeMODERATE(owner, mode);
		case 's':
			return modeSECRET(owner, mode);
		case 't':
			return modeTOPIC(owner, mode);
		case 'n':
			return modeNOBURINI(owner, mode);
	}
	_server->getHandler().numeric_reply(ERR_UNKNOWNMODE, owner, std::string(1, m));
	return false;
}

bool				Channel::modeNOBURINI(User &owner, char mode)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('n') != std::string::npos)
			return false;
		_modes += "n";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +n" + CRLF;
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('n') == std::string::npos)
			return false;
		this->delMode('n');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -n" + CRLF;
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeTOPIC(User &owner, char mode)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('t') != std::string::npos)
			return false;
		_modes += "t";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +t" + CRLF;
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('t') == std::string::npos)
			return false;
		this->delMode('t');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -t" + CRLF;
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeSECRET(User &owner, char mode)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('s') != std::string::npos)
			return false;
		_modes += "s";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +s" + CRLF;
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('s') == std::string::npos)
			return false;
		this->delMode('s');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -s" + CRLF;
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeMODERATE(User &owner, char mode)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (mode == '+')
	{
		if (this->_modes.find('m') != std::string::npos)
			return false;
		_modes += "m";
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +m" + CRLF;
		this->sendAll(msg);	
	}
	else
	{
		if (this->_modes.find('m') == std::string::npos)
			return false;
		this->delMode('m');
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -m" + CRLF;
		this->sendAll(msg);
	}
	return false;
}

bool				Channel::modeINVITE(User &owner, char mode)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (mode == '-' && _modes.find('i') != std::string::npos)
	{
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -i" + CRLF;
		this->sendAll(msg);
		this->delMode('i');
	}
	else if (_modes.find('i') == std::string::npos)
	{
		msg =	":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +i" + CRLF;
		this->sendAll(msg);
		if (this->_modes.find('i') == std::string::npos)
			_modes += "i";
	}
	return false;
}

bool				Channel::modeBAN(User &owner, char mode, std::string param)
{
	std::string msg;
	if (param == "" && mode == '-')
		return false;
	else if (param == "")
	{
		sendBanList(owner);
		return false;
	}
	else if (mode == '+')
		ban(owner, param);
	else
		unBan(owner, param);
	return true;
}

bool				Channel::modeEXCINVITE(User &owner, char mode, std::string param)
{
	std::string msg;
	if (param == "" && mode == '-')
		return false;
	else if (param == "")
	{
		sendExeInviteList(owner);
		return false;
	}
	else if (mode == '+')
		exception(owner, param, 'I');
	else
		unException(owner, param, 'I');
	return true;
}

bool				Channel::modeEXCBAN(User &owner, char mode, std::string param)
{
	std::string msg;
	if (param == "" && mode == '-')
		return false;
	else if (param == "")
	{
		sendExeBanList(owner);
		return false;
	}
	else if (mode == '+')
		exception(owner, param, 'e');
	else
		unException(owner, param, 'e');
	return true;
}

bool				Channel::modeOPERATOR(User &owner, char mode, std::string param)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (param == "")
		return false;
	else 
	{
		size_t i = 0;
		for (; i< _users.size(); i++)
			if (*_users[i].second == param)
				break;
		if (i < _users.size())
		{
			if (mode == '+')
				_users[i].first = '@';
			else
				_users[i].first = '\0';
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " " + mode +"o " + param + CRLF;
			this->sendAll(msg);
		}
		else
			_server->getHandler().numeric_reply(ERR_NOSUCHNICK, owner, param);
	}
	return true;
}

bool				Channel::modeLIMIT(User &owner, char mode, std::string param)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	else if (mode == '+' && param != "")
	{
		int limit = std::atoi(param.c_str());
		if (limit)
		{
			_limit = limit;
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + " +l :" + std::to_string(_limit) + CRLF;
			this->sendAll(msg);
			if (this->_modes.find('l') == std::string::npos)
				_modes += "l";
		}
		return (true);
	}
	else if (mode == '-' && this->_modes.find('l') == std::string::npos)
	{
		this->delMode('l');
		_limit = INT32_MAX;
		msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
						" MODE " + _name + ": -l" + CRLF;
		this->sendAll(msg);
	}
	return false;			
}
bool				Channel::modeKEY(User &owner, char mode, std::string param)
{
	std::string msg;
	if (!this->isOperator(owner))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
	if (param == "" )
	{
		msg = _name + " k * :You must specify a parameter.";
		_server->getHandler().numeric_reply(ERR_INVALIDMODEPARAM,owner, msg);
		return false;
	}
	else if (mode == '-')
	{
		if (param == _key)
		{
			_key = "";
			this->delMode('k');
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " -k :"+ param + CRLF;
			this->sendAll(msg);
		}
		return true;
	}
	else
	{
		if (_key == "")
		{
			msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" MODE " + _name + " +k :"+ param + CRLF;
			this->sendAll(msg);
			if (this->_modes.find('k') == std::string::npos)
				_modes += "k";
		}
		return true;
	}
}

void				Channel::invite(User &owner, std::string nick)
{
	if (!this->isInChannel(owner))
		return (_server->getHandler().numeric_reply(ERR_NOTONCHANNEL, owner, _name));
	if (!this->isOperator(owner))
		return (_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name));
	if (this->isInChannel(nick))
		return (_server->getHandler().numeric_reply(ERR_USERONCHANNEL, owner, nick + " "+ _name));
	_inviteList.insert(nick);
	_server->getHandler().numeric_reply(RPL_INVITING, owner, nick + " "+ _name);
	std::string msg = ":" + owner.getNick() + "!" +  owner.getUsername() + '@' + owner.getHost() + 
							" INVITE " + nick + " :"+ _name + CRLF;
	_server->send_msg(msg, nick);
	
	
}

bool				Channel::isInvited(std::string owner) const
{
	return (_inviteList.find(owner) != _inviteList.end() ||
			_excInviteList.find(owner) != _excInviteList.end());
}

bool				Channel::isInvited(User const &owner) const
{
	return (_inviteList.find(owner.getNick()) != _inviteList.end() ||
			_excInviteList.find(owner.getNick()) != _excInviteList.end());
}

bool				Channel::isBanned(std::string const owner) const
{
	return (_banList.find(owner) != _banList.end() &&
			_excBanList.find(owner) == _excBanList.end());
}


bool				Channel::isBanned(User const &owner) const
{
	return (_banList.find(owner.getNick()) != _banList.end() &&
			_excBanList.find(owner.getNick()) == _excBanList.end());
}

bool				Channel::canJoin(User const &owner) const
{
	if (this->isInChannel(owner))
	{
		_server->getHandler().numeric_reply(ERR_USERONCHANNEL, owner, owner.getNick() + " " + _name);
		return (false);
	}
	if (_modes.find('i') != std::string::npos && !this->isInvited(owner))
	{
		_server->getHandler().numeric_reply(ERR_INVITEONLYCHAN, owner, _name);
		return false;
	}
	if (_modes.find('l') != std::string::npos && this->isFull())
	{
		_server->getHandler().numeric_reply(ERR_CHANNELISFULL, owner, _name);
		return false;
	}
	if (this->isBanned(owner) && (_inviteList.find(owner.getNick()) == _inviteList.end()))
	{
		_server->getHandler().numeric_reply(ERR_BANNEDFROMCHAN, owner, _name);
		return false;
	}
	return (true);
}

bool				Channel::canSendMsg(User const &owner) const
{
	if (_modes.find('n') != std::string::npos && !this->isInChannel(owner))
	{
		_server->getHandler().numeric_reply(ERR_CANNOTSENDTOCHAN, owner, _name);
		_server->getHandler().numeric_reply(ERR_NOTONCHANNEL, owner, _name);
		return false;
	}
	if (this->isBanned(owner))
	{
		_server->getHandler().numeric_reply(ERR_BANNEDFROMCHAN, owner, _name);
		return false;
	}
	if (_modes.find('m') != std::string::npos && !this->isOperator(owner))
	{
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, owner, _name);
		return false;
	}
	return (true);
}

bool				Channel::isFull() const
{
	return (size_t(_limit) >= _users.size());
}


void				Channel::sendBanList(User &owner) 		const
{
	std::string msg;
	for (iter_excIB_list i = _banList.begin(); i != _banList.end();i++)
	{
		std::string msg = _name + " " + *i;
		_server->getHandler().numeric_reply(RPL_BANLIST, owner, msg);
	}
	_server->getHandler().numeric_reply(RPL_ENDOFBANLIST, owner, _name);

}
void				Channel::sendExeInviteList(User &owner)	const
{
	std::string msg;
	for (iter_excIB_list i = _excInviteList.begin(); i != _excInviteList.end();i++)
	{
		std::string msg = _name + " " + *i;
		_server->getHandler().numeric_reply(RPL_INVITELIST, owner, msg);
	}
	_server->getHandler().numeric_reply(RPL_ENDOFINVITELIST, owner, _name);
}

void				Channel::sendExeBanList(User &owner)	const
{
	std::string msg;
	for (iter_excIB_list i = _excBanList.begin(); i != _excBanList.end();i++)
	{
		std::string msg = _name + " " + *i;
		_server->getHandler().numeric_reply(RPL_EXCEPTLIST, owner, msg);
	}
	_server->getHandler().numeric_reply(RPL_ENDOFEXCEPTLIST, owner, _name);
}

void			Channel::join_user(User &user, std::string key , char status = 0)
{
	if (!this->canJoin(user))
		return ;
	if (key == _key)
	{
		user.addChannel(_name);
		_inviteList.erase(user.getNick());
		_users.push_back(std::pair<char,User *>(status, &user));
		std::string msg = ":" + user.getNick() + "!" +  user.getUsername() + '@' + user.getHost() + " JOIN :" + _name + CRLF;
		this->sendAll(msg);
		if (!_topic.empty())
			_server->getHandler().numeric_reply(RPL_TOPIC, user, _name);
		_server->getHandler().numeric_reply(RPL_NAMREPLY, user, "= "+_name + " :"+ this->getStrUsers());
		_server->getHandler().numeric_reply(RPL_ENDOFNAMES, user, _name);
		return ;
	}
	else
		_server->getHandler().numeric_reply(ERR_BADCHANNELKEY, user, _name);
}

void	Channel::part_user(User &user)
{
	if (!this->isInChannel(user))
		return ;
	u_int i = 0;
	for (; i<_users.size(); i++)
	{
		if (*(_users[i].second) == user)
			break ;
	}
	user.removeChannel(_name);
	this->_users.erase(this->_users.begin() + i);
}

void			Channel::sendAll(std::string msg, std::string sender) const
{
	for (size_t i = 0; i < _users.size(); i++)
	{
		if (sender == "" || sender != _users[i].second->getNick())
			_server->send_msg(msg, *(_users[i].second));
	}
}

std::string		Channel::getStrUsers() const
{
	std::string s = "";
	for(size_t i= 0;i < _users.size() ; i++)
	{
		if (_users[i].first)
			s += (_users[i].first) + (_users[i].second)->getNick() +" ";
		else
			s += (_users[i].second)->getNick() +" ";
	}
	return s;
}

std::string		Channel::getLastStrUser()
{
	size_t i = _users.size() - 1;
	if ((_users[i].first))
		return (_users[i].first) + (_users[i].second)->getNick();
	return (_users[i].second)->getNick();
}

bool			Channel::isInChannel(User const & user) const
{
	return (isInChannel(user.getNick()));
}

bool			Channel::isInChannel(std::string const & nick) const
{
	for (u_int i=0; i < this->_users.size(); i++)
	{
		if (*_users[i].second == nick)
			return true;
	}
	return false;
}

bool 			Channel::removeUser(std::string const & nick)
{
	if (!this->isInChannel(nick))
		return false;
	u_int i = 0;
	for (; i<_users.size(); i++)
	{
		if (*(_users[i].second) == nick)
			break ;
	}
	(*_users[i].second).removeChannel(_name);
	this->_users.erase(this->_users.begin() + i);
	return (true);
}

bool			Channel::removeUser(User &user)
{
	return (this->removeUser(user.getNick()));
}

bool			Channel::isOperator(User const &user) const
{
	u_int i = 0;
	for( ; i < _users.size(); i++)
	{
		if (_users[i].first == '@' && user == *_users[i].second)
			return true;
	}
	return false;
}

bool			Channel::isOperator(std::string const &user) const
{
	u_int i = 0;
	for( ; i < _users.size(); i++)
	{
		if ( _users[i].first == '@' && user == _users[i].second->getNick())
			return true;
	}
	return false;
}

void			Channel::kick(User &user, std::list<std::string> & users, std::string msg)
{ 
	if (!this->isInChannel(user))
	{
		_server->getHandler().numeric_reply(ERR_NOTONCHANNEL, user, _name);
		return;
	}
	if (!isOperator(user))
	{
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, user, _name);
		return;
	}
	for (std::list<std::string>::iterator i = users.begin(); i != users.end(); i++)
	{
		if (this->isInChannel(*i))
		{
			msg = ":" + user.getNick() + "!" +  user.getUsername() + " KICK "+ _name+ " " + *i +" :" + msg + CRLF;
			sendAll(msg);
			this->removeUser(*i);
		}
		else
			_server->getHandler().numeric_reply(ERR_USERNOTINCHANNEL, user, *i + " " + _name);
	}

}

void 				Channel::getTopic(User &user) const
{
	if(_topic == "")
		_server->getHandler().numeric_reply(RPL_NOTOPIC, user, _name);
	else 
	{
		_server->getHandler().numeric_reply(RPL_TOPIC, user, _name + " :"+ _topic);
		_server->getHandler().numeric_reply(RPL_TOPICWHOTIME, user, _name + " " + _topicSetter + " " + getTopicTime());
	}
}


void				Channel::setTopic(User &user, std::string &topic)
{
	if (!this->isInChannel(user))
		_server->getHandler().numeric_reply(ERR_NOTONCHANNEL, user, _name);
	else if (_modes.find('t') != std::string::npos &&  !isOperator(user))
		_server->getHandler().numeric_reply(ERR_CHANOPRIVSNEEDED, user, _name);
	else
	{
		_topic = topic;
		_topicSetter = user.getNick();
		_topicTime = std::time(nullptr);
		std::string msg = ":" + user.getNick() + "!" +  user.getUsername() + " TOPIC " + _name + " :"+ _topic + "\n\r";
		_server->send_msg(msg , user);
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

	int					Channel::getLimit() const
	{
		return (_limit);
	}

	std::string const & Channel::getModes() const
	{ return (this->_modes); }

	std::string 	Channel::getName(bool ck) const
	{
		if (!ck || _modes.find('s') == std::string::npos)
			return _name;
		else
			return "*";
	}

	std::string 	Channel::getKey() const
	{
		return(_key);
	}
	std::string 	Channel::getTopic() const
	{
		return(_topic);
	}

	std::string 	Channel::getCreationTime() const
	{
		return(std::to_string(_creationTime));
	}

	std::string 	Channel::getTopicTime() const
	{
		return(std::to_string(_topicTime));
	}

	void 			Channel::setStatus( std::string nick, char status = 0)
	{
		size_t i= 0;
		for( ; i < _users.size(); i++)
		{
			if (nick == (_users[i].second)->getNick())
				break;
		}
		_users[i].first = status;
	}

std::vector<std::pair<char,User *> > const	&Channel::getUserList() const
{
	return (_users);
}

Channel::ban_list_type const			&Channel::getBanList() const
{
	return (_banList);
}
Channel::excIB_list_type const		&Channel::getExeInviteList() const
{
	return (_excInviteList);
}
Channel::excIB_list_type const		&Channel::getExeBanList() const
{
	return (_excBanList);
}

size_t			 					Channel::getUserCount() const
{
	return (_users.size());
}

/*
** --------------------------------- EXCEPTION --------------------------------
*/
const char *	Channel::InvalidName::what() const throw()
{
	return ("InvalidName");
}

/* ************************************************************************** */
