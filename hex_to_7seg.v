module hex_to_7seg (out, in);
    output reg [6:0] out;   // Active-low 7-segment output
    input      [3:0] in;    // 4-bit binary input

    always @ (*) begin
        case (in)
            4'h0: out = 7'b1000000; // 0
            4'h1: out = 7'b1111001; // 1
            4'h2: out = 7'b0100100; // 2
            4'h3: out = 7'b0110000; // 3
            4'h4: out = 7'b0011001; // 4
            4'h5: out = 7'b0010010; // 5
            4'h6: out = 7'b0000010; // 6
            4'h7: out = 7'b1111000; // 7
            4'h8: out = 7'b0000000; // 8
            4'h9: out = 7'b0011000; // 9
            default: out = 7'b1111111; // Blank for non-decimal values (A-F)
        endcase
    end
endmodule