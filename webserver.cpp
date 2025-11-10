#define CPPHTTPLIB_MULTIPART_FORM_DATA

#include <fstream>
#include <httplib.h>
#include <iostream>
#include <pqxx/pqxx>
#include <unordered_set>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

class vehicle
{

    int numberOfWheels;
    string brand;
    string model;
    int year;
    string id;
    double basePrice;
    double kilometers;
    int age;
    int damageLevel;
    string category;
    double dF;
    double minP;
    double maxP;

public:
    vehicle(int numberOfWheels, const string &brand, const string &model,
            int year, const string &id, double basePrice, double kilometers,
            int age, int damageLevel, const string &category)
        : numberOfWheels(numberOfWheels), brand(brand), model(model), year(year),
          id(id), basePrice(basePrice), kilometers(kilometers),
          age(age), damageLevel(damageLevel), category(category) {}

    virtual ~vehicle() = default;
    const int getNumberOfWheels() const { return numberOfWheels; }
    const string &getID() const { return id; }
    const string &getBrand() const { return brand; }
    const string &getModel() const { return model; }
    int getYear() const { return year; }
    double getBasePrice() const { return basePrice; }
    double getKilometers() const { return kilometers; }
    int getAge() const { return age; }
    int getDamageLevel() const { return damageLevel; }
    const string &getCategory() const { return category; }

    // --- pricing logic ---
    double depreciationFactor() const
    {
        double kmFactor = min(0.50, kilometers / 200000.0);
        double damageFactor = clamp(damageLevel * 0.03, 0.0, 0.30);
        double ageFactor = min(0.30, age * 0.03);
        double dep = 1.0 - (kmFactor + damageFactor + ageFactor);
        return max(dep, 0.10);
    }

    double minPrice() const { return max(0.0, (depreciationFactor() - 0.05)) * basePrice; }
    double maxPrice() const { return min(1.0, (depreciationFactor() + 0.05)) * basePrice; }
};

class car : public vehicle
{
public:
    car(const string &brand, const string &model, int year,
        const string &id, double basePrice, double kilometers, int age,
        int damageLevel, const string &category)
        : vehicle(4, brand, model, year, id, basePrice, kilometers, age, damageLevel, category) {}
};

// subcategories
class sedan : public car
{
public:
    sedan(const string &brand, const string &model, int year,
          const string &id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Sedan") {}
};
class coupe : public car
{
public:
    coupe(const string &brand, const string &model, int year,
          const string &id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Coupe") {}
};
class hatchback : public car
{
public:
    hatchback(const string &brand, const string &model, int year,
              const string &id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Hatchback") {}
};
class convertible : public car
{
public:
    convertible(const string &brand, const string &model, int year,
                const string &id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Convertible") {}
};
class supercar : public car
{
public:
    supercar(const string &brand, const string &model, int year,
             const string &id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Supercar") {}
};

class database
{
    static pqxx::connection *conn;

public:
    database()
    {
        try
        {
            if (!conn->is_open())
            {
                cerr << "❌ Could not connect to database\n";
                throw runtime_error("Cannot connect to database");
            }
            cout << "✅ Connected to database: " << conn->dbname() << "\n";
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
        }
    }
    static void connection(string con)
    {
        conn = new pqxx::connection(con);
    }

    void selectAll(const string &table)
    {
        string query = "SELECT * FROM " + table;
        try
        {
            pqxx::nontransaction txn(*conn);
            pqxx::result r = txn.exec(query);

            if (r.empty())
            {
                cout << "⚠️ No rows found.\n";
                return;
            }

            // Print headers
            for (pqxx::row::size_type col = 0; col < r.columns(); ++col)
            {
                cout << r.column_name(col);
                if (col < r.columns() - 1)
                    cout << " | ";
            }
            cout << "\n---------------------\n";

            // Print rows
            for (const auto &row : r)
            {
                for (pqxx::row::size_type col = 0; col < row.size(); ++col)
                {
                    cout << row[col].c_str();
                    if (col < row.size() - 1)
                        cout << " | ";
                }
                cout << "\n";
            }
        }
        catch (const exception &e)
        {
            cerr << "Error: " << e.what() << "\n";
        }
    }



static json selectAllJSON_nlohmann(const string &table)
{
    json arr = json::array();

    try {
        pqxx::nontransaction txn(*conn);
        pqxx::result r = txn.exec("SELECT * FROM " + table);

        for (const auto &row : r) {
            json obj;
            for (pqxx::row::size_type col = 0; col < row.size(); ++col) {
                const string name = r.column_name(col);

                if (row[col].is_null()) {
                    obj[name] = nullptr;
                } else {
                    string sval = row[col].c_str();

                    // Try to parse as integer then double; fallback to string.
                    try {
                        // try integer
                        long long ival = stoll(sval);
                        obj[name] = ival;
                    } catch (...) {
                        try {
                            double dval = stod(sval);
                            obj[name] = dval;
                        } catch (...) {
                            obj[name] = sval;
                        }
                    }
                }
            }
            arr.push_back(obj);
        }
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << '\n';
    }

    return arr;
}


    static bool checkPassword(string username, string password)
    {
        try
        {
            string query = "select password from user_details where username = $1";
            pqxx::work W(*conn);
            pqxx::result R = W.exec_params(query, username);
            if (R.empty())
            {
                cout << "User not found";
                return false;
            }
            string actualPassword = R[0][0].as<string>();
            if (actualPassword == password)
            {
                cout << "Passwords match, user will be logged in" << endl;
                return true;
            }
            else
            {
                cout << "Incorrect password" << endl;
                return false;
            }
        }
        catch (const exception &e)
        {
            cerr << "Error: " << e.what() << "\n";
            return false;
        }
    }
    static void addVehicle(const vehicle v)
    {
        try
        {
            pqxx::work W(*conn);
            double dep = v.depreciationFactor();
            double minP = v.minPrice();
            double maxP = v.maxPrice();
            // Prepare SQL statement with parameters
            W.conn().prepare("insert_vehicle",
                             "INSERT INTO vehicles "
                             "( id,number_of_wheels, brand, model, year, base_price, kilometers, age, damage_level, category, depreciation_factor, min_price, max_price) "
                             "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12,$13)");

            // Execute prepared statement
            W.exec_prepared("insert_vehicle",
                            v.getID(),
                            (v.getNumberOfWheels()),
                            (v.getBrand()),
                            (v.getModel()),
                            (v.getYear()),
                            (v.getBasePrice()),
                            (v.getKilometers()),
                            (v.getAge()),
                            (v.getDamageLevel()),
                            (v.getCategory()),
                            dep,
                            minP,
                            maxP);

            // Commit transaction
            W.commit();

            cout << "✅ Vehicle inserted successfully: " << v.getID() << endl;
        }
        catch (const exception &e)
        {
            cout << e.what() << endl;
        }
    }

    ~database()
    {
        if (conn)
        {
            delete conn;
        }
    }
};

pqxx::connection *database::conn = nullptr;

class filehandling
{
public:
    static string readFile(string path)
    {
        ifstream file(path);
        if (!file.is_open())
        {
            cerr << "Failed to open file" << endl;
            cout << "Cant open file";
            return "error";
        }
        stringstream buffer;
        buffer << file.rdbuf();
        string fileContent = buffer.str();
        cout << "Sent content for path " << path << endl;
        return fileContent;
    }
};
string generate_session_id()
{
    const char hex_chars[] = "0123456789abcdef";
    std::string session_id = "";

    // A session ID length of 32 characters (16 bytes) is common
    const int ID_LENGTH = 32;

    for (int i = 0; i < ID_LENGTH; ++i)
    {
        // Get a random index into the hex_chars array
        int index = std::rand() % 16;
        session_id += hex_chars[index];
    }

    return session_id;
}
bool checkLogin(unordered_set<string> session_ids, string session_id)
{
    if (session_ids.count(session_id) > 0)
    {
        return true;
    }
    else
    {
        cout << "No login cookie, please login " << endl;

        return false;
    }
}
std::string get_cookie_value_manual(const httplib::Request &req, const std::string &key)
{
    // 1. Get the entire raw "Cookie" header (e.g., "session_id=abc; user=admin")
    std::string cookie_header = req.get_header_value("Cookie");

    if (cookie_header.empty())
    {
        return ""; // No cookies sent
    }

    // Prepare the key to search for (e.g., "session_id=")
    std::string search_key = key + "=";

    // 2. Find the start position of the cookie key
    size_t start_pos = cookie_header.find(search_key);

    if (start_pos == std::string::npos)
    {
        return ""; // Cookie key not found
    }

    // Adjust start_pos to the beginning of the actual value
    start_pos += search_key.length();

    // 3. Find the end position of the cookie value (either a semicolon or end of string)
    size_t end_pos = cookie_header.find(';', start_pos);

    // If no semicolon is found, the value extends to the end of the header string
    if (end_pos == std::string::npos)
    {
        end_pos = cookie_header.length();
    }

    // 4. Extract the substring (the cookie value)
    std::string value = cookie_header.substr(start_pos, end_pos - start_pos);

    // Optional: Trim leading/trailing whitespace if present
    // (A robust solution would handle trimming, but we omit it for simplicity here)

    return value;
}
int main()
{
    std::unordered_set<string> session_ids;
    database::connection("dbname=dealership user=postgres password=password host=localhost port=5432");
    httplib::Server svr;
    svr.Get("/", [&session_ids](const httplib::Request &req, httplib::Response &res)
            {   
                cout << "wow" << endl;
                string session_id = get_cookie_value_manual(req, "session_id");
                if(session_id.empty()){
                    res.set_redirect("/login.html");
                    return;
                }
                if(!checkLogin(session_ids,session_id)){
                    res.set_redirect("/login.html");
                    return;
                }
        string filecontent=filehandling::readFile("views/index.html");
        res.set_content((filecontent), "text/html"); });
    svr.Get("/login.html", [](const httplib::Request &req, httplib::Response &res)
            {
        string filecontent=filehandling::readFile("views/login.html");
        res.set_content((filecontent), "text/html"); });
    svr.Post("/login", [&session_ids](const httplib::Request &req, httplib::Response &res)
             {  
                std::string username = req.get_param_value("username");
                std::string password = req.get_param_value("password");
                if (database::checkPassword(username,password))
                {

                    string session_id = generate_session_id();
                    session_ids.insert(session_id);
                    // active_sessions[session_id] = username;

                    // // 4. Issue the Session Cookie
                    // // The format is: "key=value; Path=/; Max-Age=..."
                     std::string cookie_header = "session_id=" + session_id + "; Path=/; Max-Age=3600"; // Expires in 1 hour
                     res.set_header("Set-Cookie", cookie_header.c_str());

                    // // Redirect to a protected page
                    res.set_redirect("/");
                }
                else
                {
                    // Failed Login
                    res.set_content("Invalid credentials. <a href='/login.html'>Try again</a>", "text/html");
                    res.status = 401; // Unauthorized
                } });

    svr.Get("/addVehicle.html", [&session_ids](const httplib::Request &req, httplib::Response &res)
            {
                string session_id = get_cookie_value_manual(req, "session_id");
                if(session_id.empty()){
                    res.set_redirect("/login.html");
                    return;
                }
                if(!checkLogin(session_ids,session_id)){
                    res.set_redirect("/login.html");
                    return;
                }
        string filecontent=filehandling::readFile("views/addVehicle.html");
        res.set_content((filecontent), "text/html"); });

    // svr.Post("/addVehicle", [](const httplib::Request &req, httplib::Response &res)
    //          {
    //             const auto &form = req.form();
    //             cout << "Trying to add vehicle" << endl;
    //             try {
    //     // 🔹 Extract form fields
    //     string id              = req.get_param_value("id");
    //     int numberOfWheels     = stoi(req.get_param_value("number_of_wheels"));
    //     string brand           = req.get_param_value("brand");
    //     string model           = req.get_param_value("model");
    //     int year               = stoi(req.get_param_value("year"));
    //     double basePrice       = stod(req.get_param_value("base_price"));
    //     double kilometers      = stod(req.get_param_value("kilometers"));
    //     int age                = stoi(req.get_param_value("age"));
    //     int damageLevel        = stoi(req.get_param_value("damage_level"));
    //     string category        = req.get_param_value("category");
    //     // 🔹 Create the correct vehicle object
    //     vehicle* v = nullptr;
    //     if (category == "Sedan")
    //         v = new sedan(brand, model, year, id, basePrice, kilometers, age, damageLevel);
    //     else if (category == "Coupe")
    //         v = new coupe(brand, model, year, id, basePrice, kilometers, age, damageLevel);
    //     else if (category == "Hatchback")
    //         v = new hatchback(brand, model, year, id, basePrice, kilometers, age, damageLevel);
    //     else if (category == "Convertible")
    //         v = new convertible(brand, model, year, id, basePrice, kilometers, age, damageLevel);
    //     else if (category == "Supercar")
    //         v = new supercar(brand, model, year, id, basePrice, kilometers, age, damageLevel);
    //     else
    //         throw runtime_error("Unknown vehicle category");
    //     v->depreciationFactor();
    //     v->minPrice();
    //     v->maxPrice();

    //     std::string image_path;
    //     // 🔹 Insert into database
    //     database::addVehicle(*v);
    //     delete v;
    //     if (form.has_file("image")) {
    //         auto file = form.get_file("image");

    //         std::string filename = "./uploads/" + file.filename;
    //         std::ofstream ofs(filename, std::ios::binary);
    //         ofs << file.content;
    //         ofs.close();

    //         std::cout << "✅ File saved: " << filename << std::endl;
    //     } else {
    //         std::cout << "⚠️ No file uploaded" << std::endl;
    //     }
    // }
    //     // 🔹 Respond to the client
    //     res.set_redirect("/");
    // }
    // catch (const exception& e) {
    //     cerr << "❌ Error adding vehicle: " << e.what() << endl;
    //     res.status = 400;
    //     res.set_content(string("Error: ") + e.what(), "text/plain");
    //     res.set_redirect("/addVehicle.html");

    // } });

    svr.Post("/addVehicle", [](const httplib::Request &req, httplib::Response &res) {
    std::cout << "🚗 Received request to add vehicle" << std::endl;

    try {
        // ✅ Get multipart form data
        const auto &form = req.form;

        // ✅ Extract form fields safely
        std::string id              = form.get_field("id");
        int numberOfWheels          = std::stoi(form.get_field("number_of_wheels"));
        std::string brand           = form.get_field("brand");
        std::string model           = form.get_field("model");
        int year                    = std::stoi(form.get_field("year"));
        double basePrice            = std::stod(form.get_field("base_price"));
        double kilometers           = std::stod(form.get_field("kilometers"));
        int age                     = std::stoi(form.get_field("age"));
        int damageLevel             = std::stoi(form.get_field("damage_level"));
        std::string category        = form.get_field("category");

        

        // ✅ Create correct vehicle object
        vehicle *v = nullptr;
        if (category == "Sedan")
            v = new sedan(brand, model, year, id, basePrice, kilometers, age, damageLevel);
        else if (category == "Coupe")
            v = new coupe(brand, model, year, id, basePrice, kilometers, age, damageLevel);
        else if (category == "Hatchback")
            v = new hatchback(brand, model, year, id, basePrice, kilometers, age, damageLevel);
        else if (category == "Convertible")
            v = new convertible(brand, model, year, id, basePrice, kilometers, age, damageLevel);
        else if (category == "Supercar")
            v = new supercar(brand, model, year, id, basePrice, kilometers, age, damageLevel);
        else
            throw std::runtime_error("Unknown vehicle category");

        // ✅ Compute prices
        v->depreciationFactor();
        v->minPrice();
        v->maxPrice();

        // ✅ Insert into database
        database::addVehicle(*v);
        delete v;
        std::string image_path = "./templates/"+id+".png";;

        // ✅ Handle file upload
        if (form.has_file("image")) {
    auto file = form.get_file("image");

    // Force the image to be named after the vehicle ID
    std::string filename = "./templates/" + id + ".png";

    std::ofstream ofs(filename, std::ios::binary);
    ofs.write(file.content.data(), file.content.size());
    ofs.close();

    image_path = filename;
    std::cout << "✅ File saved: " << filename << std::endl;
} else {
    std::cout << "⚠️ No file uploaded" << std::endl;
}
        // ✅ Redirect on success
        res.set_redirect("/");
    }
    catch (const std::exception &e) {
        std::cerr << "❌ Error adding vehicle: " << e.what() << std::endl;
        res.status = 400;
        res.set_content(std::string("Error adding vehicle: ") + e.what(), "text/plain");
        res.set_redirect("/addVehicle.html");
    }
});


    svr.Get("/api/vehicles", [&session_ids](const httplib::Request &req, httplib::Response &res) {
    try {
        cout << "hi" << endl;
    string session_id = get_cookie_value_manual(req, "session_id");
                if(session_id.empty()){
                    cout << "No cookies " << endl;
                    res.set_redirect("/login.html");
                    return;
                }
                if(!checkLogin(session_ids,session_id)){
                    cout << "Invalid Cookies " << endl;
                    res.set_redirect("/login.html");
                    return;
                }
        cout << "sent data" << endl;
        std::string filecontent = database::selectAllJSON_nlohmann("vehicles").dump(); // or just selectAllJSON()
        res.set_content(filecontent, "application/json");
        res.status = 200;
        res.set_header("Access-Control-Allow-Origin", "*");
    }
    catch (const std::exception &e) {
        std::cerr << "❌ /api/vehicles error: " << e.what() << std::endl;
        res.status = 500;
        res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
    }
});


    bool ret = svr.set_mount_point("/", "./templates");
    if (!ret)
    {
        cout << "Error: The './templates' directory does not exist or cannot be mounted." << endl;
    }
    cout << "Server listening on http://localhost:8080" << endl;
    svr.listen("localhost", 8080);

    return 0;
}