C++14 Data Framework
====================

What is this data framework ?
-----------------------------
In C++, you often need to create a data layer that supports introspection, serialization. I decided to factorize this layer in one library, which use the new C++14 standard to support interesting features:
  - access by name while still **type-safe**
  - introspection
  - JSON serialization, easily extendable to other formats


Planned features
----------------
  - Data migration support (versionning, update, ...)
  - More serialization formats


Supported platforms
-------------------
  - Mac OS X - Clang 3.5+
  - Linux - Clang 3.5+
  - Linux - G++ 4.9+
  - Windows - MSYS2 MINGW 4.9+
  - *Windows - MSVC 2015 to be tested*


UNIX Build
----------
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make && ctest


Declare a new data type
-----------------------
    #include <data.h>

    struct Todo
    {
      #define TodoFields(F) \
        F(title,       std::string,  "TODO") \
        F(priority,    int,          -1) \
        F(description, std::string,  ) \

      DATA_IMPL(TodoFields)
    };


Serialization support
---------------------

    #include <data.h>
    #include <datajson.h>

    std::string encodeATodo(const Todo & todo) {
      JsonVal js;
      const std::string todoSerialized = todo.encode(js);
      return todoSerialized;
    }

    void decodeATodo(Todo & todo, const std::string & todoSerialized) {
      Json::Reader rdr; JsonVal js; // JSON encoder related
      rdr.parse(todoSerialized, js);
      todo.decodeStrict(js);
    }
