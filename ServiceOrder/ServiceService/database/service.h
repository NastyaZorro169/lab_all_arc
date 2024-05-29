#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>
#include <ctime>

namespace database
{
    class Service{
        private:
            long service_id;
            std::string service_name;
            std::string service_category;
            std::string service_content;
            long creator_id;
            time_t _create_date;

        public:
            static void init();
                        
            static Service save_service_to_mysql(long creator_id, std::string service_name, std::string service_category, std::string service_content);
            static std::vector<Service> get_services();

            Poco::JSON::Object::Ptr toJSON() const;

            long serviceId();
    };
}

#endif