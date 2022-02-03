
#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <unistd.h>
# include <cerrno>
# include <iostream>
# include <vector>
# include <fstream>

# define INSULTS_PATH "./bot/.insulti.txt"
# define BOT_NAME std::string("insultaBOT")
# define CRLF std::string("\r\n")

class Bot
{
	public:
		Bot(std::string host_ip, std::string port, std::string password);
		~Bot();

		void run();

	private:
		std::string _server_ip;
		std::string _port;
		std::string _password;
		int			_socket_fd;
		char		_buff[512];
		std::vector<std::string> _insults;

		int			_register();
		void		_load_insults(const char *file);
		
		int			_get_numeric(std::string buff) const;
		std::string	_get_cmd(std::string buff) const;
		std::string _get_sender(std::string buff) const;
		std::string _get_text(std::string buff) const;
		
		void		_send_msg(std::string msg) const;
		
		void		_handle_cmd(std::string cmd) const;
		void		_handleJOIN() const;
		void		_handlePRIVMSG() const;
		void		_handlePART() const;
};

#endif