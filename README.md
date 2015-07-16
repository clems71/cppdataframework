C++14 Data Framework
====================

Declare a new data type
-----------------------

    struct Todo
    {
        #define TodoFields(F) \
            F(title,       std::string,  "TODO") \
            F(priority,    int,          -1) \
            F(description, std::string,  ) \

        DATA_IMPL(TodoFields)
    };
