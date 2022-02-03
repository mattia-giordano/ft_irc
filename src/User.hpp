
#ifndef USER_HPP
# define USER_HPP

# include <string>
# include <vector>

class User
{
	public:
		User(int fd, std::string host);
		~User();

		std::string&	buffer();

		bool		is_passed() const;
		bool		is_registered() const;

		void		set_passed();
		void		set_registered();

		void		addChannel(std::string name);
		void		removeChannel(std::string name);
		std::string const &	getHost() const;
		int	getSocket() const;
		std::string const &	getNick() const;
		void				setNick(std::string nick);
		std::string const &	getUsername() const;
		void				setUsername(std::string nick);
		std::string const &	getRealname() const;
		void				setRealname(std::string nick);
		bool				isAway() const;
		void				setAway(bool away, std::string msg = "");
		std::string const & getAwayMsg() const;
		bool				commonChannel(const std::vector<std::string> &channels) const;
		std::vector<std::string> const	&getChannels() const;
		std::string const & getModes() const;
		bool				hasMode(char mode) const;
		void				addMode(char mode);
		void				delMode(char mode);

		bool				operator==(User const & other) const;
		bool				operator==(std::string const & other) const;
	private:
		
		int			_socket_fd;
		std::string _host;
		bool		_pass_set;
		bool		_registered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer;
		bool		_away;
		std::string	_away_msg;
		std::string	_modes;
		std::vector<std::string> _channels;
};

std::string	toUpper(std::string const & str);

#endif