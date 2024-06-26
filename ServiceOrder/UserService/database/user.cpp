#include "user.h"
#include "database.h"
#include "../config/config.h"
#include "../database/cache.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <cppkafka/cppkafka.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void User::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();

            for (auto &hint : database::Database::get_all_hints())
            {
                Statement create_stmt(session);

                create_stmt << "CREATE TABLE IF NOT EXISTS `User` ("
                            << "`id` INT NOT NULL,"
                            << "`login` VARCHAR(256) NOT NULL,"
                            << "`password` VARCHAR(256) NOT NULL,"
                            << "`first_name` VARCHAR(256) NOT NULL,"
                            << "`last_name` VARCHAR(256) NOT NULL,"
                            << "`email` VARCHAR(256) NULL,"
                            << "PRIMARY KEY (`id`),KEY `fn` (`first_name`),KEY `ln` (`last_name`));"
                            << hint,
                    now;
                
                std::cout << create_stmt.toString() << std::endl;
            }


            Statement create_seqs(session);

            create_seqs <<"create table IF NOT EXISTS Seqs(id int not null primary key auto_increment);",now;

            std::cout << create_seqs.toString() << std::endl;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.login() = object->getValue<std::string>("login");
        user.password() = object->getValue<std::string>("password");

        return user;
    }

    long getNextId(Poco::Data::Session session)
    {
        try
        { 
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Seqs values();";

            insert.execute();

            long id;

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << id << std::endl;

            Poco::Data::Statement deleteStmt(session);

            deleteStmt << "delete from Seqs;";

            deleteStmt.execute();

            return id;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<long> User::auth(std::string &login, std::string &password)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();

            for (auto &hint : database::Database::get_all_hints())
            {
                Poco::Data::Statement select(session);
                long id;
                select << "SELECT id FROM User where login=? and password=?" << hint,
                    into(id),
                    use(login),
                    use(password),
                    range(0, 1); //  iterate over result set one row at a time

                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst()) return id;
            }
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }
    std::optional<User> User::read_by_id(long id)
    {
        try
        {
            std::string hint = database::Database::sharding_hint(id);

            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User a;
            select << "SELECT id, first_name, last_name, email, login, password FROM User where id = ? " << hint,
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._login),
                into(a._password),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return a;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }

    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();

            std::vector<User> result;
            
            for (auto &hint : database::Database::get_all_hints())
            {
                Statement select(session);
                User a;
                select << "SELECT id, first_name, last_name, email, login, password FROM User " << hint,
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._email),
                    into(a._login),
                    into(a._password),
                    range(0, 1); //  iterate over result set one row at a time

                while (!select.done())
                {
                    if (select.execute())
                        result.push_back(a);
                }
            }

            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();

            std::vector<User> result;
            
            first_name += "%";
            last_name += "%";

            for (auto &hint : database::Database::get_all_hints())
            {
                Statement select(session);
                
                User a;
                select << "SELECT id, first_name, last_name, email, login, password FROM User where first_name LIKE ? and last_name LIKE ?"
                        << hint,
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._email),
                    into(a._login),
                    into(a._password),
                    use(first_name),
                    use(last_name),
                    range(0, 1); //  iterate over result set one row at a time

                while (!select.done())
                {
                    if (select.execute())
                        result.push_back(a);
                }
            }

            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            
            _id = getNextId(session);

            std::string hint = database::Database::sharding_hint(_id);

            Poco::Data::Statement insert(session);

            insert << "INSERT INTO User (id, first_name,last_name,email,login,password) VALUES(?, ?, ?, ?, ?, ?) " << hint,
                use(_id),
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_login),
                use(_password);

            insert.execute();
            
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    #include <mutex>
    void User::send_to_queue()
    {
        static cppkafka::Configuration config ={
            {"metadata.broker.list", Config::get().get_queue_host()},
            {"acks","all"}};
        static cppkafka::Producer producer(config);
        static std::mutex mtx;
        static int message_key{0};
        using Hdr = cppkafka::MessageBuilder::HeaderType;

        std::lock_guard<std::mutex> lock(mtx);
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        bool not_sent = true;

        cppkafka::MessageBuilder builder(Config::get().get_queue_topic());
        std::string mk=std::to_string(++message_key);
        builder.key(mk); // set some key
        builder.header(Hdr{"producer_type","user writer"}); // set some custom header
        builder.payload(message); // set message

        while (not_sent)
        {
            try
            {
                producer.produce(builder);
                not_sent = false;
            }
            catch (std::exception& err)
            {
                std::cout << "kafka error:" << err.what() << std::endl;
            }
        }
    }

    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::optional<User> User::read_from_cache_by_id(long id)
    {
        try
        {
            std::string result;
            if (database::Cache::get().get(id, result))
                return fromJSON(result);
            else
                return std::optional<User>();
        }
        catch (std::exception& err)
        {
           std::cout << "statement:" << err.what() << std::endl;

           return std::optional<User>();
        }
    }

     void User::save_to_cache()
    {
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        database::Cache::get().put(_id, message);
    }
}