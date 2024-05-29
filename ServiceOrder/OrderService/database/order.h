#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>
#include <ctime>

namespace database
{
    class Order{
        private:
            long user_id;
            long service_id;
        public:
            static void init();
            Poco::JSON::Object::Ptr toJSON() const;
            static Order add_service_to_order(long service_id, long user_id);
            static std::vector<Order> get_order(long user_id);
    };
}

#endif