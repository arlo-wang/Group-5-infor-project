#include "lcd.hpp"
#include <unordered_map>
#include <vector>

constexpr uint8_t NO_COLUMNS  {20};
constexpr uint8_t NO_ROWS     {4};
constexpr uint8_t LCD_ADDRESS {0x27};

unsigned int product_num {1};
unsigned int latest_product_num {0};
float total_price {0};

struct Product {
    String name;
    String price;
    String quantity;
};

LiquidCrystal_I2C lcd_display {0x27, NO_COLUMNS, NO_ROWS};

std::unordered_map<unsigned int, Product> product_list;

String product_name      {""};
String product_price     {""};
String product_quantity  {""};

namespace lcd 
{
    // Display connecting to the server
    void display_connecting()
    {
        lcd_display.clear();

        lcd_display.setCursor(5, 1);
        lcd_display.print("Connecting");

        lcd_display.setCursor(8, 2);
        lcd_display.print("...");
    }

    // Display connected to the server
    void display_connected()
    {
        lcd_display.clear();

        lcd_display.setCursor(5, 1);
        lcd_display.print("Connected!");

        lcd_display.setCursor(8, 2);
        lcd_display.print(":)");
    }

    // Display no connection to the server
    void display_no_connection()
    {
        lcd_display.clear();
        lcd_display.setCursor(5, 1);
        lcd_display.print("No Connection!");

        lcd_display.setCursor(8, 2);
        lcd_display.print(":(");
    }

    void add_product(String name, String price, String quantity)
    {
        // Check if product with same name already exists
        for (unsigned int i = 0; i < product_num; i++) {
            if (product_list[i].name == name) {

                int existingQuantity = product_list[i].quantity.toInt();
                int newQuantity = quantity.toInt();

                // Add quantities and convert back to string
                product_list[i].quantity = String(existingQuantity + newQuantity);
                latest_product_num = i;
                total_price += price.toFloat();
                return;
            }
        }
        // If no matching product found, add as new product
        product_list[product_num] = {name, price, quantity};
        latest_product_num = product_num;
        total_price += price.toFloat();
        product_num++;
    }

    void display_latest_product()
    {
        lcd_display.clear();
        lcd_display.setCursor(0, 0);
        lcd_display.print("Product    : " + product_list[latest_product_num].name);

        lcd_display.setCursor(0, 1);
        lcd_display.print("Price      : " + product_list[latest_product_num].price);

        lcd_display.setCursor(0, 2);
        lcd_display.print("Quantity   : " + product_list[latest_product_num].quantity);

        lcd_display.setCursor(0, 3);
        // Format total_price to show only 2 decimal places
        char formatted_price[10]; 
        dtostrf(total_price, 1, 2, formatted_price);
        lcd_display.print("Total price: " + String(formatted_price));
    }
    
    void local_setup() 
    {
        lcd_display.init();
        lcd_display.backlight();
    }

    // Local loop
    void local_loop() 
    {
        display_latest_product();
        delay(1000);
    }


}