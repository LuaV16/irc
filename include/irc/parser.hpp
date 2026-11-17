/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cdaureo- <cdaureo-@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/17 17:08:38 by cdaureo-          #+#    #+#             */
/*   Updated: 2026/11/17 17:55:51 by cdaureo-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

 #ifndef IRC_PARSER_HPP
  # define IRC_PARSER_HPP

  # include <string>
  # include <vector>
struct IrcMessage
{
    std::string prefix;
    std::string command;
    std::vector<std::string> params; 
};
bool parseLine(const std::string &line, IrcMessage &message);
#endif