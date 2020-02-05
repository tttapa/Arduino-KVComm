#include <KVComm/KV_Builder.hpp>
#include <KVComm/KV_Parser.hpp>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

/// Define a custom type that can be added to the dict as well.
struct CustomType {
    float x;
    float y;
};

/// Define how CustomType has to be serialized and deserialized, as well as its
/// type ID and size
template <>
struct KV_Type<CustomType> {
    /// Return a unique type ID.
    inline static uint8_t getTypeID() { return 100; }
    /// Return the required buffersize in bytes.
    inline static size_t getLength() { return 2 * sizeof(float); }
    /// Write a CustomType object to a byte buffer.
    inline static void writeToBuffer(const CustomType &t, uint8_t *buffer) {
        memcpy(buffer + 0 * sizeof(float), &t.x, sizeof(float));
        memcpy(buffer + 1 * sizeof(float), &t.y, sizeof(float));
    }
    /// Read a CustomType object from a byte buffer.
    inline static void readFromBuffer(CustomType &t, const uint8_t *buffer) {
        memcpy(&t.x, buffer + 0 * sizeof(float), sizeof(float));
        memcpy(&t.y, buffer + 1 * sizeof(float), sizeof(float));
    }
};

/// Method for printing CustomType objects.
std::ostream &operator<<(std::ostream &os, CustomType t) {
    return os << '(' << t.x << ',' << t.y << ')';
}

/// Helper function for printing lists (arrays/vectors)
template <class T>
void printList(std::ostream &os, const T &list) {
    for (auto elem : list)
        os << elem << "  ";
    os << endl;
}

int main() {
    Static_KV_Builder<512> kv;

    //                                                                        //
    // -------------------- Adding data to the dictionary ------------------- //
    //                                                                        //

    // Adding single values
    // =====================
    kv.add("π", 3.14159265358979323846);
    kv.add("The meaning of life, the universe and everything", 42);
    kv.add("message", "The EAGLE has landed");
    kv.add("success", true);

    // If you explicitly specify the type, the value will be converted
    // to this type, otherwise the type is derived from the second argument
    kv.add<CustomType>("custom", {1.1, 2.2});

    // Adding variables (using variable name as key)
    uint32_t steak = 0xDEADBEEF;
    ADD_VAR(kv, steak); // This is equivalent to:
    kv.add("steak", steak);

    // Adding arrays of values
    // ========================
    float coordinates[]               = {0.1, 5.2, 3.4};
    std::array<float, 4> motorOutputs = {0.56, 0.55, 0.54, 0.57};
    std::vector<double> vector        = {1e10, 1e11, 1e12, 1e13, 1e14};
    kv.add("coordinates", coordinates);
    kv.add("motor outputs", motorOutputs);
    kv.add("vector", vector);
    kv.add("integers", {1, 2, 3, 4});
    // You can specify the type explicitly if necessary:
    //   kv.add<int>("integers", {1, 2, 3, 4});

    // Overwriting a value
    // ======================
    // Must be the same type and size
    kv.add("integers", {10, 20, 30, 40});

    //                                                                        //
    // ------------------ Retrieving data from the buffer ------------------- //
    //                           (on the computer)                            //

    // Parsing the data from the buffer
    // ================================
    KV_Parser dict = {kv.getBuffer(), kv.getLength()};

    // Printing all keys
    // =================
    cout << endl << "keys:" << endl;
    for (auto &element : dict)
        cout << " + " << element.first << endl;
    cout << endl;

    // Checking if a key exists
    // ========================
    cout << std::boolalpha << "contains π? " << dict.contains("π") << endl;
    cout << std::boolalpha << "contains ω? " << dict.contains("ω") << endl
         << endl;

    // Checking the type of a value
    // ============================
    cout << "`success` has type bool?\t" << dict["success"].hasType<bool>()
         << endl;
    cout << "`custom` has type double?\t" << dict["custom"].hasType<double>()
         << endl;
    cout << "`integers` has type int?\t" << dict["integers"].hasType<int>()
         << endl
         << endl;

    // Retrieving single values
    // ========================
    // You have to specify the type because C++ is statically typed

    // Retrieve as the given type
    double pi = dict["π"].getAs<double>();
    cout << "π = " << pi << endl;

    cout
        << "The meaning of life, the universe and everything = "
        << dict["The meaning of life, the universe and everything"].getAs<int>()
        << endl;

    // Retrieve as an std::string
    cout << "message = " << dict["message"].getString() << endl;

    // Retrieve and store in an existing variable
    bool success;
    dict["success"].get(success);
    cout << std::boolalpha << "success = " << success << endl;

    cout << "custom = " << dict["custom"].getAs<CustomType>() << endl;

    cout << std::hex << std::uppercase << "steak = 0x"
         << dict["steak"].getAs<uint32_t>() << std::dec << std::nouppercase
         << endl
         << endl;

    // Retrieving elements from arrays
    // ===============================
    float z = dict["coordinates"].getAs<float>(2); // index 2
    cout << "coordinates[2] = " << z << endl;
    cout << "motorOutputs[0] = " << dict["motor outputs"].getAs<float>(0)
         << endl;
    cout << "ingegers[3] = " << dict["integers"].getAs<int>(3) << endl;

    // Retrieving complete arrays
    // ==========================
    // Arrays have a fixed size
    std::array<float, 3> coords = dict["coordinates"].getArray<float, 3>();
    printList(cout << "coordinates = ", coords);

    // You can use dynamic vectors as well
    std::vector<float> motor = dict["motor outputs"].getVector<float>();
    printList(cout << "motorOutputs = ", motor);

    // No need to specify the entire type, just use `auto` to make the compiler
    // figure out the type for you
    auto vec = dict["vector"].getVector<double>();
    printList(cout << "vector = ", vec);
    cout << endl;

    // Retreiving values that don't exist
    // ==================================
    try {
        dict["ω"];
    } catch (std::out_of_range &e) {
        cerr << "Key `ω` doesn't exist in dict" << endl;
    }

    // Retreiving values as the wrong type
    // ===================================
    try {
        dict["π"].getAs<int>();
    } catch (KV_Exception &e) {
        cerr << "Value of `π` is not of type `int`" << endl;
    }

    // Retreiving values outside of the array
    // ======================================
    try {
        dict["integers"].getAs<int>(4);
    } catch (KV_Exception &e) {
        auto length =
            dict["integers"].getDataLength() / KV_Type<int>::getLength();
        cerr << "Index 4 is out of bounds for array of length " << length
             << endl
             << endl;
    }

    //                                                                        //
    // ------------------- Dumping the data in the buffer ------------------- //
    //                      (to understand how it works)                      //

    cout << "memory dump:" << endl;
    kv.print(cout); // print the buffer as a readable hexdump
    // kv.printPython(cout); // print the buffer as a Python bytes object
}

/*
Expected output
===============

keys:
 + The meaning of life, the universe and everything
 + coordinates
 + custom
 + integers
 + message
 + motor outputs
 + steak
 + success
 + vector
 + π

contains π? true
contains ω? false

`success` has type bool?	true
`custom` has type double?	false
`integers` has type int?	true

π = 3.14159
The meaning of life, the universe and everything = 42
message = The EAGLE has landed
success = true
custom = (1.1,2.2)
steak = 0xDEADBEEF

coordinates[2] = 3.4
motorOutputs[0] = 0.56
ingegers[3] = 40
coordinates = 0.1  5.2  3.4  
motorOutputs = 0.56  0.55  0.54  0.57  
vector = 1e+10  1e+11  1e+12  1e+13  1e+14  

Key `ω` doesn't exist in dict
Value of `π` is not of type `int`
Index 4 is out of bounds for array of length 4

memory dump:
   0   02 0A 08 00   . . . . 
   4   CF 80 00 00   . . . . 
   8   18 2D 44 54   . - D T 
  12   FB 21 09 40   . ! . @ 
  16   30 05 04 00   0 . . . 
  20   54 68 65 20   T h e   
  24   6D 65 61 6E   m e a n 
  28   69 6E 67 20   i n g   
  32   6F 66 20 6C   o f   l 
  36   69 66 65 2C   i f e , 
  40   20 74 68 65     t h e 
  44   20 75 6E 69     u n i 
  48   76 65 72 73   v e r s 
  52   65 20 61 6E   e   a n 
  56   64 20 65 76   d   e v 
  60   65 72 79 74   e r y t 
  64   68 69 6E 67   h i n g 
  68   00 00 00 00   . . . . 
  72   2A 00 00 00   * . . . 
  76   07 0C 15 00   . . . . 
  80   6D 65 73 73   m e s s 
  84   61 67 65 00   a g e . 
  88   54 68 65 20   T h e   
  92   45 41 47 4C   E A G L 
  96   45 20 68 61   E   h a 
 100   73 20 6C 61   s   l a 
 104   6E 64 65 64   n d e d 
 108   00 00 00 00   . . . . 
 112   07 0B 01 00   . . . . 
 116   73 75 63 63   s u c c 
 120   65 73 73 00   e s s . 
 124   01 00 00 00   . . . . 
 128   06 64 08 00   . d . . 
 132   63 75 73 74   c u s t 
 136   6F 6D 00 00   o m . . 
 140   CD CC 8C 3F   . . . ? 
 144   CD CC 0C 40   . . . @ 
 148   05 06 04 00   . . . . 
 152   73 74 65 61   s t e a 
 156   6B 00 00 00   k . . . 
 160   EF BE AD DE   . . . . 
 164   0B 09 0C 00   . . . . 
 168   63 6F 6F 72   c o o r 
 172   64 69 6E 61   d i n a 
 176   74 65 73 00   t e s . 
 180   CD CC CC 3D   . . . = 
 184   66 66 A6 40   f f . @ 
 188   9A 99 59 40   . . Y @ 
 192   0D 09 10 00   . . . . 
 196   6D 6F 74 6F   m o t o 
 200   72 20 6F 75   r   o u 
 204   74 70 75 74   t p u t 
 208   73 00 00 00   s . . . 
 212   29 5C 0F 3F   ) \ . ? 
 216   CD CC 0C 3F   . . . ? 
 220   71 3D 0A 3F   q = . ? 
 224   85 EB 11 3F   . . . ? 
 228   06 0A 28 00   . . ( . 
 232   76 65 63 74   v e c t 
 236   6F 72 00 00   o r . . 
 240   00 00 00 20   . . .   
 244   5F A0 02 42   _ . . B 
 248   00 00 00 E8   . . . . 
 252   76 48 37 42   v H 7 B 
 256   00 00 00 A2   . . . . 
 260   94 1A 6D 42   . . m B 
 264   00 00 40 E5   . . @ . 
 268   9C 30 A2 42   . 0 . B 
 272   00 00 90 1E   . . . . 
 276   C4 BC D6 42   . . . B 
 280   08 05 10 00   . . . . 
 284   69 6E 74 65   i n t e 
 288   67 65 72 73   g e r s 
 292   00 00 00 00   . . . . 
 296   0A 00 00 00   . . . . 
 300   14 00 00 00   . . . . 
 304   1E 00 00 00   . . . . 
 308   28 00 00 00   ( . . .

 */