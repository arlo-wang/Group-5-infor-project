import time
import intel_jtag_uart

def main():
    # Create a JTAG UART object
    jtag = intel_jtag_uart.intel_jtag_uart()
    print("Connected to JTAG UART.")

    # Optionally read and print any initial banner or prompt from firmware:
    time.sleep(0.2)
    initial_msg = jtag.read().decode()
    if initial_msg:
        print(initial_msg, end="")

    while True:
        # Just read the entire line from the user
        user_input = input()

        # If you want to detect "exit":
        if user_input.lower() == "exit":
            ...
        
        # Send it to the DE10
        jtag.write((user_input + "\n").encode())

        # Pause, then read everything from the DE10
        time.sleep(0.1)
        response = jtag.read().decode()
        if response:
            print(response, end="")



if __name__ == "__main__":
    main()
