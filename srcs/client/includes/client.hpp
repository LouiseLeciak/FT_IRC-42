/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lleciak <lleciak@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 01:22:35 by tauer             #+#    #+#             */
/*   Updated: 2024/12/13 09:30:42 by lleciak          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../../utils/includes/colors.hpp"

#include <iostream>

class Client
{
  private:
	std::string _nickName;
  
	int 		_fd; // fd client
	std::string _IPadd;// adresse ip du client
	
	//std::vector<std::string> channelsList;

  public:
	//!Base
	Client();
	~Client();

	//? setters
	void 		setFd(const int fd);
	void 		setIPadd(const std::string &IPadd);

	//? getters
	int 		Fd() const;
	std::string IPadd() const;
	std::string nickName() const;
};