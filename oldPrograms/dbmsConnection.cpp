#include <iostream>
#include <pqxx/pqxx>
#include <string>

using namespace std;

class database
{
    static pqxx::connection *conn;

public:
    database()
    {
        try
        {
            conn = new pqxx::connection("dbname=dealership user=postgres password=password host=localhost port=5432");
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
    bool checkPassword(string username, string password)
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

    ~database()
    {
        if (conn)
        {
            delete conn;
        }
    }
};

pqxx::connection *database::conn = nullptr;

int main()
{
    database db;
    // db.selectAll("user_details");
    db.checkPassword("Ayush", "Stupid");
}
