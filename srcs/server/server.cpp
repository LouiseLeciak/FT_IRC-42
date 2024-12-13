/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lleciak <lleciak@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 01:44:30 by tauer             #+#    #+#             */
/*   Updated: 2024/12/13 10:09:53 by lleciak          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/server.hpp"
#include <algorithm>

bool Server::_Signal = false; // init static variable

Server::Server(int Port) : _Port(Port), _SocketFd(-1)
{
}

//! methods

void Server::Init()
{
	struct sockaddr_in	add; // represente une adresse ipv4 et un port
	struct pollfd		NewPoll; // struct utilisee pour gereer les fds
	int					en;

	add.sin_family = AF_INET;
	add.sin_port = htons(this->Port());
	add.sin_addr.s_addr = INADDR_ANY;
	// creer une socket pour le serveur et on verifie qu'elle est bien cree
	// quand elle est cree la fonciton socket renvois un fd
	_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketFd() == -1)
		throwSocketOptionError(-1, ERR_SOCKET);
	en = 1;
	// set up l'option de la socket pour reuse les adresses et les ports
	// pratique quand un serveur doit lier la meme adresse/port qu'il
	// utilisait avant sans attendre
	// la fonction setsockopt est utilisepour mettre des option sur une socket
	// permet de controle le comportement d'une socket
	if (setsockopt(SocketFd(), SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throwSocketOptionError(-1, ERR_SOCKET_OPTION_R);
	// option de socket O_NONBLOCK pour une socket non bloquante
	// fcntl pour des operations de controle de fd
	// NONBLOCK => les operations comme read et write sur la socket return direct
	// meme s'il n;y a pas de data dispo a lire ou que write ne se complete pas immediatement
	// permet un mechanisme flexible pour gerer les I/O operations (input/output)
	// de facon asynchrone sans bloquer l'execution du programme
	if (fcntl(SocketFd(), F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error(ERR_SOCKET_OPTION_N));
	// on lie la socket a l'adresse avec bind
	// si on ne bind pas la socket est juste un endpoint sans adresse ou port associe
	// bind permet d'assigner une adresse specifique et un port a une socket
	// en lui donnant adresse et port on permet a d'autres process
	// de communiquer avec la socket
	if (bind(SocketFd(), (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error(ERR_SOCKET_BIND));
	// listen pour les connexions a venir, la socket devient une socket passive
	// les passives socket represente le cote serveur
	// elle accepte les connections qui arrivent des clients irc entre autre.
	if (listen(SocketFd(), SOMAXCONN) == -1)
		throw(std::runtime_error(ERR_SOCKET_LISTEN));
	NewPoll.fd = SocketFd(); // ajout de la socket du serveur au pollfd
	NewPoll.events = POLLIN; // POLLIN est set pour indiquer qu'il y ades datas a lire
	NewPoll.revents = 0;
	fds.push_back(NewPoll); // ajout de la socket serveur au vector pollfd
}

void Server::Run()
{
	Init(); // creation de la socket sevreur
	std::cout << GREEN_BG << BOLD_WHITE << " SERVER ON " << RESET << std::endl << BOLD_MAGENTA << "\tPort : " << RESET << LIGHT_YELLOW << this->Port() << RESET << std::endl << BOLD_MAGENTA << "\tSocket Fd : " << RESET << LIGHT_YELLOW << this->SocketFd() << RESET << std::endl << std::endl;
	// on run le serveur jusqu'a reception d'un signal
	while (!Server::_Signal)
	{
		if ((poll(&fds[0], fds.size(), -1) == -1) && Server::_Signal == false)// attend un event
			throw(std::runtime_error(ERR_POLL_FAIL));
		for (size_t i = 0; i < fds.size(); i++)// verifie tous les fds
		{
			if (fds[i].revents & POLLIN)// y a t-il une data a read ?
			{
				if (fds[i].fd == SocketFd())
					AcceptNewClient();
				else
					ReceiveNewData(fds[i].fd);
			}
		}
	}
	CloseFds();// on ferme les fds quand le server ferme
}

void Server::AcceptNewClient()
{
	Client C; // creation d'un nouveau client

	struct sockaddr_in CliAdd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(CliAdd);
	
	// la fonction accepte est bloquee jusqu'a recevoir une demande de connexion
	// puis il return un nouveau fd representant la connexion avec un client
	// et utilisee pour communiquer avec lui
	int incomingFd = accept(SocketFd(), (sockaddr *)&(CliAdd), &len); // accepter le client
	if (incomingFd == -1)
		{std::cout << "\t\t" << RED_BG << BOLD_RED << "failed to accept" << RESET << std::endl; return;}
	if (fcntl(incomingFd, F_SETFL, O_NONBLOCK) == -1)
		{std::cout << "\t\t" << RED_BG << BOLD_RED << "failed to fcntl" << RESET << std::endl; return;}
	
	NewPoll.fd = incomingFd; // ajout de la socket client au vector
	NewPoll.events = POLLIN; // set POLLIN for reading data
	NewPoll.revents = 0;

	C.setFd(incomingFd); // set le fd client
	C.setIPadd(inet_ntoa((CliAdd.sin_addr))); // convertir l'adresse ip en string
	
	clients.push_back(C); // ajout du cleint au vector de client
	fds.push_back(NewPoll); // et sa socket au vecotr de pollfd

	std::cout << "\t\t\t\t\t\t" << GREEN_BG << BOLD_GREEN << "Client "  << RESET <<  GREEN_BG << BOLD_YELLOW << incomingFd << RESET << GREEN_BG << " Connected !" << RESET << std::endl;
	
}

void	Server::kickClient(int fd) {
	std::cout << "\t\t\t\t\t\t" << RED_BG << BOLD_RED << "Client " << RESET << RED_BG << BOLD_YELLOW << fd << RESET << RED_BG << " Disconnected !" << RESET << std::endl;
		ClearClients(fd);
		close(fd);
}

void	Server::PongClient(int fd) {
	const char *Pong = "PONG localhost\n";
	std::cout << "\t\t\t\t\t\t" << YELLOW_BG << BOLD_YELLOW "PONGED CLIENT " << fd << RESET << std::endl;
	send(fd, Pong, strlen(Pong), 0);
}

std::string Server::remove(const std::string &Data, char c) {
	std::string ret = "";
	for (size_t i = 0; i < Data.size(); i++) {
		if (Data[i] != c)
			ret += Data[i];
	}
	return (ret);
}

void	Server::HandleNick(int fd, const std::string &Data) {
	(void)Data;
	
	std::string Nickname = Data.substr(Data.find("NICK") + 5);
	Nickname = Nickname.substr(0, Nickname.find("\r\n"));
	for(size_t i = 0; i < clients.size(); i++) {
		if (clients[i].nickName() == Nickname) {
			std::string error = ":server 433 * " + Nickname + " :Nickname is already in use\r\n";
			send(fd, error.c_str(), error.size(), 0);
			return ;
		}
	}
	clients[fd].nickName() = Nickname;
	std::string welcome = ":server 001 " + Nickname + " :Welcome to the IRC server\r\n";
	send(fd, welcome.c_str(), welcome.size(), 0);
	std::cout << "\t\t\t\t\t\t" << GREEN_BG << BOLD_GREEN << "Client "  << RESET <<  GREEN_BG << BOLD_YELLOW << fd << " " <<  Nickname << RESET << GREEN_BG << " Named !" << RESET << std::endl;
}

void	Server::HandleNewData(int fd, std::string &Data) {
	
		if (Data.find("\r\n") != std::string::npos) {
			std::cout << YELLOW_BG << BOLD_YELLOW << "Client " << RESET << YELLOW_BG << BOLD_RED << fd << " Data :" << RESET
			<< "\n" << Data;   
			if (Data.find("PING") != std::string::npos)
				PongClient(fd);
			else if (Data.find("NICK") != std::string::npos)
				HandleNick(fd, Data);
		}
		//strBuff.erase(std::remove_if(strBuff.begin(), strBuff.end(), isspace), strBuff.end());
}


void Server::ReceiveNewData(int fd)
{
	char buff[1024]; // buffer pour les datas recues
	memset(buff, 0, sizeof(buff)); // clear le buffer
	// on call recv() pour recevoir les datas de la socket du client
	// la data est lu dans buff, le nombre de bytes recu est stocke dans la variable bytes
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);// on recoit les datas
	
	if (bytes <= 0) // on verifie si le client s'est deco ou s'il y a eu une erreur
		kickClient(fd);
	else {// on affiche les datas recues
		buff[bytes] = '\0';
		std::string strBuff(buff);
		HandleNewData(fd, strBuff);
	}
}

//! getters

int Server::Port() const
{
	return (_Port);
}

int Server::SocketFd() const
{
	return (_SocketFd);
}

//! signal

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << "\033[2K"; 
	Server::_Signal = true;
}

//?cleaning

void Server::CloseFds()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
			std::cout << "\t\t\t\t\t\t" << RED_BG << BOLD_RED << "Client " << RESET << RED_BG << BOLD_YELLOW << clients[i].Fd() << RESET << RED_BG << " Disconnected !" << RESET << std::endl;
		if (clients[i].Fd() >= 0)
			close(clients[i].Fd());
	}
}

void Server::ClearClients(int fd)
{
	for (size_t i = 0; i < fds.size(); i++)
		if (fds[i].fd == fd)
			fds.erase(fds.begin() + i);
	for (size_t i = 0; i < clients.size(); i++)
		if (clients[i].Fd() == fd)
		{
			clients.erase(clients.begin() + i);
			break ;
		}
}
