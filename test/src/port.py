
import serial.tools.list_ports as list_ports

def findPort(serial_number):
    ret = list()
    for port in list_ports.comports():
        if port.serial_number == serial_number:
            ret.append(port)
    if len(ret) != 1:
        raise Exception(f"{len(ret)} ports found. Expected this to be 1")
    return ret[0]

def main():
    for port in list_ports.comports():
        if(port.serial_number):
            serial_number = port.serial_number
        else:
            serial_number = ""
        print(f"{port.device}: {serial_number}")

if __name__ == "__main__":
    main()
