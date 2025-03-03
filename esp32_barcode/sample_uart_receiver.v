//The FPGA needs a UART receiver/transmitor module to handle incoming ESP32 transmissions.
//just for reference may not be useful :/

module uart_rx (
    input wire clk,          // System clock (e.g., 50 MHz)
    input wire rst,          // Reset
    input wire rx,           // UART RX line (connected to ESP32 TX)
    output reg [7:0] data_out,  // Received byte
    output reg valid         // Valid signal when new data is received
);
    parameter BAUDRATE = 115200;
    parameter CLK_FREQ = 50_000_000; // FPGA clock frequency (50 MHz)
    parameter BIT_PERIOD = CLK_FREQ / BAUDRATE;
    
    reg [7:0] shift_reg;
    reg [3:0] bit_index;
    reg [15:0] baud_counter;
    reg receiving;
    
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            shift_reg <= 8'b0;
            bit_index <= 0;
            baud_counter <= 0;
            receiving <= 0;
            valid <= 0;
        end else if (!receiving && !rx) begin
            receiving <= 1;  // Start bit detected
            baud_counter <= BIT_PERIOD / 2; // Start sampling in middle of bit
        end else if (receiving) begin
            if (baud_counter == BIT_PERIOD) begin
                baud_counter <= 0;
                if (bit_index < 8) begin
                    shift_reg <= {rx, shift_reg[7:1]}; // Shift in received bit
                    bit_index <= bit_index + 1;
                end else begin
                    receiving <= 0;
                    valid <= 1;       // Signal that data is ready
                    data_out <= shift_reg;
                end
            end else begin
                baud_counter <= baud_counter + 1;
            end
        end
    end
endmodule
