#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <stdexcept>
# include <vector>
# include <iostream>
# include <list>
# include <set>

#include "User.hpp"

class Server;

class Channel
{
	public:
		typedef		std::vector<std::pair<char,User *> >	user_list_type;
		typedef		std::set<std::string>					ban_list_type;
		typedef 	std::set<std::string>::iterator			iter_ban_list;
		typedef 	std::set<std::string>::const_iterator	citer_ban_list;
		typedef		std::set<std::string>					excIB_list_type;
		typedef 	std::set<std::string>::iterator			iter_excIB_list;
		typedef 	std::set<std::string>::const_iterator	citer_excIB_list;
	private:
		std::string 	_name;
		std::string		_key;
		std::string 	_topic;
		std::string 	_topicSetter;
		std::time_t		_topicTime;
		std::time_t		_creationTime;
		Server			*_server;
		std::string		_modes;
		int 			_limit;
		std::set<std::string>  _banList;
		std::set<std::string>  _excBanList;
		std::set<std::string>  _excInviteList;
		std::set<std::string>  _inviteList;
		std::vector<std::pair<char,User *> > _users;

	public:
		Channel();
		Channel(std::string name,Server &server);
		Channel(std::string name, std::string key, Server &server);
		Channel(Channel const & channel);
		~Channel();
		/*----METHODS----*/
		
		void				join_user(User &user, std::string key , char status);
		bool				removeUser(User &user);
		bool 				removeUser(std::string const & nick);
		void				part_user(User &user);
		void				sendAll(std::string msg, std::string sender = "") const;
		bool				isInChannel(User const & user) const;
		bool				isInChannel(std::string const & nick) const;
		bool				addMode(User &owner, char m, char mode, std::string param = "");
		void				addMode(char mode);
		void				kick(User &user, std::list<std::string> &users, std::string msg = "Because me stai sul cazzo");
		bool				isOperator(User const &user) const;
		bool				isOperator(std::string const &user) const;
		bool				empty();
		void				delMode(char mode);

		void				invite(User &owner, std::string nick);
		bool				isBanned(User const &owner) const;
		bool				canJoin(User const &owner) const;
		bool				canSendMsg(User const &owner) const;
		bool				isFull() const;
		bool				isInvited(User const &owner) const;
		bool				isInvited(std::string const owner) const;
		bool				isBanned(std::string const owner) const;

		void				ban(User &owner, std::string nick);
		void				exception(User &owner, std::string nick, char type);
		void				unBan(User &owner, std::string nick);
		void				unException(User &owner, std::string nick, char type);
		void				sendBanList(User &owner) const;
		void				sendExeInviteList(User &owner) const;
		void				sendExeBanList(User &owner) const;

		bool				modeINVITE(User &owner, char mode);
		bool				modeBAN(User &owner, char mode, std::string param);
		bool				modeEXCINVITE(User &owner, char mode, std::string param);
		bool				modeEXCBAN(User &owner, char mode, std::string param);
		bool				modeOPERATOR(User &owner, char mode, std::string param);
		bool				modeLIMIT(User &owner, char mode, std::string param);
		bool				modeKEY(User &owner, char mode, std::string param);
		bool				modeMODERATE(User &owner, char mode);
		bool				modeSECRET(User &owner, char mode);
		bool				modeTOPIC(User &owner, char mode);
		bool				modeNOBURINI(User &owner, char mode);

		/*----GETTER STR----*/
		std::string			getStrUsers() const;
		std::string			getLastStrUser();



		/*----GETTER----*/
		size_t			 		getUserCount() const;
		int						getLimit() const;
		std::string const 		&getModes() const;
		std::string 			getName(bool ck = false) const;
		std::string 			getKey() const;
		std::string 			getTopic() const;
		std::string 			getTopicTime() const;
		std::string 			getCreationTime() const;
		void 					getTopic(User &user) const;
		user_list_type const	&getUserList() const;

		const ban_list_type 		&getBanList() const;
		const excIB_list_type 		&getExeInviteList() const;
		const excIB_list_type 		&getExeBanList() const;
		
		/*----SETTER----*/
		void 			setStatus( std::string nick, char status);
		void			setTopic(User &user, std::string &topic);
	
		class InvalidName: public std::exception
		{ 
			const char *what() const throw();
		};
};

User 					*join_channel(std::string arg);
#endif /* ********************************************************* CHANNEL_H */
