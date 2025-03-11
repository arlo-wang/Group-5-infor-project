#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

namespace lcd {

    void local_setup();
    void local_loop(); 
    void display_connecting();
    void display_connected();
    void display_no_connection();
    void add_product(String name, String price, String quantity);
    void display_latest_product();

}