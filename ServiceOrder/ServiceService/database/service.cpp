#include "service.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    void Service::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Service` ("
                        << "`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`service_name` VARCHAR(256) NOT NULL,"
                        << "`service_category` VARCHAR(256) NOT NULL,"
                        << "`service_content` VARCHAR(256) NOT NULL,"
                        << "`creator_id` INT NOT NULL,"
                        << "`create_date` DATETIME NOT NULL,"
                        << "PRIMARY KEY (`id`));",
                now;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.displayText() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr Service::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        Poco::Timestamp tsp = Poco::Timestamp::fromEpochTime(_create_date);

        root->set("id", service_id);
        root->set("service_name", service_name);
        root->set("service_category", service_category);
        root->set("service_content", service_content);
        root->set("creator_id", creator_id);
        root->set("create_date", tsp);

        return root;
    }

    Service Service::save_service_to_mysql(long creator_id, std::string service_name, std::string service_category, std::string service_content)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            Service service;

            insert << "INSERT INTO `Service` (creator_id, service_name, service_category, service_content, create_date,) VALUES(?, ?, ?, ?, NOW())",
                use(creator_id),
                use(service_name),
                use(service_category),
                use(service_content);

            insert.execute();

            Poco::Data::Statement select_id(session);
            select_id << "SELECT LAST_INSERT_ID()",
                into(service.service_id),
                range(0, 1);

            if (!select_id.done())
            {
                select_id.execute();
            }

            Poco::DateTime tsp;

            Poco::Data::Statement select_service(session);
            select_service << "SELECT create_date from `Service` where id = ?",
                into(tsp),
                use(service.service_id),
                range(0, 1);

            if (!select_service.done())
            {
                select_service.execute();
            }

            service._create_date = tsp.timestamp().epochTime();
            service.creator_id = creator_id;
            service.service_name = service_name;
            service.service_category = service_category;
            service.service_content = service_content;

            return service;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.displayText() << std::endl;
            throw;
        }
    }

    std::vector<Service> Service::get_services()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Service> result;
            Service a;
            
            select << "SELECT id, creator_id, service_name, service_category, service_content, create_date FROM Service ",
                into(a.service_id),
                into(a.creator_id),
                into(a.service_name),
                into(a.service_category),
                into(a.service_content),
                into(a._create_date),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.displayText() << std::endl;
            throw;
        }
    }

    long Service::serviceId(){
        return service_id;
    }

}
