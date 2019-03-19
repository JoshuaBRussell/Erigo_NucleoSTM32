import serial
import time

START_BYTE = b'\x02' #ASCII Start of Text
DELIMITER = b'\xF1'  #
END_BYTE = b'\x03'   #ASCII End of Text

TEST_AMPLITUDE_MA  = 300
NM_AMPITUDE_MA = 80
FREQ_SEL = 1 #0/1/2/3 <->12.5/25/50/100Hz  

def main(Test_Amplitude_mA, NM_Amplitude_mA, Freq_Sel):
    ser = serial.Serial("COM3", 115200, timeout = 1.0)
    for i in range(5):
        data = START_BYTE + (Test_Amplitude_mA).to_bytes(2, byteorder = 'big') + DELIMITER + (NM_Amplitude_mA).to_bytes(2, byteorder = 'big') + DELIMITER + (Freq_Sel).to_bytes(2, byteorder = 'big') + END_BYTE
        print("Sending Data...%d", i)
        print(ser.write(data))
        print(ser.read(10))
        time.sleep(1)


    ser.close()


if __name__ == "__main__":
    main(TEST_AMPLITUDE_MA, NM_AMPITUDE_MA, FREQ_SEL)