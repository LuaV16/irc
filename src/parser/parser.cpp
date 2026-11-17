/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cdaureo- <cdaureo-@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/17 17:08:40 by cdaureo-          #+#    #+#             */
/*   Updated: 2026/11/17 17:55:48 by cdaureo-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc/parser.hpp"
namespace ParserUtils
{
    //Avanza hasta el siguiente espacio
    void skipSpaces(const std::string &line, std::string::size_type &position)
    {
        while (position < line.size() && line[position] == ' ')
            position++;
    }
    //Lee palabra
    std::string readWord(const std::string &line, std::string::size_type &position)
    {
        std::string::size_type start = position;
        while (position < line.size() && line[position] != ' ')
            position++;
        return line.substr(start, position - start);
    }
    //Hace uppercase al comando para evitar errores
    void upperCaseCommand(std::string &command)
    {
        for (std::string::size_type i = 0; i < command.size(); i++)
        {
            if (command[i] >= 'a' && command[i] <= 'z')
                command[i] = command[i] - 'a' + 'A';
        }
           
    }
}
// Separa el prefijo opcional, el comando y sus parámetros.
// Un ':' al inicio de un parámetro convierte todo lo restante
// en un único parámetro, conservando sus espacios.
bool parseLine(const std::string &line, IrcMessage &message)
{
    std::string::size_type i = 0;
    message.prefix.clear();
    message.command.clear();
    message.params.clear();
    ParserUtils::skipSpaces(line, i);
    if (line.size() == i)
        return false;
    if (line[i] == ':')
    {
        i++;
        message.prefix = ParserUtils::readWord(line,i);
        if (message.prefix.empty())
            return false;
        ParserUtils::skipSpaces(line, i);
    }
    message.command = ParserUtils::readWord(line,i);
        if (message.command.empty())
            return false;
    ParserUtils::upperCaseCommand(message.command);
    while(line.size() > i)
    {
        ParserUtils::skipSpaces(line, i);
        if (i == line.size())
            break;
        if (line[i] == ':')
        {
            message.params.push_back(line.substr(i + 1));
            break;
        }
        message.params.push_back(ParserUtils::readWord(line, i));
    }
    return true;
}
