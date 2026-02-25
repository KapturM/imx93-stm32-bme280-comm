import serial
import time

port='/dev/ttyLP2'
baudrate=115200

def send_and_receive(message):
    try:
        # Open the serial port
        with serial.Serial(port, baudrate, timeout=2) as ser:
            # Wait a moment for the connection to initialize
            time.sleep(2)
            
            # Send the message
            ser.write(message.encode('utf-8'))
            print(f"Sent: {message.strip()}")

            # Read the response
            response = ser.readline().decode('utf-8').strip()
            
            if response:
                print(f"Received: {response}")
            else:
                print("No response received.")
                
    except serial.SerialException as e:
        print(f"Error: {e}")

# Test STM32 commands
send_and_receive("getInfo\n")
time.sleep(1)
send_and_receive("getTemp\n")
time.sleep(1)
send_and_receive("getPres\n")
time.sleep(1)
send_and_receive("getHumid\n")
time.sleep(1)
send_and_receive("getAll\n")
