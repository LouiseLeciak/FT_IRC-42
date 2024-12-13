/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lleciak <lleciak@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 01:17:22 by tauer             #+#    #+#             */
/*   Updated: 2024/12/13 09:34:33 by lleciak          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/includes/server.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	std::cout << "\033c";
	Server S(atoi(argv[1]));
	try
	{
		signal(SIGINT, Server::SignalHandler); // catch ctrl c
		signal(SIGQUIT, Server::SignalHandler); // catch ctrl "\"
		S.Run();
	}
	catch (std::exception &e)
	{
		S.CloseFds();
		std::cerr << RED_BG << BOLD_RED << "ERROR : " << e.what() << std::endl;
	}
	std::cout << std::endl << RED_BG << BOLD_RED << "SERVER DOWN" << RESET << std::endl;
	return (0);
}
