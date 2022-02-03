
#include <string>
#include <iostream>

#include "Bot.hpp"

void	parse_args(int ac, char **av, std::string& server_ip, std::string& port, std::string& password)
{
	int i = 0;
	if (ac == 4)
		server_ip = av[i++];
	port = av[i++];
	password = av[i];
}

int main(int ac, char *av[])
{
	if (ac != 3 && ac != 4)
	{
		std::cerr<<"usage: "<<av[0]<<" [server_ip] port password"<<std::endl;
		return (1);
	}
	std::string server_ip = "127.0.0.1";
	std::string port;
	std::string password;
	parse_args(ac, &av[1], server_ip, port, password);
	
	Bot *bot;
	try
	{
		bot = new Bot(server_ip, port, password);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error occured while creating the bot: " << e.what() << std::endl;
		return (1);
	}
	try
	{
		bot->run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error occured while the bot was running: " << e.what() << std::endl;
		return (1);
	}
	delete bot;
}