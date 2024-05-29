#include <string>
#include <iostream>
#include <fstream>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include "database/user.h"



int main() 
{
    try
    {
        std::string json;
        std::ifstream is("generic_data.json");
        std::istream_iterator<char> eos;
        std::istream_iterator<char> iit(is);
        while (iit != eos)
            json.push_back(*(iit++));
        is.close();

        database::User::clear_all();

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json);
        Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

        size_t i{0};
        for (i = 0; i < arr->size(); ++i)
        {
            Poco::JSON::Object::Ptr object = arr->getObject(i);

            database::User u;

            u.first_name() = object->getValue<std::string>("FirstName");
            u.last_name() = object->getValue<std::string>("LastName");
            u.email() = object->getValue<std::string>("Email");
            u.login() = u.first_name()+u.last_name()+u.email();
            u.password() ="qwertyuiop";

            u.save_to_mysql();

            if(i%100==0) std::cout << "." << std::endl; 
        }

        std::cout << "Inserted " << i << " records" << std::endl; 

        return 0;
       
    }
    catch (Poco::Data::MySQL::ConnectionException &e)
    {
        std::cout << "connection:" << e.what() << std::endl;
    }
    catch (Poco::Data::MySQL::StatementException &e)
    {
        std::cout << "statement:" << e.what() << std::endl;
    }
    catch (std::exception* ex)
    {
        std::cout << "exception:" << ex->what() << std::endl;
    }

    return 1;
    
}