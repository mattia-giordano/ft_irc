
#include "CommandHandler.hpp"
#include "Server.hpp"

CommandHandler::CommandHandler(Server	&server): _server(server)
{
	this->_handlers["PASS"] = &CommandHandler::_handlePASS;
	this->_handlers["NICK"] = &CommandHandler::_handleNICK;
	this->_handlers["USER"] = &CommandHandler::_handleUSER;
	this->_handlers["MOTD"] = &CommandHandler::_handleMOTD;
	this->_handlers["LUSERS"] = &CommandHandler::_handleLUSERS;
	this->_handlers["PING"] = &CommandHandler::_handlePING;
	this->_handlers["JOIN"] = &CommandHandler::_handleJOIN;
	this->_handlers["PART"] = &CommandHandler::_handlePART;
	this->_handlers["PRIVMSG"] = &CommandHandler::_handlePRIVMSG;
	this->_handlers["AWAY"] = &CommandHandler::_handleAWAY;
	this->_handlers["QUIT"] = &CommandHandler::_handleQUIT;
	this->_handlers["WHO"] = &CommandHandler::_handleWHO;
	this->_handlers["KICK"] = &CommandHandler::_handleKICK;
	this->_handlers["MODE"] = &CommandHandler::_handleMODE;
	this->_handlers["TOPIC"] = &CommandHandler::_handleTOPIC;
	this->_handlers["NAMES"] = &CommandHandler::_handleNAMES;
	this->_handlers["INVITE"] = &CommandHandler::_handleINVITE;
	this->_handlers["LIST"] = &CommandHandler::_handleLIST;
}

void CommandHandler::_parse_cmd(std::string cmd_line)
{
	if (cmd_line.empty())
		return ;
	int pos = cmd_line.find(" ");
	this->_command = toUpper(cmd_line.substr(0, pos));
	cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
	this->_params.clear();

	while (!cmd_line.empty())
	{
		if (cmd_line[0] == ':')
		{
			cmd_line.erase(0, 1);
			if (cmd_line.empty())
			{
				this->_params.push_back("");
				break ;
			}
			this->_params.push_back(cmd_line);
			cmd_line.erase(0);
		}
		else
		{
			pos = cmd_line.find(" ");
			this->_params.push_back(cmd_line.substr(0, pos));
			cmd_line.erase(0, (pos != -1) ? pos + 1 : pos);
		}
	}
}

void CommandHandler::handle(std::string cmd_line, User& owner)
{
	_parse_cmd(cmd_line);
	if (this->_command.empty())
		return ;
	if (!owner.is_passed() && this->_command != "PASS")
		return ;
	else if (owner.is_passed() && !owner.is_registered() && this->_command != "NICK" && this->_command != "USER")
		return numeric_reply(ERR_NOTREGISTERED, owner, this->_command);
	if (this->_handlers.find(this->_command) == this->_handlers.end())
		numeric_reply(ERR_UNKNOWNCOMMAND, owner, this->_command);
	else
		(*this.*(this->_handlers[this->_command]))(owner);
}

void CommandHandler::_handlePASS(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command);
	if (owner.is_registered())
		return numeric_reply(ERR_ALREADYREGISTERED, owner);
	if (this->_server.checkPass(this->_params.front()))
		owner.set_passed();
	else
		numeric_reply(ERR_PASSWDMISMATCH, owner);
}

void CommandHandler::_handleNICK(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(ERR_NONICKNAMEGIVEN, owner); 
	std::string& nick = this->_params.front();
	std::vector<User*> const & users = this->_server.getUserList();
	for (u_int i = 0; i < users.size(); i++)
	{
		if (*users[i] == nick)
			return numeric_reply(ERR_NICKNAMEINUSE, owner, nick); 
	}
	std::string old_nick = owner.getNick();
	owner.setNick(nick);
	if (old_nick != "")
	{
		std::string msg = ":" + old_nick + "!" + owner.getUsername() + "@" + owner.getHost() + " NICK :" + owner.getNick() + CRLF;
		this->_server.send_msg(msg, owner);
	}
	if (!owner.is_registered() && !owner.getUsername().empty())
		_welcome_msg(owner);
}

void CommandHandler::_handleUSER(User& owner) 
{
	if (this->_params.size() != 4)
		return numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command);
	if (owner.is_registered())
		return numeric_reply(ERR_ALREADYREGISTERED, owner);
	std::string username = this->_params.front();
	if (username.empty()) // i think this case wont ever occur
		return numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command);
	std::string realname = this->_params.back();
	owner.setUsername(username);
	owner.setRealname(realname);
	if (!owner.getNick().empty())
		_welcome_msg(owner);
}

void CommandHandler::_handleMOTD(User& owner)
{
	if (this->_command == "MOTD" && this->_params.size() && this->_params.front() != SERV_NAME)
		return numeric_reply(ERR_NOSUCHSERVER, owner, this->_params.front());
	std::vector<std::string> motd = this->_server.getMotd();
	if (!motd.size())
		return numeric_reply(ERR_NOMOTD, owner);
	numeric_reply(RPL_MOTDSTART, owner); 
	for (u_int i=0; i < motd.size(); i++)
		numeric_reply(RPL_MOTD, owner, motd[i]);
	numeric_reply(RPL_ENDOFMOTD, owner); 
}

void CommandHandler::_handleLUSERS(User& owner)
{
	int user_count = this->_server.getUserList().size();
	int chan_count = this->_server.getchannelList().size();
	numeric_reply(RPL_LUSERCLIENT, owner, std::to_string(user_count));
	numeric_reply(RPL_LUSEROP, owner);
	numeric_reply(RPL_LUSERCHANNELS, owner, std::to_string(chan_count)); 
	numeric_reply(RPL_LUSERME, owner, std::to_string(user_count)); 
}

void CommandHandler::_handlePING(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command);
	std::string msg = ":" + SERV_NAME + " PONG " + SERV_NAME + " :" + this->_params.front() + CRLF;
	this->_server.send_msg(msg, owner);
}

void CommandHandler::_handlePRIVMSG(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return numeric_reply(ERR_NORECIPIENT, owner, this->_command);
	if (this->_params.size() == 1)
		return numeric_reply(ERR_NOTEXTTOSEND, owner);

	std::string targets = this->_params.front();
	_iterator it = ++this->_params.begin();
	std::string text = " :" + *it;
	for (++it; it != this->_params.cend(); ++it)
		text += " "+*it;
	std::string head = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " PRIVMSG ";
	while (!targets.empty())
	{
		int pos = targets.find(",");
		std::string curr_target = targets.substr(0, pos);
		std::string msg = head + curr_target + text + CRLF;
		int rv;
		if (curr_target[0] == '#')
			rv = this->_server.send_msg(msg, curr_target, owner);
		else
			rv = this->_server.send_msg(msg, curr_target);
		if (rv == RPL_AWAY)
			numeric_reply(rv, owner, curr_target + " : " + this->_server.getUser(curr_target).getAwayMsg());
		if (rv == ERR_NOSUCHNICK)
			numeric_reply(rv, owner, curr_target);
		targets.erase(0, (pos != -1) ? pos + 1 : pos);
	}
}

void CommandHandler::_handleAWAY(User& owner)
{
	if (!this->_params.size())
	{
		owner.setAway(false);
		numeric_reply(RPL_UNAWAY, owner);
	}
	else
	{
		owner.setAway(true, this->_params.front());
		numeric_reply(RPL_NOWAWAY, owner);
	}
}

void CommandHandler::_handleJOIN(User& owner)
{
	if (_params.empty())
		return numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command);
	std::list<std::string> names;
	std::list<std::string> keys;
	int pos;
	
	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		names.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	
	_params.pop_front();
	if (!_params.empty())
	{
		while( _params.front() != "")
			{
				pos = _params.front().find(",");
				keys.push_back(_params.front().substr(0, pos));
				_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
			}
		_params.pop_front();
	}
	if (names.front()[0] != '#')
		return (numeric_reply(ERR_NOSUCHCHANNEL, owner, names.front()));
	while(!names.empty())
	{
		char stat  = 0;
		if (!_server.exist_channel(names.front()))
		{
			Channel ch(names.front(), keys.front(), _server);
			_server.add_channel(ch);
			stat = '@';
		}
		Channel &chan = _server.get_channel(names.front());
		chan.join_user(owner, keys.front(), stat);
		if (!keys.empty())
			keys.pop_front();
		names.pop_front();
	}
}

void CommandHandler::_handlePART(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return (numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command)); // ERR_NEEDMOREPARAMS
	std::string targets = this->_params.front();
	std::string reason;
	if (this->_params.size() > 1 &&  this->_params.front() != "")
	{
		_iterator it = ++this->_params.begin();
		reason = " :\"" + *it;
		for (++it; it != this->_params.cend(); ++it)
			reason += " "+*it;
		reason += "\"";
	}
	std::string head = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " PART ";
	while (!targets.empty())
	{
		int pos = targets.find(",");
		std::string curr_target = targets.substr(0, pos);
		std::string msg = head + curr_target + reason + CRLF;

		if (!this->_server.exist_channel(curr_target))
			numeric_reply(ERR_NOSUCHCHANNEL, owner, curr_target);
		else
		{
			Channel& tmp_chan = this->_server.get_channel(curr_target);
			if (!tmp_chan.isInChannel(owner))
				numeric_reply(ERR_NOTONCHANNEL, owner, curr_target);
			else
			{
				// send to owner and to other inside channel
				this->_server.send_msg(msg, owner);
				this->_server.send_msg(msg, curr_target, owner);
				tmp_chan.part_user(owner);
				if (tmp_chan.empty())
					_server.removeChannel(tmp_chan.getName());
			}
		}
		targets.erase(0, (pos != -1) ? pos + 1 : pos);
	}
}

void CommandHandler::_handleQUIT(User& owner)
{
	std::string reason = (_params.size() == 1) ? _params.front() : owner.getNick();
	std::string msg = "ERROR :Closing Link: " + owner.getNick() + "[" + owner.getHost() + "] (Quit: " + reason + ")" + CRLF;
	this->_server.send_msg(msg, owner);
	msg = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " QUIT :Quit: " + reason + CRLF;
	this->_server.sendAllChans(msg, owner);
	
	this->_server.deleteUser(owner.getNick());
}

void CommandHandler::_handleWHO(User& owner)
{
	const std::vector<User*> &us =  _server.getUserList();
	std::string msg;
	Channel ch;
	if (_params.empty())
	{
		for(size_t i =0 ; i !=us.size(); i++)
		{
			if (!((us[i]->commonChannel(owner.getChannels())) || us[i]->hasMode('i')) || *us[i] == owner)
			{														
				msg = (us[i]->getChannels().empty() ? "* " : us[i]->getChannels().back()+ " ") + us[i]->getUsername() + " " + us[i]->getHost() + " myIRCServer " + us[i]->getNick() +
				 " H :0 "  + us[i]->getRealname();
				numeric_reply(RPL_WHOREPLY, owner, msg);
			}
		}
		numeric_reply(RPL_ENDOFWHO, owner, "*");
	}
	else if(_server.exist_channel(_params.front()))
	{
		ch = _server.get_channel(_params.front());
		const Channel::user_list_type &users = ch.getUserList();
		std::string header = ch.getName(true) + " ";
		for (size_t i =0 ;i != users.size(); i++)
		{
			msg = header + users[i].second->getUsername() + " " +  users[i].second->getHost() + " " + SERV_NAME + " " + users[i].second->getNick() + " H";
			if (users[i].first)
				msg += users[i].first;
			msg += " :0 " + users[i].second->getRealname();
			numeric_reply(RPL_WHOREPLY, owner, msg);
		}
		numeric_reply(RPL_ENDOFWHO, owner, ch.getName(true));
	}
}

void	CommandHandler::_handleKICK(User &owner)
{
	if (_params.size() < 2)
		return numeric_reply(ERR_NEEDMOREPARAMS, owner, "KICK");
	std::list<std::string> channels;
	std::list<std::string> users;
	int pos;
	
	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		channels.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	_params.pop_front();


	while( _params.front() != "")
	{
		pos = _params.front().find(",");
		users.push_back(_params.front().substr(0, pos));
		_params.front().erase(0, (pos != -1) ? pos + 1 : pos);
	}
	_params.pop_front();

	while (!channels.empty())
	{
		if (_server.exist_channel(channels.front()))
		{
			Channel &chan = _server.get_channel(channels.front());
			if (!_params.empty())
				chan.kick(owner,users, _params.front());
			else
				chan.kick(owner,users);
			if (chan.empty())
				_server.removeChannel(chan.getName());
			channels.pop_front();
		}
		else
			numeric_reply(ERR_NOSUCHCHANNEL, owner, channels.front());
	}
}


void	CommandHandler::_handleMODE(User& owner)
{
	if (!this->_params.size() || this->_params.front() == "")
		return (numeric_reply(ERR_NEEDMOREPARAMS, owner, this->_command));
	std::string target = this->_params.front();
	if (target[0] == '#') // CHANNEL MODE
	{
		if (!_server.exist_channel(target))
			return (numeric_reply(ERR_NOSUCHCHANNEL, owner, target));
		Channel &ch = _server.get_channel(target);
		if (this->_params.size() == 1 )
		{
			numeric_reply(RPL_CHANNELMODEIS, owner, target + " +" + _server.get_channel(target).getModes());
			numeric_reply(RPL_CREATIONTIME, owner, target + " " + _server.get_channel(target).getCreationTime());
			return ;
		}
		_params.pop_front();
		std::string mode = _params.front();
		_params.pop_front();
		char type = (mode[0] == '-' || mode[0] == '+') ? mode[0] : 0;
		for (size_t i = (type != 0); i < mode.size(); i++)
		{
			if (ch.addMode(owner, mode[i], type, _params.front()))
				_params.pop_front();
		}
	}
	else	// USER MODE
	{
		std::vector<User*> users = this->_server.getUserList();
		uint i=0;
		for (; i<users.size() && users[i]->getNick() != target; i++) ;
		if (i == users.size())
			return (numeric_reply(ERR_NOSUCHNICK, owner, target));
		if (owner.getNick() != target)
			return (numeric_reply(ERR_USERSDONTMATCH, owner));
		if (this->_params.size() == 1 )
			return (numeric_reply(RPL_UMODEIS, owner, target));
		std::string modestring = *(++(this->_params.begin()));
		std::string msg = " ";
		for (i=0; i<modestring.length(); i++)
		{
			char mode = modestring[i];
			if (mode == '+' || mode == '-')
				continue;
			if (UMODES.find(mode) == std::string::npos)
				numeric_reply(ERR_UMODEUNKNOWNFLAG, owner);
			else if (i && modestring[i - 1] == '-')
			{
				owner.delMode(mode);
				msg += "-";
				msg += mode;
			}
			else
			{
				owner.addMode(mode);
				msg += "+";
				msg += mode;
			}
		}
		if (msg != " ")
		{
			msg = ":" + owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost() + " MODE " + owner.getNick() + msg + CRLF;
			this->_server.send_msg(msg, owner);
		}
	}
}

void	CommandHandler::_handleTOPIC(User& owner)
{
	if (_params.size() < 1 || this->_params.front() == "")
		numeric_reply(ERR_NEEDMOREPARAMS, owner, "TOPIC");
	else if (!_server.exist_channel(_params.front()))
		numeric_reply(ERR_NOSUCHCHANNEL, owner, _params.front());
	else 
	{
		Channel &ch = _server.get_channel(_params.front());
		if (_params.size() == 1)
		{
			ch.getTopic(owner);
			
			return;
		}
		_params.pop_front();
		ch.setTopic(owner, _params.front());
	}
}

void	CommandHandler::_handleLIST(User& owner)
{
	numeric_reply(RPL_LISTSTART, owner);
	std::string msg = "";
	const Server::chan_type		&chs = _server.getchannelList();
	for (Server::chan_it i = chs.cbegin(); i != chs.cend(); i++)
	{
		msg = (*i).second.getName(true) + " " + std::to_string((*i).second.getUserCount()) + " :[+" + (*i).second.getModes() + "] " + (*i).second.getTopic();
		numeric_reply(RPL_LIST, owner, msg);
	}
	numeric_reply(RPL_LISTEND, owner);
}

void	CommandHandler::_handleNAMES(User& owner)
{
	if (_params.size() < 1 || this->_params.front() == "")
	{
		for (std::map<std::string, Channel>::const_iterator i = _server.getchannelList().cbegin();
			i != _server.getchannelList().cend() ; i++)
		{
			std::string msg = "= " + (*i).second.getName(true) + " :"+  (*i).second.getStrUsers() ;
			numeric_reply(RPL_NAMREPLY, owner, msg);
			numeric_reply(RPL_ENDOFNAMES, owner, (*i).second.getName(true));
		}
		const std::vector<User*> & users = _server.getUserList();
		std::string msg = ":" + owner.getNick() + "!" +  owner.getUsername() + " ";
		for (size_t i = 0; i < users.size() ; i++)
		{
			if (!users[i]->getChannels().size())
			{
				msg +=  users[i]->getNick() + " * " ;
			}
		}
		msg += CRLF;
		_server.send_msg( msg, owner);
	}
}	

void		CommandHandler::_handleINVITE(User& owner) 
{
	std::string msg = "";
	std::string nick;
	if (_params.size() < 2 || _params.back() == "")
		return (numeric_reply(ERR_NEEDMOREPARAMS, owner, _command));
	nick = _params.front();
	if (!_server.exist_user(_params.front()))
		return (numeric_reply(ERR_NOSUCHNICK, owner, _params.front()));
	_params.pop_front();
	if (!_server.exist_channel(_params.front()))
		return (numeric_reply(ERR_NOSUCHCHANNEL, owner, _params.front()));
	Channel &ch = _server.get_channel(_params.front());
	ch.invite(owner, nick);
}

void		CommandHandler::numeric_reply(int val, User const &owner, std::string extra) const
{
	std::string msg = ":" + SERV_NAME + " ";
	if (val < 10)
	{
		msg+= "00";
		msg+= val+'0';
	}
	else
		msg+=std::to_string(val);

	msg += " " + owner.getNick() + " ";
	switch (val)
	{
		case RPL_WELCOME:
			msg += ":Welcome to the Internet Relay Network ";
			msg += owner.getNick() + "!" + owner.getUsername() + "@" + owner.getHost();
			break;
		case RPL_YOURHOST: 
			msg += ":Your host is " + SERV_NAME + ", running version IRC1.0";
			break;
		case RPL_CREATED: 
			msg += ":This server was created " + extra;
			break;
		case RPL_MYINFO: 
			msg += SERV_NAME + " IRC1.0 " + UMODES + " " + CMODES;
			break;
		case RPL_UMODEIS:
			msg += owner.getModes();
			break;
		case RPL_LUSERCLIENT:
			msg += ":There are " + extra + " users and 0 invisible on 1 servers";
			break;
		case RPL_LUSEROP:
			msg += "0 :operator(s) online";
			break;
		case RPL_LUSERCHANNELS:
			msg += extra + " :channels formed";
			break;
		case RPL_LUSERME:
			msg += ":I have " + extra + " clients and 1 servers";
			break;
		case RPL_AWAY:
			msg += extra;
			break;
		case RPL_UNAWAY:
			msg += ":You are no longer marked as being away";
			break;
		case RPL_NOWAWAY:
			msg += ":You have been marked as being away";
			break;
		case RPL_ENDOFWHO:
			msg += extra + " :End of /WHO list";
			break;
		case RPL_LISTSTART:
			msg += "Channel :Users  Name";
			break;
		case RPL_LIST:
			msg += extra;
			break;
		case RPL_LISTEND: 
			msg += extra + " :End of /LIST";
			break;
		case RPL_CHANNELMODEIS: 
			msg += extra;
			break;
		case RPL_CREATIONTIME:
			msg += extra;
			break;
		case RPL_NOTOPIC:
			msg += extra + " :No topic is set";
			break;
		case RPL_TOPIC:
			msg += extra;
			break;
		case RPL_TOPICWHOTIME:
			msg += extra;
			break;
		case RPL_INVITING:
			msg += extra;
			break;
		case RPL_INVITELIST: 
			msg += extra;
			break;
		case RPL_ENDOFINVITELIST: 
			msg += extra  + " :End of channel invite list.";
			break;
		case RPL_EXCEPTLIST:
			msg += extra;
			break;
		case RPL_ENDOFEXCEPTLIST:
			msg += ":End of channel exception list.";
			break;
		case RPL_WHOREPLY:
			msg += extra ;
			break;
		case RPL_NAMREPLY:
			msg += extra;
			break;
		case RPL_ENDOFNAMES:
			msg += extra + " :End of /NAMES list";
			break;
		case RPL_BANLIST:
			msg += extra + " :End of channel ban list.";
			break;
		case RPL_ENDOFBANLIST:
			msg += extra  + " :End of /NAMES list.";
			break;
		case RPL_MOTD:
			msg += ":" + extra;
			break;
		case RPL_MOTDSTART:
			msg += ":- " + SERV_NAME + " Message of the day - ";
			break;
		case RPL_ENDOFMOTD:
			msg += ":End of /MOTD command.";
			break;
		case ERR_NOSUCHNICK:
			msg += extra + " :No such nick/channel";
			break;
		case ERR_NOSUCHSERVER:
			msg += extra + " :No such server";
			break;
		case ERR_NOSUCHCHANNEL:
			msg += extra + " :No such channel";
			break;
		case ERR_CANNOTSENDTOCHAN:
			msg += extra + " :Cannot send to channel";
			break;
		case ERR_NORECIPIENT:
			msg += ":No recipient given (" + extra + ")";
			break;
		case ERR_NOTEXTTOSEND:
			msg += ":No text to send";
			break;
		case ERR_UNKNOWNCOMMAND :
			msg += extra + " :Unknown command";
			break;
		case ERR_NOMOTD:
			msg += ":MOTD File is missing";
			break;
		case ERR_NONICKNAMEGIVEN:
			msg += ":No nickname given";
			break;
		case ERR_NICKNAMEINUSE:
			msg += extra + " :Nickname is already in use"; 
			break;
		case ERR_USERNOTINCHANNEL:
			msg += extra + " :They aren't on that channel";
			break;
		case ERR_NOTONCHANNEL:
			msg += extra + " :You're not on that channel";
			break;
		case ERR_USERONCHANNEL:
			msg += extra + " :is already on channel";
			break;
		case ERR_NOTREGISTERED:
			msg += extra + " :You have not registered";
			break;
		case ERR_NEEDMOREPARAMS:
			msg += extra + " :Not enough parameters"; 
			break;
		case ERR_ALREADYREGISTERED:
			msg += ":You may not reregister"; 
			break;
		case ERR_PASSWDMISMATCH:
			msg += ":Password incorrect";
			break;
		case ERR_CHANNELISFULL:
			msg += extra + " :Cannot join channel (+l)";
			break;
		case ERR_UNKNOWNMODE:
			msg += extra + " :is unknown mode char to me";
			break;
		case ERR_INVITEONLYCHAN:
			msg += extra + " :Cannot join channel (+i)";
			break;
		case ERR_BANNEDFROMCHAN:
			msg += extra + " :Cannot join channel (+b)";
			break;
		case ERR_BADCHANNELKEY:
			msg += extra + " :Cannot join channel (+k)";
			break;
		case ERR_CHANOPRIVSNEEDED:
			msg += extra + " :You're not channel operator";
			break;
		case ERR_UMODEUNKNOWNFLAG:
			msg += ":Unknown MODE flag";
			break;
		case ERR_USERSDONTMATCH:
			msg += ":Cant change mode for other users";
			break;
		case ERR_INVALIDMODEPARAM:
			msg += extra ;
			break;
	}
	msg += CRLF;
	this->_server.send_msg(msg, owner);
}

void	CommandHandler::_welcome_msg(User& target)
{
	target.set_registered();
	numeric_reply(RPL_WELCOME, target);
	numeric_reply(RPL_YOURHOST, target);
	numeric_reply(RPL_CREATED, target, this->_server.getDateTimeCreated());
	numeric_reply(RPL_MYINFO, target);
	_handleMOTD(target);
	_handleLUSERS(target);
}