#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <string>
# include <list>
# include <map>

# include "User.hpp"

class Server;

class CommandHandler
{
	public:
		CommandHandler(Server	&server);

		void handle(std::string cmd_line, User& owner);

	private:
		typedef std::list<std::string>::iterator _iterator;

		std::string				_command;
		std::list<std::string>	_params;
		Server					&_server;

		std::map<std::string, void (CommandHandler::*)(User&)> _handlers;

		void 					_parse_cmd(std::string cmd_line);
		void					_handlePASS(User &owner);
		void					_handleNICK(User& owner);
		void					_handleUSER(User& owner);
		void					_handleMOTD(User& owner);
		void					_handleLUSERS(User& owner);
		void					_handlePING(User& owner);
		void					_handleJOIN(User &owner);
		void					_handlePART(User &owner);
		void					_handlePRIVMSG(User &owner);
		void					_handleAWAY(User &owner);
		void					_handleQUIT(User &owner);
		void					_handleKICK(User &owner);
		void 					_handleWHO(User& owner);
		void 					_handleMODE(User& owner);
		void					_handleTOPIC(User& owner);
		void					_handleNAMES(User& owner);
		void					_handleLIST(User& owner);
		void					_handleINVITE(User& owner);

		void					_welcome_msg(User& target);
	public:
		void					numeric_reply(int val, User const &owner, std::string extra = "") const;
};

#endif