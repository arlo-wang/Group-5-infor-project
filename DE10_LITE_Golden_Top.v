module DE10_LITE_Golden_Top(
    // CLOCK
    input                   ADC_CLK_10,
    input                   MAX10_CLK1_50,
    input                   MAX10_CLK2_50,

    // SDRAM
    output      [12:0]      DRAM_ADDR,
    output       [1:0]      DRAM_BA,
    output                  DRAM_CAS_N,
    output                  DRAM_CKE,
    output                  DRAM_CLK,
    output                  DRAM_CS_N,
    inout       [15:0]      DRAM_DQ,
    output                  DRAM_LDQM,
    output                  DRAM_RAS_N,
    output                  DRAM_UDQM,
    output                  DRAM_WE_N,

    // 7-Segment Displays
    output       [7:0]      HEX0,
    output       [7:0]      HEX1,
    output       [7:0]      HEX2,
    output       [7:0]      HEX3,
    output       [7:0]      HEX4,
    output       [7:0]      HEX5,

    // Push Buttons
    input        [1:0]      KEY,

    // LEDs
    output       [9:0]      LEDR,

    // Switches
    input        [9:0]      SW,

    // VGA (not used)
    output       [3:0]      VGA_B,
    output       [3:0]      VGA_G,
    output                  VGA_HS,
    output       [3:0]      VGA_R,
    output                  VGA_VS,

    // Accelerometer
    output                  GSENSOR_CS_N,
    input  [2:1]            GSENSOR_INT,
    output                  GSENSOR_SCLK,
    inout                   GSENSOR_SDI,
    inout                   GSENSOR_SDO,

    // Arduino IO
    inout       [15:0]      ARDUINO_IO,
    inout                   ARDUINO_RESET_N,

    // General GPIO
    inout       [35:0]      GPIO,
	 
	 // External Pulse Output
	 output 						PULSE_OUT
);
		 wire			[23:0]	uart_data_buffer_export;
		 wire						pulse_signal;
    //========================================================
    // Instantiate Qsys system
    //========================================================
    nios_accelerometer u0 (
        // Clocks & Reset
        .clk_clk            (MAX10_CLK1_50),
        .reset_reset_n      (1'b1),

        // LED PIO
        .pio_0_external_connection_export (LEDR[9:0]),

        // Accelerometer SPI/I2C signals
        .accelerometer_spi_external_interface_I2C_SDAT (GSENSOR_SDI),
        .accelerometer_spi_external_interface_I2C_SCLK (GSENSOR_SCLK),
        .accelerometer_spi_external_interface_G_SENSOR_CS_N (GSENSOR_CS_N),
		  .accelerometer_spi_external_interface_G_SENSOR_INT (GSENSOR_INT),
        
		  .uart_data_buffer_external_connection_export			(uart_data_buffer_export),
		  .pulse_external_connection_export						(pulse_signal)
    );

    
    assign PULSE_OUT = pulse_signal;
    // Break the 24-bit buffer into 6 nibbles (used for 6 HEX displays)
    // Break the 24-bit buffer into 6 decimal digits for 7-segment displays
    reg [3:0] dec_digits [5:0];  // Store decimal digits

    always @(posedge MAX10_CLK1_50) begin
        integer temp;
        temp = uart_data_buffer_export; // Store the 24-bit binary value

        // Extract each decimal digit (Least Significant Digit first)
        dec_digits[0] <= temp % 10; temp = temp / 10;
        dec_digits[1] <= temp % 10; temp = temp / 10;
        dec_digits[2] <= temp % 10; temp = temp / 10;
        dec_digits[3] <= temp % 10; temp = temp / 10;
        dec_digits[4] <= temp % 10; temp = temp / 10;
        dec_digits[5] <= temp % 10; // Most Significant Digit
    end

    //========================================================
    // 7-Segment display converters (Display decimal digits)
    //========================================================
    hex_to_7seg SEG0(.in(dec_digits[0]), .out(HEX0)); 
    hex_to_7seg SEG1(.in(dec_digits[1]), .out(HEX1)); 
    hex_to_7seg SEG2(.in(dec_digits[2]), .out(HEX2)); 
    hex_to_7seg SEG3(.in(dec_digits[3]), .out(HEX3)); 
    hex_to_7seg SEG4(.in(dec_digits[4]), .out(HEX4)); 
    hex_to_7seg SEG5(.in(dec_digits[5]), .out(HEX5)); 

endmodule
