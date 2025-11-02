#include<iostream>
#include <algorithm> 

using namespace std;
class vehicle {
protected:
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
    vehicle(int numberOfWheels, const string& brand, const string& model,
            int year, const string& id, double basePrice, double kilometers,
            int age, int damageLevel, const string& category)
        : numberOfWheels(numberOfWheels), brand(brand), model(model), year(year),
          id(id), basePrice(basePrice), kilometers(kilometers),
          age(age), damageLevel(damageLevel), category(category) {}

    virtual ~vehicle() = default;

    const string& getID() const { return id; }
    const string& getBrand() const { return brand; }
    const string& getModel() const { return model; }
    int getYear() const { return year; }
    double getBasePrice() const { return basePrice; }
    double getKilometers() const { return kilometers; }
    int getAge() const { return age; }
    int getDamageLevel() const { return damageLevel; }
    const string& getCategory() const { return category; }

    // --- pricing logic ---
    double depreciationFactor() const {
        double kmFactor = min(0.50, kilometers / 200000.0);
        double damageFactor = clamp(damageLevel * 0.03, 0.0, 0.30);
        double ageFactor = min(0.30, age * 0.03);
        double dep = 1.0 - (kmFactor + damageFactor + ageFactor);
        return max(dep, 0.10);
    }

    double minPrice() const { return max(0.0, (depreciationFactor() - 0.05)) * basePrice; }
    double maxPrice() const { return min(1.0, (depreciationFactor() + 0.05)) * basePrice; }
};

class car : public vehicle {
public:
    car(const string& brand, const string& model, int year,
        const string& id, double basePrice, double kilometers, int age,
        int damageLevel, const string& category)
        : vehicle(4, brand, model, year, id, basePrice, kilometers, age, damageLevel, category) {}
};

// subcategories
class sedan : public car {
public:
    sedan(const string& brand, const string& model, int year,
          const string& id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Sedan") {}
};
class coupe : public car {
public:
    coupe(const string& brand, const string& model, int year,
          const string& id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Coupe") {}
};
class hatchback : public car {
public:
    hatchback(const string& brand, const string& model, int year,
              const string& id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Hatchback") {}
};
class convertible : public car {
public:
    convertible(const string& brand, const string& model, int year,
                const string& id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Convertible") {}
};
class supercar : public car {
public:
    supercar(const string& brand, const string& model, int year,
             const string& id, double basePrice, double kilometers, int age, int damageLevel)
        : car(brand, model, year, id, basePrice, kilometers, age, damageLevel, "Supercar") {}
};

// #include <iostream>
// #include <string>
// #include <chrono>
// #include <ctime>

// class vehicle {
// protected:
//     int numberOfWheels;
//     string brand;
//     string model;
//     int year;
//     string id;
//     double price;
//     double mileage;        // total kilometers driven
//     bool isAvailable;      // whether the vehicle is in stock
//     string fuelType;  // e.g. Petrol, Diesel, Electric, Hybrid
//     string color;

//     // Protected constructor
//     vehicle(int numberOfWheels, const string& brand, const string& model,
//             int year, const string& id, double price, double mileage,
//             bool isAvailable, const string& fuelType, const string& color)
//         : numberOfWheels(numberOfWheels), brand(brand), model(model), year(year),
//           id(id), price(price), mileage(mileage),
//           isAvailable(isAvailable), fuelType(fuelType), color(color) {}

// public:
//     virtual ~vehicle() = default;

//     // Calculate vehicle age automatically from current year
//     int getAge() const {
//         auto now = chrono::system_clock::now();
//         time_t t = chrono::system_clock::to_time_t(now);
//         tm* now_tm = localtime(&t);
//         return (now_tm->tm_year + 1900) - year;
//     }

//     void setAvailability(bool status) { isAvailable = status; }

//     virtual void display() const {
//         cout << "--------------------------------\n";
//         cout << "Brand: " << brand
//                   << "\nModel: " << model
//                   << "\nYear: " << year
//                   << "\nAge: " << getAge() << " years"
//                   << "\nID: " << id
//                   << "\nColor: " << color
//                   << "\nWheels: " << numberOfWheels
//                   << "\nFuel: " << fuelType
//                   << "\nMileage: " << mileage << " km"
//                   << "\nPrice: $" << price
//                   << "\nAvailable: " << (isAvailable ? "Yes" : "No")
//                   << "\n--------------------------------\n";
//     }
// };

// // Base car class
// class car : public vehicle {
// public:
//     car(const string& brand, const string& model, int year, const string& id,
//         double price, double mileage, bool isAvailable,
//         const string& fuelType, const string& color)
//         : vehicle(4, brand, model, year, id, price, mileage, isAvailable, fuelType, color) {}

//     void display() const override {
//         cout << "[Car]\n";
//         vehicle::display();
//     }
// };

// // Subcategories
// class coupe : public car {
// public:
//     coupe(const string& brand, const string& model, int year, const string& id,
//           double price, double mileage, bool isAvailable,
//           const string& fuelType, const string& color)
//         : car(brand, model, year, id, price, mileage, isAvailable, fuelType, color) {}

//     void display() const override {
//         cout << "[Coupe]\n";
//         car::display();
//     }
// };

// class convertible : public car {
// public:
//     convertible(const string& brand, const string& model, int year, const string& id,
//                 double price, double mileage, bool isAvailable,
//                 const string& fuelType, const string& color)
//         : car(brand, model, year, id, price, mileage, isAvailable, fuelType, color) {}

//     void display() const override {
//         cout << "[Convertible]\n";
//         car::display();
//     }
// };

// class sedan : public car {
// public:
//     sedan(const string& brand, const string& model, int year, const string& id,
//           double price, double mileage, bool isAvailable,
//           const string& fuelType, const string& color)
//         : car(brand, model, year, id, price, mileage, isAvailable, fuelType, color) {}

//     void display() const override {
//         cout << "[Sedan]\n";
//         car::display();
//     }
// };

// class supercar : public car {
// public:
//     supercar(const string& brand, const string& model, int year, const string& id,
//              double price, double mileage, bool isAvailable,
//              const string& fuelType, const string& color)
//         : car(brand, model, year, id, price, mileage, isAvailable, fuelType, color) {}

//     void display() const override {
//         cout << "[Supercar]\n";
//         car::display();
//     }
// };

// // Example usage
// int main() {
//     coupe c("BMW", "M4", 2021, "C001", 76000, 15000, true, "Petrol", "Blue");
//     convertible cv("Mazda", "MX-5", 2022, "C002", 31000, 8000, true, "Petrol", "Red");
//     sedan s("Toyota", "Camry", 2020, "C004", 27000, 45000, false, "Hybrid", "Silver");
//     supercar sc("Ferrari", "SF90 Stradale", 2025, "C005", 500000, 1000, true, "Hybrid", "Yellow");

//     c.display();
//     cv.display();
//     s.display();
//     sc.display();

//     return 0;
// }
